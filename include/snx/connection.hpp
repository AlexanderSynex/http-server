#pragma once

#include "snx/logging/levels.hpp"
#include "snx/logging/logger.hpp"
#include <asm-generic/socket.h>
#include <memory>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace snx::net {

template <class address_family> class connection {
 public:
  using discriptor_type = int;
  using address_type = address_family::address_type;

  template <class connection_traits>
  constexpr explicit connection(connection_traits)
      : connection(address_family::domain, connection_traits::connection_type) {
  }
  connection(discriptor_type fd) : fd{fd} {
    if (fd < 0) {
      return;
    }
    constexpr auto enable = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const void *>(&enable), sizeof(enable));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
               reinterpret_cast<const void *>(&enable), sizeof(enable));
    setsockopt(fd, SOL_SOCKET, SOCK_NONBLOCK,
               reinterpret_cast<const void *>(&enable), sizeof(enable));
  }

  connection(connection &&other) : fd{other.fd}, address(other.address) {
    other.fd = -1;
    other.address = {0};
  }

  void close() {
    logging::TSLogger{} << logging::levels::debug << "Closing connection" << fd;
    address = {0};

    if (fd == -1)
      return;

    ::close(fd);
    fd = -1;
  }

  bool closed() const { return fd < 0; }

  void set_nonblocking() {
    constexpr auto enable = 1;
    setsockopt(fd, SOL_SOCKET, SOCK_NONBLOCK,
               reinterpret_cast<const void *>(&enable), sizeof(enable));
  }

  virtual ~connection() { close(); }

  operator discriptor_type() const { return fd; }

  std::unique_ptr<connection<address_family>> get_caller() const {
    address_type peer;
    socklen_t size = sizeof(address_type);
    auto fd = accept4(this->fd, reinterpret_cast<sockaddr *>(&peer), &size,
                      SOCK_NONBLOCK);
    auto newConnection = std::make_unique<connection<address_family>>(fd);
    newConnection->address = std::move(peer);
    return std::move(newConnection);
  }

 private:
  connection(int domain, int type) : connection{socket(domain, type, 0)} {}

 protected:
  discriptor_type fd;
  address_type address;
};
}  // namespace snx::net
