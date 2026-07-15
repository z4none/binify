#pragma once

#include <filesystem>

namespace binify::core {

[[nodiscard]] std::filesystem::path normalize_path_lexically(const std::filesystem::path& path);
[[nodiscard]] std::filesystem::path remove_trailing_separators(const std::filesystem::path& path);
[[nodiscard]] bool path_equal_case_insensitive(
  const std::filesystem::path& left,
  const std::filesystem::path& right);

} // namespace binify::core

