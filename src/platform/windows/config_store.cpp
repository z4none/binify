#include "platform/windows/config_store.h"

#include <windows.h>

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <system_error>

#include "core/error.h"

namespace binify::platform::windows {

namespace {

core::Error io_error(std::wstring message, const std::filesystem::path& path) {
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

std::filesystem::path unique_temp_path(const std::filesystem::path& target) {
  const auto filename = target.filename().wstring() + L".tmp." + std::to_wstring(GetCurrentProcessId());
  return target.parent_path() / filename;
}

} // namespace

FileConfigStore::FileConfigStore(std::filesystem::path config_path)
  : config_path_(std::move(config_path)) {}

core::Result<std::optional<core::Config>> FileConfigStore::load() const {
  std::error_code error;
  if (!std::filesystem::exists(config_path_, error)) {
    return std::optional<core::Config>{};
  }
  if (error) {
    return io_error(L"Failed to check config file existence.", config_path_);
  }

  std::ifstream stream{config_path_, std::ios::binary};
  if (!stream) {
    return io_error(L"Failed to open config file for reading.", config_path_);
  }

  const std::string json_text{
    std::istreambuf_iterator<char>{stream},
    std::istreambuf_iterator<char>{},
  };

  auto config = core::deserialize_config_from_json(json_text);
  if (!config) {
    return config.error();
  }

  return std::optional<core::Config>{config.value()};
}

core::Result<void> FileConfigStore::save(const core::Config& config) const {
  auto json_text = core::serialize_config_to_json(config);
  if (!json_text) {
    return json_text.error();
  }

  std::error_code error;
  std::filesystem::create_directories(config_path_.parent_path(), error);
  if (error) {
    return io_error(L"Failed to create config directory.", config_path_.parent_path());
  }

  const auto temp_path = unique_temp_path(config_path_);
  {
    std::ofstream stream{temp_path, std::ios::binary | std::ios::trunc};
    if (!stream) {
      return io_error(L"Failed to open temporary config file for writing.", temp_path);
    }
    stream << json_text.value();
    stream.flush();
    if (!stream) {
      return io_error(L"Failed to write temporary config file.", temp_path);
    }
  }

  if (!MoveFileExW(
        temp_path.c_str(),
        config_path_.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    const auto native_code = GetLastError();
    std::filesystem::remove(temp_path, error);
    return core::make_error(
      core::ErrorCode::config_io_error,
      L"Failed to replace config file.",
      native_code,
      config_path_.wstring());
  }

  return {};
}

const std::filesystem::path& FileConfigStore::config_path() const noexcept {
  return config_path_;
}

core::Result<std::filesystem::path> default_config_path() {
  auto local_app_data = environment_variable(L"LOCALAPPDATA");
  if (!local_app_data) {
    return local_app_data.error();
  }
  return std::filesystem::path{local_app_data.value()} / L"binify" / L"config.json";
}

} // namespace binify::platform::windows
