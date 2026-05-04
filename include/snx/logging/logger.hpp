#pragma once

#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>

namespace snx::logging {

template <class T>
concept Serializable = not std::is_pointer_v<T>;

/// @struct Thread-safe ostream logger
class TSLogger {
 public:
  static void enable_multithreading() {

  }
 
  explicit TSLogger();
  template <Serializable Type> TSLogger &operator<<(Type &&value) &&;
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