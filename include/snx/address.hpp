#pragma once

#include "snx/connection.hpp"
#include "snx/connections/tcp.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <netinet/in.h>
#include <string_view>

namespace snx::net {

template <typename address_family> class address;

template <> class address<connection_traits::network_connection> {
 public:
  using address_family = connection_traits::network_connection;
  address(std::string_view host, std::size_t port) : host{host}, port{port} {}

  bool bind(connection<address_family> &connection) && {
    typename ::snx::net::connection<address_family>::address_type addr = {
        address_family::domain, htons(port), inet_addr(host.data())};
    return ::bind(connection, reinterpret_cast<const struct sockaddr *>(&addr),
                  sizeof(decltype(addr))) != -1;
  }

 private:
  std::string host;
  std::size_t port;
};

}  // namespace snx::net
