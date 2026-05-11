#include "snx/server.hpp"
#include "snx/address.hpp"
#include "snx/connections/tcp.hpp"
#include "snx/logging/levels.hpp"
#include "snx/logging/logger.hpp"
#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iomanip>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

constexpr std::string_view empty_page = R"(HTTP/1.1 200 OK
Content-Type: text/html

<!DOCTYPE html>
<body>
<h1>Its a dummy page</h1>
</body>
</html>
)";

using namespace snx::net;

server::server(std::string_view host, std::size_t port, std::size_t threads)
    : tp(threads), gate(connection_traits::tcp_traits{}),
      ed(epoll_create(max_connections)) {
  if (not address<connection_type>{std::move(host), port}.bind(gate)) {
    logging::TSLogger{} << logging::levels::error
                        << "Can not bind address to socket";
    throw std::invalid_argument(strerror(errno));
  }
}

server::~server() {
  ssrc.request_stop();
  stop_event.notify_all();
  close(ed);
  logging::TSLogger{} << logging::levels::info << "Server shutdown";
}

void server::start() {
  if (listen(gate, max_connections) == -1) {
    logging::TSLogger{} << logging::levels::error
                        << "Error configuring backlog for" << max_connections
                        << "connections";
    throw std::runtime_error(strerror(errno));
  }

  ev.data.fd = gate;
  ev.events = EPOLLIN;

  if (epoll_ctl(ed, EPOLL_CTL_ADD, gate, &ev) == -1) {
    logging::TSLogger{} << logging::levels::error
                        << "Error binding server to epoll";
    throw std::runtime_error(strerror(errno));
  }

  scheduler =
      std::jthread(std::bind(&server::scheduling_task, this, ssrc.get_token()));
}

void snx::net::server::scheduling_task(std::stop_token stoken) {
  logging::TSLogger{} << logging::levels::info << "Waiting for connections";
  std::array<epoll_event, max_connections> events = {};
  using namespace std::chrono;
  constexpr auto timeout = duration_cast<decltype(1ms)>(100ms).count();

  for (; not stoken.stop_requested();) {
    auto events_number = epoll_wait(ed, events.data(), events.size(), timeout);

    if (events_number == -1) {
      logging::TSLogger{} << logging::levels::error
                          << "Error waiting for epoll events";
      throw std::runtime_error(strerror(errno));
    }

    if (events_number == 0) {
      continue;
    }

    for (auto i = 0; i < events_number; ++i) {
      auto event = events[i];
      auto caller = gate.get_caller();
      tp.attach([user = std::move(caller), this]() mutable {
        connection_handler(std::move(user));
      });
    }
  }
}

void snx::net::server::connection_handler(
    std::unique_ptr<connection> connection) {
  std::array<char, 512> buffer;
  std::string request = {};
  for (auto bytes = 0;
       (bytes = recv(*connection, buffer.data(), buffer.size(), 0)) > 0;) {
    request.insert_range(request.end(),
                         buffer | std::ranges::views::take(bytes));
  }
  request.erase(std::remove(request.begin(), request.end(), '\r'),
                request.end());
  logging::TSLogger{} << logging::levels::info
                      << "Request:" << std::quoted(request);

  auto sent = send(*connection, empty_page.data(), empty_page.size(), 0);
  if (sent == -1) {
    logging::TSLogger{} << logging::levels::error
                        << "Response was not fully sent";
  }
}
