#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

namespace snx::net::connection_traits {
  ///@struct Структура описания соединения через сеть
  struct network_connection {
    static constexpr auto domain = AF_INET;
    using address_type = sockaddr_in;
  };

  ///@struct Структура описания соединения для TCP
  struct tcp_traits {
    static constexpr auto connection_type = SOCK_STREAM | SOCK_NONBLOCK;
  };
};