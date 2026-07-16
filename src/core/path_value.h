#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace binify::core {

struct PathEntry {
  std::wstring text;
};

struct PathUpdate {
  std::wstring value;
  bool changed = false;
};

[[nodiscard]] std::vector<PathEntry> split_path_value(std::wstring_view value);
[[nodiscard]] std::wstring join_path_entries(const std::vector<PathEntry>& entries);
[[nodiscard]] bool path_entry_matches(const std::wstring& entry_text, const std::filesystem::path& target_path);
[[nodiscard]] PathUpdate add_path_entry(std::wstring_view current_value, const std::filesystem::path& path_to_add);
[[nodiscard]] PathUpdate remove_path_entry(std::wstring_view current_value, const std::filesystem::path& path_to_remove);
[[nodiscard]] PathUpdate migrate_path_entry(
  std::wstring_view current_value,
  const std::filesystem::path& old_path,
  const std::filesystem::path& new_path);

} // namespace binify::core

