#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace binify::core {

struct DirectoryEntry {
  std::wstring filename;
  std::filesystem::path path;
};

struct CommandConflict {
  std::wstring normalized_command_name;
  std::vector<std::filesystem::path> conflicting_paths;
};

[[nodiscard]] std::span<const std::wstring_view> command_conflict_extensions() noexcept;
[[nodiscard]] std::vector<std::filesystem::path> find_command_conflicts(
  const std::wstring& normalized_command_name,
  std::span<const DirectoryEntry> entries);
[[nodiscard]] bool has_command_conflict(
  const std::wstring& normalized_command_name,
  std::span<const DirectoryEntry> entries);

} // namespace binify::core

