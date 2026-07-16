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
  case ErrorCode::config_io_error:
    return L"config_io_error";
  case ErrorCode::command_name_empty:
    return L"command_name_empty";
  case ErrorCode::command_name_invalid_character:
    return L"command_name_invalid_character";
  case ErrorCode::command_name_reserved:
    return L"command_name_reserved";
  case ErrorCode::command_name_invalid_suffix:
    return L"command_name_invalid_suffix";
  case ErrorCode::path_invalid:
    return L"path_invalid";
  case ErrorCode::path_not_absolute:
    return L"path_not_absolute";
  case ErrorCode::encoding_error:
    return L"encoding_error";
  case ErrorCode::source_invalid:
    return L"source_invalid";
  case ErrorCode::link_creation_failed:
    return L"link_creation_failed";
  case ErrorCode::link_mode_unavailable:
    return L"link_mode_unavailable";
  case ErrorCode::entry_transaction_failed:
    return L"entry_transaction_failed";
  case ErrorCode::entry_transaction_rollback_failed:
    return L"entry_transaction_rollback_failed";
  case ErrorCode::platform_error:
    return L"platform_error";
  }

  return L"unknown";
}

bool is_recoverable_config_error(ErrorCode code) noexcept {
  return code == ErrorCode::config_missing || code == ErrorCode::config_invalid;
}

} // namespace binify::core
