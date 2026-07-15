#include "core/path_utils.h"

#include <algorithm>
#include <cwctype>
#include <string>

namespace binify::core {

namespace {

std::wstring lower_ascii_path(std::filesystem::path path) {
  auto text = remove_trailing_separators(path.lexically_normal()).wstring();
  std::ranges::replace(text, L'/', L'\\');
  std::ranges::transform(text, text.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towlower(ch));
  });
  return text;
}

} // namespace

std::filesystem::path normalize_path_lexically(const std::filesystem::path& path) {
  return remove_trailing_separators(path.lexically_normal());
}

std::filesystem::path remove_trailing_separators(const std::filesystem::path& path) {
  auto text = path.wstring();
  while (text.size() > 1 && (text.back() == L'\\' || text.back() == L'/')) {
    const std::filesystem::path candidate{text.substr(0, text.size() - 1)};
    if (candidate.has_root_path() && candidate.root_path() == candidate) {
      break;
    }
    text.pop_back();
  }
  return std::filesystem::path{text};
}

bool path_equal_case_insensitive(
  const std::filesystem::path& left,
  const std::filesystem::path& right) {
  return lower_ascii_path(left) == lower_ascii_path(right);
}

} // namespace binify::core

