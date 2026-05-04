#pragma once

#include "logger.hpp"

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
