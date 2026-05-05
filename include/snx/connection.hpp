#pragma once

#include "snx/logging/logger.hpp"
#include <stdexcept>
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
      logging::TSLogger{} << logging::levels::error << "Bad discriptor provided"
                          << fd;
      throw std::invalid_argument("Can not start connection");
    }

    logging::TSLogger{} << logging::levels::info << "Opened connection:" << fd
                        << "fd";
  }
  virtual ~connection() { close(fd); }

  operator discriptor_type() const { return fd; }

 private:
  connection(int domain, int type) : connection{socket(domain, type, 0)} {}

 protected:
  discriptor_type fd;
  address_type address;
};
}  // namespace snx::net