
#include "snx/logging/levels.hpp"
#include "snx/logging/logger.hpp"
#include "snx/server.hpp"
#include <atomic>
#include <cassert>
#include <csignal>
#include <thread>

#define FAST_RUN

#include <filesystem>
#include <format>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

std::atomic_bool suspend_requested = false;

void handle_stop(int code) {
  suspend_requested.store(true, std::memory_order_release);
}

int main(int argc, const char *argv[]) {
  signal(SIGINT, handle_stop);
  auto host = std::string_view("127.0.0.1");
  auto port = 8080;
  std::filesystem::path root_page = "index.html";

#ifndef FAST_RUN
  if (argc == 2) {
    auto addr = inet_addr(argv[1]);
    if (addr == -1) {
      perror("First argument is not compatable with ipv4");
      return 1;
    }
    host = addr;
  }

  if (argc == 3) {
    try {
      port = std::stoi(argv[2]);
    } catch (std::exception e) {
      perror("Port type is not number-like");
      return 1;
    }
  }

  if (argc == 4) {
    root_page = argv[3];
  }
#endif

  if (not std::filesystem::exists(root_page)) {
    throw std::invalid_argument(
        std::format("No root page '{}' exists", root_page.string()));
  }

  auto server = snx::net::server(host, port, 2);

  server.start();

  snx::logging::TSLogger{} << snx::logging::levels::info << "Server started!"
                            << "Press Ctrl+C to shutdown";

  for (; not suspend_requested.load(std::memory_order_acquire);) {
    using namespace std::chrono;
    std::this_thread::sleep_for(100ms);
  }

  return 0;
}
