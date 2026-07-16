#include "platform/windows/file_logger.h"

#include <windows.h>

#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <system_error>

#include "core/error.h"
#include "core/text_encoding.h"

namespace binify::platform::windows {
namespace {

core::Error log_error(std::wstring message, const std::filesystem::path& path) {
  return core::make_error(core::ErrorCode::config_io_error, std::move(message), std::nullopt, path.wstring());
}

core::Result<std::wstring> environment_variable(const wchar_t* name) {
  wchar_t* raw_value = nullptr;
  std::size_t value_size = 0;
  if (_wdupenv_s(&raw_value, &value_size, name) != 0) {
    return core::make_error(core::ErrorCode::platform_error, L"Failed to read required environment variable.");
  }
  const std::unique_ptr<wchar_t, decltype(&std::free)> value{raw_value, std::free};
  if (value == nullptr || value.get()[0] == L'\0') {
    return core::make_error(core::ErrorCode::platform_error, L"Required environment variable is not set.");
  }
  return std::wstring{value.get()};
}

std::wstring_view level_text(app::LogLevel level) noexcept {
  switch (level) {
  case app::LogLevel::info:
    return L"INFO";
  case app::LogLevel::warning:
    return L"WARN";
  case app::LogLevel::error:
    return L"ERROR";
  }
  return L"INFO";
}

} // namespace

FileLogger::FileLogger(FileLoggerOptions options) : options_(std::move(options)) {}

core::Result<void> FileLogger::write(app::LogLevel level, std::wstring_view message) const {
  std::wstring line;
  line += L"[";
  line += level_text(level);
  line += L"] ";
  line += message;
  line += L"\r\n";

  auto encoded = core::wide_to_utf8(line);
  if (!encoded) {
    return encoded.error();
  }

  std::error_code error;
  std::filesystem::create_directories(options_.log_directory, error);
  if (error) {
    return log_error(L"Failed to create log directory.", options_.log_directory);
  }

  auto rotated = rotate_if_needed(encoded.value().size());
  if (!rotated) {
    return rotated.error();
  }

  std::ofstream stream{active_log_path(), std::ios::binary | std::ios::app};
  if (!stream) {
    return log_error(L"Failed to open log file.", active_log_path());
  }
  stream << encoded.value();
  stream.flush();
  if (!stream) {
    return log_error(L"Failed to write log file.", active_log_path());
  }

  return {};
}

const std::filesystem::path& FileLogger::log_directory() const noexcept {
  return options_.log_directory;
}

std::filesystem::path FileLogger::active_log_path() const {
  return options_.log_directory / L"binify.log";
}

std::filesystem::path FileLogger::rotated_log_path(std::size_t index) const {
  return options_.log_directory / (L"binify." + std::to_wstring(index) + L".log");
}

core::Result<void> FileLogger::rotate_if_needed(std::uintmax_t incoming_bytes) const {
  if (options_.max_files <= 1 || options_.max_file_size_bytes == 0) {
    return {};
  }

  std::error_code error;
  const auto active = active_log_path();
  const bool exists = std::filesystem::exists(active, error);
  if (error) {
    return log_error(L"Failed to inspect log file.", active);
  }
  if (!exists) {
    return {};
  }

  const auto current_size = std::filesystem::file_size(active, error);
  if (error) {
    return log_error(L"Failed to read log file size.", active);
  }
  if (current_size + incoming_bytes <= options_.max_file_size_bytes) {
    return {};
  }

  const auto oldest = rotated_log_path(options_.max_files - 1);
  std::filesystem::remove(oldest, error);
  if (error) {
    return log_error(L"Failed to remove oldest rotated log.", oldest);
  }

  for (std::size_t index = options_.max_files - 1; index > 1; --index) {
    const auto from = rotated_log_path(index - 1);
    const auto to = rotated_log_path(index);
    if (std::filesystem::exists(from, error)) {
      std::filesystem::rename(from, to, error);
      if (error) {
        return log_error(L"Failed to rotate log file.", from);
      }
    }
  }

  std::filesystem::rename(active, rotated_log_path(1), error);
  if (error) {
    return log_error(L"Failed to rotate active log file.", active);
  }

  return {};
}

core::Result<std::filesystem::path> default_log_directory() {
  auto local_app_data = environment_variable(L"LOCALAPPDATA");
  if (!local_app_data) {
    return local_app_data.error();
  }
  return std::filesystem::path{local_app_data.value()} / L"binify" / L"logs";
}

} // namespace binify::platform::windows
