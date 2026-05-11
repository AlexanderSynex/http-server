#pragma once

#include <concepts>
#include <string_view>
namespace snx::logging::levels {

struct warning_t {
  constexpr static std::string_view type = "WARNING";
};

struct error_t {
  constexpr static std::string_view type = "ERROR";
};

struct info_t {
  constexpr static std::string_view type = "INFO";
};

struct debug_t {
  constexpr static std::string_view type = "DEBUG";
};

constexpr auto info = info_t{};
constexpr auto debug = debug_t{};
constexpr auto warning = warning_t{};
constexpr auto error = error_t{};

template <typename out_type> 
concept service_msg_type = requires {
  {out_type::type} -> std::convertible_to<std::string_view>;
  {out_type::type.data() != nullptr};
};

}  // namespace snx::logging::levels
