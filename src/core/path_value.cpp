#include "core/path_value.h"

#include <algorithm>
#include <cwctype>

#include "core/path_utils.h"

namespace binify::core {

namespace {

std::wstring trim(std::wstring_view value) {
  auto first = value.begin();
  while (first != value.end() && std::iswspace(*first)) {
    ++first;
  }

  auto last = value.end();
  while (last != first && std::iswspace(*(last - 1))) {
    --last;
  }

  return std::wstring{first, last};
}

std::wstring normalized_for_compare(std::wstring text) {
  text = trim(text);
  if (text.size() >= 2 && text.front() == L'"' && text.back() == L'"') {
    text = text.substr(1, text.size() - 2);
  }
  return remove_trailing_separators(std::filesystem::path{text}.lexically_normal()).wstring();
}

} // namespace

std::vector<PathEntry> split_path_value(std::wstring_view value) {
  std::vector<PathEntry> entries;
  std::size_t start = 0;
  while (start <= value.size()) {
    const auto separator = value.find(L';', start);
    const auto end = separator == std::wstring_view::npos ? value.size() : separator;
    const auto entry = trim(value.substr(start, end - start));
    if (!entry.empty()) {
      entries.push_back(PathEntry{.text = entry});
    }
    if (separator == std::wstring_view::npos) {
      break;
    }
    start = separator + 1;
  }
  return entries;
}

std::wstring join_path_entries(const std::vector<PathEntry>& entries) {
  std::wstring value;
  for (std::size_t index = 0; index < entries.size(); ++index) {
    if (index > 0) {
      value += L';';
    }
    value += entries[index].text;
  }
  return value;
}

bool path_entry_matches(const std::wstring& entry_text, const std::filesystem::path& target_path) {
  return path_equal_case_insensitive(normalized_for_compare(entry_text), normalize_path_lexically(target_path));
}

PathUpdate add_path_entry(std::wstring_view current_value, const std::filesystem::path& path_to_add) {
  auto entries = split_path_value(current_value);
  if (std::ranges::any_of(entries, [&](const PathEntry& entry) {
        return path_entry_matches(entry.text, path_to_add);
      })) {
    return PathUpdate{.value = join_path_entries(entries), .changed = false};
  }

  entries.push_back(PathEntry{.text = normalize_path_lexically(path_to_add).wstring()});
  return PathUpdate{.value = join_path_entries(entries), .changed = true};
}

PathUpdate remove_path_entry(std::wstring_view current_value, const std::filesystem::path& path_to_remove) {
  auto entries = split_path_value(current_value);
  const auto original_size = entries.size();
  std::erase_if(entries, [&](const PathEntry& entry) {
    return path_entry_matches(entry.text, path_to_remove);
  });
  return PathUpdate{.value = join_path_entries(entries), .changed = entries.size() != original_size};
}

PathUpdate migrate_path_entry(
  std::wstring_view current_value,
  const std::filesystem::path& old_path,
  const std::filesystem::path& new_path) {
  auto removed = remove_path_entry(current_value, old_path);
  auto added = add_path_entry(removed.value, new_path);
  return PathUpdate{.value = added.value, .changed = removed.changed || added.changed};
}

} // namespace binify::core

