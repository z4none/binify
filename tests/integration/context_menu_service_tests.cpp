#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include <windows.h>

#include "platform/windows/context_menu_service.h"

namespace {

using binify::platform::windows::RegistryContextMenuService;

std::wstring unique_test_key() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return L"Software\\binify-tests\\context-menu-" + std::to_wstring(ticks) + L"\\shell\\binify";
}

std::wstring read_default_value(const std::wstring& key_path) {
  HKEY key = nullptr;
  const auto status = RegOpenKeyExW(HKEY_CURRENT_USER, key_path.c_str(), 0, KEY_QUERY_VALUE, &key);
  if (status != ERROR_SUCCESS) {
    return {};
  }

  DWORD type = 0;
  DWORD bytes = 0;
  RegQueryValueExW(key, nullptr, nullptr, &type, nullptr, &bytes);
  std::wstring value(bytes / sizeof(wchar_t), L'\0');
  RegQueryValueExW(key, nullptr, nullptr, &type, reinterpret_cast<LPBYTE>(value.data()), &bytes);
  RegCloseKey(key);
  while (!value.empty() && value.back() == L'\0') {
    value.pop_back();
  }
  return value;
}

bool key_exists(const std::wstring& key_path) {
  HKEY key = nullptr;
  const auto status = RegOpenKeyExW(HKEY_CURRENT_USER, key_path.c_str(), 0, KEY_QUERY_VALUE, &key);
  if (status == ERROR_SUCCESS) {
    RegCloseKey(key);
    return true;
  }
  return false;
}

TEST(ContextMenuServiceTests, InstallsAndUninstallsContextMenuUnderCustomKey) {
  const auto key_path = unique_test_key();
  const auto command_key_path = key_path + L"\\command";
  const RegistryContextMenuService service{key_path};

  const auto installed = service.install(L"C:\\Program Files\\binify\\binify.exe");

  ASSERT_TRUE(installed);
  EXPECT_EQ(read_default_value(key_path), L"加入 Binify...");
  EXPECT_EQ(read_default_value(command_key_path), L"\"C:\\Program Files\\binify\\binify.exe\" --add \"%1\"");

  const auto uninstalled = service.uninstall();

  ASSERT_TRUE(uninstalled);
  EXPECT_FALSE(key_exists(key_path));
}

TEST(ContextMenuServiceTests, UninstallIsIdempotent) {
  const auto key_path = unique_test_key();
  const RegistryContextMenuService service{key_path};

  EXPECT_TRUE(service.uninstall());
  EXPECT_TRUE(service.uninstall());
}

} // namespace

