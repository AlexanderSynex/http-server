#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using addr_t = sockaddr_in;

struct Connection {
  void close() {
    ::close(socket);
    socket = -1;
  }

  bool is_free() const { return socket == -1; }
  int socket = -1;
};

template <typename Type> using ptr = std::unique_ptr<Type>;

int main(int argc, const char *argv[]) {
  auto host = inet_addr("127.0.0.1");
  auto port = 8080;
  std::filesystem::path root_page = "index.html";

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

  if (not std::filesystem::exists(root_page)) {
    throw std::invalid_argument("No root page exists");
  }

  std::stop_source ssource;
  std::shared_mutex m;
  constexpr std::size_t thread_limit = 1;
  auto thread_pool = std::array<ptr<std::jthread>, thread_limit>{};
  auto connections = std::unordered_map<std::thread::id, Connection>{};
  for (auto i = 0; i < thread_limit; ++i) {
    thread_pool[i] = std::make_unique<std::jthread>(
        [&cons = connections, &m, id = i,
         home_page = root_page](std::stop_token stoken) {
          using namespace std::chrono_literals;
          {
            std::lock_guard l(m);
            std::cout << "Starting handler " << id
                      << " at: " << std::this_thread::get_id() << std::endl;
          }

          auto &connection = cons[std::this_thread::get_id()];
          for (auto buffer = std::array<char, 1024>{0};
               not stoken.stop_requested();) {
            std::this_thread::sleep_for(100ms);

            if (connection.socket == -1)
              continue;

            for (auto connectionValid = true; connectionValid;) {
              auto status =
                  recv(connection.socket, buffer.data(), buffer.max_size(), 0);
              if (status == 0) {
                connectionValid = false;
                connection.close();
                continue;
              }

              auto message = std::string_view{buffer.data(), buffer.size()};
              std::string header = {};
              std::getline(std::stringstream{} << message, header);

              auto response = std::string{"HTTP/1.1 200 OK\n"
                                          "Content-Type: text/html\n\n"};
              response +=
                  (std::stringstream{} << std::ifstream{home_page}.rdbuf())
                      .str();
              send(connection.socket, response.data(), response.size(), 0);
              connectionValid = false;
              {
                std::lock_guard l(m);
                std::cout << id << ": " << message << std::endl;
              }
            }
            connection.close();
          };
        },
        ssource.get_token());
  }

  auto sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in peer, addr = {AF_INET, htons(port), host};

  if (bind(sock, reinterpret_cast<const struct sockaddr *>(&addr),
           sizeof(addr)) == -1) {
    perror((std::string{} + "Can not bind to address " + argv[1] + ":" +
            std::to_string(port))
               .c_str());
    return 1;
  }

  if (listen(sock, 20) == -1) {
    perror("Can not start listening for 20 connection");
    return 1;
  }

  {
    std::lock_guard l(m);
    std::cout << "Server started at " << argv[1] << ":" << port << std::endl;
  }

  for (;;) {
    {
      std::lock_guard l(m);
      std::cout << "Waiting for connection" << std::endl;
    }
    socklen_t len = sizeof(peer);
    auto connection =
        accept(sock, reinterpret_cast<struct sockaddr *>(&peer), &len);
    if (connection == -1) {
      using namespace std::chrono_literals;
      std::cerr << "Skipping. Go to sleep for 1 second" << std::endl;
      std::this_thread::sleep_for(1s);
      continue;
    }

    if (auto it = std::find_if(connections.begin(), connections.end(),
                               [](decltype(connections)::const_reference ref)
                                   -> bool { return ref.second.is_free(); });
        it == connections.end()) {
      std::cerr << "Connection refused. No free handlers" << std::endl;
      close(connection);
      continue;
    } else {
      std::lock_guard l(m);
      it->second.socket = connection;
    }
  }

  close(sock);
  return 0;
}
