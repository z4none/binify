#include "core/bat_wrapper.h"

#include "core/error.h"
#include "core/text_encoding.h"

namespace binify::core {

namespace {

std::wstring escape_batch_path(std::wstring value) {
  std::wstring escaped;
  escaped.reserve(value.size());
  for (const auto ch : value) {
    if (ch == L'%') {
      escaped += L"%%";
    } else {
      escaped.push_back(ch);
    }
  }
  return escaped;
}

} // namespace

Result<std::string> generate_bat_wrapper(const std::filesystem::path& source_exe) {
  if (source_exe.empty() || !source_exe.is_absolute()) {
    return make_error(
      ErrorCode::source_invalid,
      L"BAT wrapper source path must be absolute.",
      std::nullopt,
      source_exe.wstring());
  }

  const std::wstring content =
    L"@echo off\r\n"
    L"\"" + escape_batch_path(source_exe.wstring()) + L"\" %*\r\n"
    L"exit /b %ERRORLEVEL%\r\n";

  return wide_to_utf8(content);
}

} // namespace binify::core

