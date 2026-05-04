#include "snx/logging/logger.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>

using namespace snx::logging;

TSLogger::TSLogger() {
  preamble =
      std::format("[{:%d/%m/%YT%H:%M}] ", std::chrono::system_clock::now());
}

TSLogger::~TSLogger() {
  std::lock_guard<std::mutex> l(m);
  if (auto buffer_string = buffer.str(); not buffer_string.empty()) {
    std::ranges::replace(buffer_string, '\n', ' ');
    std::clog << preamble << buffer_string << std::endl;
  }
}
