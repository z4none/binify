#include "platform/windows/shell_service.h"

#include <windows.h>
#include <shellapi.h>

#include <system_error>

#include "core/error.h"

namespace binify::platform::windows {

core::Result<void> WindowsShellService::open_directory(const std::filesystem::path& directory) const {
  std::error_code error;
  if (directory.empty() || !std::filesystem::exists(directory, error) || !std::filesystem::is_directory(directory, error)) {
    return core::make_error(core::ErrorCode::path_invalid, L"Bin directory does not exist.", std::nullopt, directory.wstring());
  }
  if (error) {
    return core::make_error(core::ErrorCode::path_invalid, L"Failed to inspect Bin directory.", error.value(), directory.wstring());
  }

  const auto result = reinterpret_cast<INT_PTR>(
    ShellExecuteW(nullptr, L"open", directory.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
  if (result <= 32) {
    return core::make_error(
      core::ErrorCode::platform_error,
      L"Failed to open Bin directory.",
      static_cast<unsigned long>(result),
      directory.wstring());
  }

  return {};
}

} // namespace binify::platform::windows
