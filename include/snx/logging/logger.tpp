#pragma once

#include "logger.hpp"
#include <format>

template <snx::logging::Serializable Type>
snx::logging::TSLogger &snx::logging::TSLogger::operator<<(Type &&value) && {
  buffer << value;
  return *this;
}

template <snx::logging::Serializable Type>
snx::logging::TSLogger &snx::logging::TSLogger::operator<<(Type &&value) & {
  buffer << " " << std::forward<Type>(value);
  return *this;
}

template <snx::logging::levels::service_msg_type msg_type>
snx::logging::TSLogger &snx::logging::TSLogger::operator<<(msg_type value) && {
  preamble = std::format("{:8} {}", msg_type::type, preamble);
  return *this;
}