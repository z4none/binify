#include "core/error.h"

#include <utility>

namespace binify::core {

Error make_error(ErrorCode code, std::wstring message) {
  return make_error(code, std::move(message), std::nullopt, std::nullopt);
}

Error make_error(
  ErrorCode code,
  std::wstring message,
  std::optional<unsigned long> native_code,
  std::optional<std::wstring> path) {
  return Error{
    .code = code,
    .technical_message = std::move(message),
    .native_code = native_code,
    .path = std::move(path),
  };
}

std::wstring_view to_wstring(ErrorCode code) noexcept {
  switch (code) {
  case ErrorCode::unknown:
    return L"unknown";
  case ErrorCode::not_implemented:
    return L"not_implemented";
  case ErrorCode::invalid_argument:
    return L"invalid_argument";
  case ErrorCode::config_invalid:
    return L"config_invalid";
  case ErrorCode::config_missing:
    return L"config_missing";
  case ErrorCode::path_invalid:
    return L"path_invalid";
  case ErrorCode::path_not_absolute:
    return L"path_not_absolute";
  case ErrorCode::encoding_error:
    return L"encoding_error";
  case ErrorCode::platform_error:
    return L"platform_error";
  }

  return L"unknown";
}

bool is_recoverable_config_error(ErrorCode code) noexcept {
  return code == ErrorCode::config_missing || code == ErrorCode::config_invalid;
}

} // namespace binify::core
