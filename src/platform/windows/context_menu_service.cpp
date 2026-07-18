#include "platform/windows/context_menu_service.h"

#include <windows.h>

#include <string>

#include "core/context_menu_command.h"
#include "core/error.h"

namespace binify::platform::windows {

namespace {

constexpr const wchar_t* kDefaultShellKeyPath = L"Software\\Classes\\SystemFileAssociations\\.exe\\shell\\binify";
constexpr const wchar_t* kMenuText = L"加入 Binify...";

core::Error registry_error(std::wstring message, LSTATUS status, const std::wstring& path) {
  return core::make_error(
    core::ErrorCode::context_menu_registry_error,
    std::move(message),
    static_cast<unsigned long>(status),
    L"HKCU\\" + path);
}

core::Result<void> set_string_value(HKEY key, const wchar_t* value_name, const std::wstring& value) {
  const auto bytes = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
  const auto status = RegSetValueExW(
    key,
    value_name,
    0,
    REG_SZ,
    reinterpret_cast<const BYTE*>(value.c_str()),
    bytes);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to write context menu registry value.", status, L"Software\\Classes");
  }
  return {};
}

} // namespace

RegistryContextMenuService::RegistryContextMenuService()
  : shell_key_path_(kDefaultShellKeyPath) {}

RegistryContextMenuService::RegistryContextMenuService(std::wstring shell_key_path)
  : shell_key_path_(std::move(shell_key_path)) {}

core::Result<void> RegistryContextMenuService::install(const std::filesystem::path& executable_path) const {
  auto command = core::format_context_menu_command(executable_path);
  if (!command) {
    return command.error();
  }

  HKEY shell_key = nullptr;
  auto status = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    shell_key_path_.c_str(),
    0,
    nullptr,
    REG_OPTION_NON_VOLATILE,
    KEY_SET_VALUE,
    nullptr,
    &shell_key,
    nullptr);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to create context menu shell key.", status, shell_key_path_);
  }

  auto written_menu = set_string_value(shell_key, nullptr, kMenuText);
  auto written_icon = written_menu ? set_string_value(shell_key, L"Icon", executable_path.wstring()) : written_menu;
  RegCloseKey(shell_key);
  if (!written_menu) {
    return written_menu;
  }
  if (!written_icon) {
    return written_icon;
  }

  const auto command_key_path = shell_key_path_ + L"\\command";
  HKEY command_key = nullptr;
  status = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    command_key_path.c_str(),
    0,
    nullptr,
    REG_OPTION_NON_VOLATILE,
    KEY_SET_VALUE,
    nullptr,
    &command_key,
    nullptr);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to create context menu command key.", status, command_key_path);
  }

  auto written_command = set_string_value(command_key, nullptr, command.value());
  RegCloseKey(command_key);
  return written_command;
}

core::Result<void> RegistryContextMenuService::uninstall() const {
  const auto status = RegDeleteTreeW(HKEY_CURRENT_USER, shell_key_path_.c_str());
  if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
    return {};
  }
  return registry_error(L"Failed to delete context menu registry key.", status, shell_key_path_);
}

} // namespace binify::platform::windows
