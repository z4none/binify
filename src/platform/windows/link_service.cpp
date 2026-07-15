#include "platform/windows/link_service.h"

#include <windows.h>

#include <fstream>
#include <system_error>

#include "core/bat_wrapper.h"
#include "core/error.h"

namespace binify::platform::windows {

namespace {

core::Result<void> validate_source_file(const std::filesystem::path& source_path) {
  std::error_code error;
  if (!std::filesystem::is_regular_file(source_path, error)) {
    return core::make_error(
      core::ErrorCode::source_invalid,
      L"Source path is not a regular file.",
      error.value() == 0 ? std::nullopt : std::optional<unsigned long>{static_cast<unsigned long>(error.value())},
      source_path.wstring());
  }
  return {};
}

core::Error last_error(core::ErrorCode code, std::wstring message, const std::filesystem::path& path) {
  return core::make_error(code, std::move(message), GetLastError(), path.wstring());
}

void remove_if_exists(const std::filesystem::path& path) {
  std::error_code ignored;
  std::filesystem::remove(path, ignored);
}

core::Result<void> ensure_parent_directory(const std::filesystem::path& entry_path) {
  std::error_code error;
  std::filesystem::create_directories(entry_path.parent_path(), error);
  if (error) {
    return core::make_error(
      core::ErrorCode::link_creation_failed,
      L"Failed to create entry parent directory.",
      static_cast<unsigned long>(error.value()),
      entry_path.parent_path().wstring());
  }
  return {};
}

core::Result<void> validate_entry_created(const std::filesystem::path& entry_path) {
  std::error_code error;
  if (!std::filesystem::exists(entry_path, error)) {
    return core::make_error(
      core::ErrorCode::link_creation_failed,
      L"Entry was not created.",
      error.value() == 0 ? std::nullopt : std::optional<unsigned long>{static_cast<unsigned long>(error.value())},
      entry_path.wstring());
  }
  return {};
}

core::Result<void> validate_common_paths(
  const std::filesystem::path& source_path,
  const std::filesystem::path& entry_path) {
  auto source_valid = validate_source_file(source_path);
  if (!source_valid) {
    return source_valid;
  }
  if (entry_path.empty()) {
    return core::make_error(core::ErrorCode::invalid_argument, L"Entry path is required.");
  }
  return ensure_parent_directory(entry_path);
}

core::Result<void> same_volume_required(
  const std::filesystem::path& source_path,
  const std::filesystem::path& entry_path) {
  if (source_path.root_name() != entry_path.root_name()) {
    return core::make_error(
      core::ErrorCode::link_mode_unavailable,
      L"Hard link requires source and entry paths to be on the same volume.",
      std::nullopt,
      entry_path.wstring());
  }
  return {};
}

} // namespace

core::Result<void> WindowsLinkService::create_symbolic_link(
  const std::filesystem::path& source_path,
  const std::filesystem::path& entry_path) const {
  auto valid = validate_common_paths(source_path, entry_path);
  if (!valid) {
    return valid;
  }

  remove_if_exists(entry_path);
  if (!CreateSymbolicLinkW(
        entry_path.c_str(),
        source_path.c_str(),
        SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
    const auto first_error = GetLastError();
    if (!CreateSymbolicLinkW(entry_path.c_str(), source_path.c_str(), 0)) {
      remove_if_exists(entry_path);
      return core::make_error(
        core::ErrorCode::link_mode_unavailable,
        L"Failed to create symbolic link.",
        first_error,
        entry_path.wstring());
    }
  }

  auto created = validate_entry_created(entry_path);
  if (!created) {
    remove_if_exists(entry_path);
    return created;
  }
  return {};
}

core::Result<void> WindowsLinkService::create_hard_link(
  const std::filesystem::path& source_path,
  const std::filesystem::path& entry_path) const {
  auto valid = validate_common_paths(source_path, entry_path);
  if (!valid) {
    return valid;
  }
  auto same_volume = same_volume_required(source_path, entry_path);
  if (!same_volume) {
    return same_volume;
  }

  remove_if_exists(entry_path);
  if (!CreateHardLinkW(entry_path.c_str(), source_path.c_str(), nullptr)) {
    remove_if_exists(entry_path);
    return last_error(core::ErrorCode::link_mode_unavailable, L"Failed to create hard link.", entry_path);
  }

  auto created = validate_entry_created(entry_path);
  if (!created) {
    remove_if_exists(entry_path);
    return created;
  }
  return {};
}

core::Result<void> WindowsLinkService::create_cmd_wrapper(
  const std::filesystem::path& source_path,
  const std::filesystem::path& entry_path) const {
  auto valid = validate_common_paths(source_path, entry_path);
  if (!valid) {
    return valid;
  }

  auto content = core::generate_bat_wrapper(source_path);
  if (!content) {
    return content.error();
  }

  const auto temp_path = entry_path.parent_path() / (entry_path.filename().wstring() + L".tmp");
  {
    std::ofstream stream{temp_path, std::ios::binary | std::ios::trunc};
    if (!stream) {
      return core::make_error(
        core::ErrorCode::link_creation_failed,
        L"Failed to open temporary BAT wrapper.",
        std::nullopt,
        temp_path.wstring());
    }
    stream << content.value();
    stream.flush();
    if (!stream) {
      remove_if_exists(temp_path);
      return core::make_error(
        core::ErrorCode::link_creation_failed,
        L"Failed to write BAT wrapper.",
        std::nullopt,
        temp_path.wstring());
    }
  }

  if (!MoveFileExW(temp_path.c_str(), entry_path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    const auto native_code = GetLastError();
    remove_if_exists(temp_path);
    return core::make_error(
      core::ErrorCode::link_creation_failed,
      L"Failed to replace BAT wrapper.",
      native_code,
      entry_path.wstring());
  }

  return validate_entry_created(entry_path);
}

} // namespace binify::platform::windows
