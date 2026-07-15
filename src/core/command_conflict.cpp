#include "core/command_conflict.h"

#include <algorithm>
#include <array>
#include <cwctype>

namespace binify::core {

namespace {

std::wstring lower(std::wstring value) {
  std::ranges::transform(value, value.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towlower(ch));
  });
  return value;
}

std::wstring extension_lower(const std::wstring& filename) {
  return lower(std::filesystem::path{filename}.extension().wstring());
}

std::wstring stem_lower(const std::wstring& filename) {
  return lower(std::filesystem::path{filename}.stem().wstring());
}

bool is_conflict_extension(const std::wstring& extension) {
  const auto extensions = command_conflict_extensions();
  return std::ranges::any_of(extensions, [&](std::wstring_view candidate) {
    return extension == candidate;
  });
}

std::size_t extension_rank(const std::filesystem::path& path) {
  const auto extension = lower(path.extension().wstring());
  const auto extensions = command_conflict_extensions();
  for (std::size_t index = 0; index < extensions.size(); ++index) {
    if (extension == extensions[index]) {
      return index;
    }
  }
  return extensions.size();
}

} // namespace

std::span<const std::wstring_view> command_conflict_extensions() noexcept {
  static constexpr std::array<std::wstring_view, 4> kExtensions{
    L".exe",
    L".com",
    L".bat",
    L".cmd",
  };
  return kExtensions;
}

std::vector<std::filesystem::path> find_command_conflicts(
  const std::wstring& normalized_command_name,
  std::span<const DirectoryEntry> entries) {
  const auto expected_stem = lower(normalized_command_name);
  std::vector<std::filesystem::path> conflicts;

  for (const auto& entry : entries) {
    if (stem_lower(entry.filename) == expected_stem && is_conflict_extension(extension_lower(entry.filename))) {
      conflicts.push_back(entry.path);
    }
  }

  std::ranges::sort(conflicts, [](const std::filesystem::path& left, const std::filesystem::path& right) {
    const auto left_rank = extension_rank(left);
    const auto right_rank = extension_rank(right);
    if (left_rank != right_rank) {
      return left_rank < right_rank;
    }
    return lower(left.wstring()) < lower(right.wstring());
  });
  return conflicts;
}

bool has_command_conflict(
  const std::wstring& normalized_command_name,
  std::span<const DirectoryEntry> entries) {
  return !find_command_conflicts(normalized_command_name, entries).empty();
}

} // namespace binify::core
