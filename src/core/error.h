#pragma once

#include <optional>
#include <string>

namespace binify::core {

enum class ErrorCode {
  unknown,
  not_implemented,
  invalid_argument,
  config_invalid,
  platform_error
};

struct Error {
  ErrorCode code = ErrorCode::unknown;
  std::wstring technical_message;
  std::optional<unsigned long> native_code;
  std::optional<std::wstring> path;
};

[[nodiscard]] Error make_error(ErrorCode code, std::wstring message);

} // namespace binify::core

