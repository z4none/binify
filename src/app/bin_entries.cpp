#include "app/bin_entries.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <system_error>
#include <cwchar>

#include "core/command_name.h"
#include "core/error.h"
#include "core/text_encoding.h"

namespace binify::app {
namespace {

std::optional<unsigned long> native_code(const std::error_code& error) {
  if (!error) {
    return std::nullopt;
  }
  return static_cast<unsigned long>(error.value());
}

std::wstring unescape_batch_percent(std::wstring value) {
  std::wstring unescaped;
  unescaped.reserve(value.size());
  for (std::size_t index = 0; index < value.size(); ++index) {
    if (value[index] == L'%' && index + 1 < value.size() && value[index + 1] == L'%') {
      unescaped.push_back(L'%');
      ++index;
    } else {
      unescaped.push_back(value[index]);
    }
  }
  return unescaped;
}

std::optional<std::filesystem::path> read_cmd_wrapper_target(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return std::nullopt;
  }

  const std::string content{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
  return parse_cmd_wrapper_target(content);
}

BinEntryType classify_regular_file(const std::filesystem::path& path, std::optional<std::filesystem::path>& target) {
  const auto extension = path.extension().wstring();
  if (_wcsicmp(extension.c_str(), L".bat") == 0 || _wcsicmp(extension.c_str(), L".cmd") == 0) {
    target = read_cmd_wrapper_target(path);
    if (target) {
      return BinEntryType::cmd_wrapper;
    }
  }

  if (_wcsicmp(extension.c_str(), L".exe") == 0) {
    return BinEntryType::hard_link;
  }

  return BinEntryType::file;
}

core::Result<void> validate_entry_path(const std::filesystem::path& entry_path) {
  if (entry_path.empty() || !entry_path.is_absolute()) {
    return core::make_error(core::ErrorCode::path_invalid, L"Entry path must be absolute.", std::nullopt, entry_path.wstring());
  }
  return {};
}

} // namespace

std::wstring_view display_name(BinEntryType type) noexcept {
  switch (type) {
  case BinEntryType::symbolic_link:
    return L"Symbolic Link";
  case BinEntryType::hard_link:
    return L"Hard Link";
  case BinEntryType::cmd_wrapper:
    return L"CMD Wrapper";
  case BinEntryType::file:
    return L"File";
  }
  return L"File";
}

std::optional<std::filesystem::path> parse_cmd_wrapper_target(std::string_view content) {
  auto wide = core::utf8_to_wide(std::string{content});
  if (!wide) {
    return std::nullopt;
  }

  constexpr std::wstring_view prefix = L"@echo off\r\n\"";
  constexpr std::wstring_view suffix = L"\" %*\r\nexit /b %ERRORLEVEL%\r\n";
  const auto& text = wide.value();
  if (!text.starts_with(prefix) || !text.ends_with(suffix)) {
    return std::nullopt;
  }

  const auto target_start = prefix.size();
  const auto target_length = text.size() - prefix.size() - suffix.size();
  if (target_length == 0) {
    return std::nullopt;
  }

  return std::filesystem::path{unescape_batch_percent(text.substr(target_start, target_length))};
}

core::Result<std::vector<BinEntry>> StdBinEntryFileSystem::scan(const std::filesystem::path& bin_directory) const {
  if (bin_directory.empty() || !bin_directory.is_absolute()) {
    return core::make_error(core::ErrorCode::path_invalid, L"Bin directory must be absolute.", std::nullopt, bin_directory.wstring());
  }

  std::error_code error;
  if (!std::filesystem::exists(bin_directory, error) || !std::filesystem::is_directory(bin_directory, error)) {
    return core::make_error(
      core::ErrorCode::path_invalid,
      L"Bin directory does not exist.",
      native_code(error),
      bin_directory.wstring());
  }

  std::vector<BinEntry> entries;
  for (const auto& entry : std::filesystem::directory_iterator(bin_directory, error)) {
    if (error) {
      return core::make_error(core::ErrorCode::path_invalid, L"Failed to scan Bin directory.", native_code(error), bin_directory.wstring());
    }
    if (!entry.is_regular_file(error) && !entry.is_symlink(error)) {
      continue;
    }

    std::optional<std::filesystem::path> target;
    BinEntryType type = BinEntryType::file;
    if (entry.is_symlink(error)) {
      type = BinEntryType::symbolic_link;
      target = std::filesystem::read_symlink(entry.path(), error);
      if (error) {
        target.reset();
        error.clear();
      }
    } else {
      type = classify_regular_file(entry.path(), target);
    }

    entries.push_back(BinEntry{
      .name = entry.path().filename().wstring(),
      .type = type,
      .path = entry.path(),
      .target = std::move(target),
      .modified_time = entry.last_write_time(error),
    });
    if (error) {
      error.clear();
    }
  }

  std::ranges::sort(entries, [](const BinEntry& left, const BinEntry& right) {
    return _wcsicmp(left.name.c_str(), right.name.c_str()) < 0;
  });
  return entries;
}

core::Result<void> StdBinEntryFileSystem::rename_entry(
  const std::filesystem::path& entry_path,
  const std::wstring& new_name) const {
  auto valid_path = validate_entry_path(entry_path);
  if (!valid_path) {
    return valid_path;
  }

  auto command_name = core::normalize_command_name(new_name);
  if (!command_name) {
    return command_name.error();
  }

  const auto new_path = entry_path.parent_path() / (command_name.value().normalized + entry_path.extension().wstring());
  std::error_code error;
  if (std::filesystem::exists(new_path, error)) {
    return core::make_error(core::ErrorCode::entry_transaction_failed, L"Target entry already exists.", native_code(error), new_path.wstring());
  }

  std::filesystem::rename(entry_path, new_path, error);
  if (error) {
    return core::make_error(core::ErrorCode::entry_transaction_failed, L"Failed to rename entry.", native_code(error), entry_path.wstring());
  }
  return {};
}

core::Result<void> StdBinEntryFileSystem::delete_entry(const std::filesystem::path& entry_path) const {
  auto valid_path = validate_entry_path(entry_path);
  if (!valid_path) {
    return valid_path;
  }

  std::error_code error;
  if (!std::filesystem::remove(entry_path, error)) {
    return core::make_error(core::ErrorCode::entry_transaction_failed, L"Entry file was not deleted.", native_code(error), entry_path.wstring());
  }
  if (error) {
    return core::make_error(core::ErrorCode::entry_transaction_failed, L"Failed to delete entry file.", native_code(error), entry_path.wstring());
  }
  return {};
}

} // namespace binify::app
