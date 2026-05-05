#pragma once

#include "snx/logging/levels.hpp"
#include <concepts>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>

namespace snx::logging {

template <class T>
concept Serializable = requires(T &&obj, std::ostream &out) {
  { out << obj } -> std::convertible_to<std::ostream &>;
  not levels::service_msg_type<T>;
};

/// @struct Thread-safe ostream logger
class TSLogger {
 public:
  static void enable_multithreading() {}

  explicit TSLogger();
  template <Serializable Type> TSLogger &operator<<(Type &&value) &&;
  template <levels::service_msg_type msg_type>
  TSLogger &operator<<(msg_type) &&;
  template <Serializable Type> TSLogger &operator<<(Type &&value) &;
  ~TSLogger();

 private:
  inline static std::mutex m;
  std::string preamble;
  std::stringstream buffer;
  inline static bool show_thread = false;
};

}  // namespace snx::logging

#include "logger.tpp"