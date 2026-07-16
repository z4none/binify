#include "platform/windows/path_service.h"

#include <windows.h>

#include <string>

#include "core/error.h"
#include "core/path_value.h"
#include "platform/windows/environment_broadcast.h"

namespace binify::platform::windows {

namespace {

constexpr const wchar_t* kEnvironmentSubkey = L"Environment";
constexpr const wchar_t* kPathValueName = L"Path";

core::Error registry_error(std::wstring message, LSTATUS status) {
  return core::make_error(
    core::ErrorCode::path_registry_error,
    std::move(message),
    static_cast<unsigned long>(status),
    L"HKCU\\Environment\\Path");
}

core::Result<std::wstring> read_user_path_value() {
  HKEY key = nullptr;
  auto status = RegOpenKeyExW(HKEY_CURRENT_USER, kEnvironmentSubkey, 0, KEY_QUERY_VALUE, &key);
  if (status == ERROR_FILE_NOT_FOUND) {
    return std::wstring{};
  }
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to open HKCU Environment key for reading.", status);
  }

  DWORD type = 0;
  DWORD bytes = 0;
  status = RegQueryValueExW(key, kPathValueName, nullptr, &type, nullptr, &bytes);
  if (status == ERROR_FILE_NOT_FOUND) {
    RegCloseKey(key);
    return std::wstring{};
  }
  if (status != ERROR_SUCCESS) {
    RegCloseKey(key);
    return registry_error(L"Failed to query user PATH size.", status);
  }
  if (type != REG_SZ && type != REG_EXPAND_SZ) {
    RegCloseKey(key);
    return core::make_error(core::ErrorCode::path_registry_error, L"User PATH registry value has unsupported type.");
  }

  std::wstring value(bytes / sizeof(wchar_t), L'\0');
  status = RegQueryValueExW(key, kPathValueName, nullptr, &type, reinterpret_cast<LPBYTE>(value.data()), &bytes);
  RegCloseKey(key);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to read user PATH value.", status);
  }

  while (!value.empty() && value.back() == L'\0') {
    value.pop_back();
  }
  return value;
}

core::Result<void> write_user_path_value(const std::wstring& value) {
  HKEY key = nullptr;
  auto status = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    kEnvironmentSubkey,
    0,
    nullptr,
    REG_OPTION_NON_VOLATILE,
    KEY_SET_VALUE,
    nullptr,
    &key,
    nullptr);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to open HKCU Environment key for writing.", status);
  }

  const auto bytes = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
  status = RegSetValueExW(
    key,
    kPathValueName,
    0,
    REG_EXPAND_SZ,
    reinterpret_cast<const BYTE*>(value.c_str()),
    bytes);
  RegCloseKey(key);
  if (status != ERROR_SUCCESS) {
    return registry_error(L"Failed to write user PATH value.", status);
  }

  return broadcast_environment_change();
}

core::Result<void> apply_update(const core::PathUpdate& update) {
  if (!update.changed) {
    return {};
  }
  return write_user_path_value(update.value);
}

} // namespace

core::Result<void> RegistryPathService::add_user_path(const std::filesystem::path& path) const {
  auto current = read_user_path_value();
  if (!current) {
    return current.error();
  }
  return apply_update(core::add_path_entry(current.value(), path));
}

core::Result<void> RegistryPathService::remove_user_path(const std::filesystem::path& path) const {
  auto current = read_user_path_value();
  if (!current) {
    return current.error();
  }
  return apply_update(core::remove_path_entry(current.value(), path));
}

core::Result<void> RegistryPathService::migrate_user_path(
  const std::filesystem::path& old_path,
  const std::filesystem::path& new_path) const {
  auto current = read_user_path_value();
  if (!current) {
    return current.error();
  }
  return apply_update(core::migrate_path_entry(current.value(), old_path, new_path));
}

} // namespace binify::platform::windows

