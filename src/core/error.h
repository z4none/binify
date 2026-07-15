#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace binify::core {

enum class ErrorCode {
  unknown,
  not_implemented,
  invalid_argument,
  config_invalid,
  config_missing,
  config_io_error,
  command_name_empty,
  command_name_invalid_character,
  command_name_reserved,
  command_name_invalid_suffix,
  path_invalid,
  path_not_absolute,
  encoding_error,
  source_invalid,
  link_creation_failed,
  link_mode_unavailable,
  platform_error
};

struct Error {
  ErrorCode code = ErrorCode::unknown;
  std::wstring technical_message;
  std::optional<unsigned long> native_code;
  std::optional<std::wstring> path;
};

[[nodiscard]] Error make_error(ErrorCode code, std::wstring message);
[[nodiscard]] Error make_error(
  ErrorCode code,
  std::wstring message,
  std::optional<unsigned long> native_code,
  std::optional<std::wstring> path);
[[nodiscard]] std::wstring_view to_wstring(ErrorCode code) noexcept;
[[nodiscard]] bool is_recoverable_config_error(ErrorCode code) noexcept;

} // namespace binify::core
