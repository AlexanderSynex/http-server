#pragma once

#include "snx/connection.hpp"
#include "snx/connections/tcp.hpp"
#include "snx/thread_pool.hpp"
#include <condition_variable>
#include <cstddef>
#include <stop_token>
#include <string_view>
#include <sys/epoll.h>
#include <thread>

namespace snx::net {

class server {
 public:
  static constexpr auto max_connections = 1024;
  using connection_type = connection_traits::network_connection;
  using connection = connection<connection_type>;

  server(std::string_view host, std::size_t port,
         std::size_t threads = std::thread::hardware_concurrency() - 1);
  server(const server &) = delete;
  server(server &&) = delete;
  server &operator=(const server &) = delete;
  server &operator=(server &&) = delete;
  virtual ~server();

  void start();

 private:
  void scheduling_task(std::stop_token stoken);
  void connection_handler(std::unique_ptr<connection> connection);

 private:
  connection gate;
  std::stop_source ssrc;
  std::condition_variable stop_event;
  
  // epoll descriptor
  int ed;
  epoll_event ev;

  std::jthread scheduler;
  thread_pool tp;
};
}  // namespace snx::net
