#include "ui/ui_runtime.h"

#include <system_error>

#include "core/error.h"

namespace binify::ui {
namespace {

std::wstring detect_system_language() {
  wchar_t locale_name[LOCALE_NAME_MAX_LENGTH]{};
  if (GetUserDefaultLocaleName(locale_name, LOCALE_NAME_MAX_LENGTH) == 0) {
    return std::wstring{core::kFallbackLanguageCode};
  }

  std::wstring locale = locale_name;
  if (locale.starts_with(L"zh")) {
    return L"zh-CN";
  }
  return locale;
}

std::wstring load_requested_language(const platform::windows::FileConfigStore& config_store) {
  const auto loaded = config_store.load();
  if (!loaded || !loaded.value()) {
    return std::wstring{core::kSystemLanguageCode};
  }
  return loaded.value()->language;
}

} // namespace

core::Result<std::vector<core::DirectoryEntry>> WindowsDirectoryListing::list_entries(
  const std::filesystem::path& directory) const {
  std::vector<core::DirectoryEntry> entries;
  std::error_code error_code;

  if (!std::filesystem::exists(directory, error_code)) {
    if (error_code) {
      return core::make_error(core::ErrorCode::path_invalid, L"Failed to inspect Bin directory.", error_code.value(), directory.wstring());
    }
    return entries;
  }

  for (const auto& entry : std::filesystem::directory_iterator(directory, error_code)) {
    if (error_code) {
      return core::make_error(core::ErrorCode::path_invalid, L"Failed to list Bin directory.", error_code.value(), directory.wstring());
    }
    entries.push_back(core::DirectoryEntry{
      .filename = entry.path().filename().wstring(),
      .path = entry.path(),
    });
  }

  return entries;
}

RuntimeContext::RuntimeContext(
  std::filesystem::path config_path,
  std::filesystem::path log_directory,
  std::filesystem::path language_directory,
  std::filesystem::path executable_path)
  : config_store(std::move(config_path)),
    language_packs(core::load_language_packs(language_directory)),
    system_language(detect_system_language()),
    translator(language_packs, load_requested_language(config_store), system_language),
    logger(platform::windows::FileLoggerOptions{.log_directory = std::move(log_directory)}),
    settings_workflow(config_store, path_service, context_menu_service),
    add_command_workflow(link_service, directory_listing),
    uninstall_workflow(path_service, context_menu_service),
    executable_path_(std::move(executable_path)) {}

const std::filesystem::path& RuntimeContext::executable_path() const noexcept {
  return executable_path_;
}

std::wstring RuntimeContext::text(std::string_view key) const {
  return translator.text(key);
}

std::wstring RuntimeContext::text_for_language(std::wstring_view language, std::string_view key) const {
  return core::Translator{language_packs, std::wstring{language}, system_language}.text(key);
}

std::wstring error_message(const core::Error& error) {
  std::wstring message{core::to_wstring(error.code)};
  if (!error.technical_message.empty()) {
    message += L"\r\n";
    message += error.technical_message;
  }
  if (error.native_code) {
    message += L"\r\nNative error: ";
    message += std::to_wstring(*error.native_code);
  }
  if (error.path) {
    message += L"\r\nPath: ";
    message += *error.path;
  }
  return message;
}

std::filesystem::path current_executable_path() {
  std::wstring buffer(MAX_PATH, L'\0');
  DWORD copied = 0;
  for (;;) {
    copied = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (copied == 0) {
      return {};
    }
    if (copied < buffer.size() - 1) {
      buffer.resize(copied);
      return buffer;
    }
    buffer.resize(buffer.size() * 2);
  }
}

core::Result<std::filesystem::path> default_config_path() {
  return platform::windows::default_config_path();
}

core::Result<std::filesystem::path> default_log_directory() {
  return platform::windows::default_log_directory();
}

std::filesystem::path default_language_directory(const std::filesystem::path& executable_path) {
  if (executable_path.empty()) {
    return L"lang";
  }
  return executable_path.parent_path() / L"lang";
}

void show_error(HWND owner, const core::Error& error) {
  const auto message = error_message(error);
  MessageBoxW(owner, message.c_str(), L"binify error", MB_ICONERROR | MB_OK);
}

void show_info(HWND owner, const std::wstring& message) {
  MessageBoxW(owner, message.c_str(), L"binify", MB_ICONINFORMATION | MB_OK);
}

} // namespace binify::ui
