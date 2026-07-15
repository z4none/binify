#include "core/command_name.h"

#include <algorithm>
#include <array>
#include <cwctype>
#include <string_view>

#include "core/error.h"

namespace binify::core {

namespace {

constexpr std::wstring_view kInvalidFileNameChars = L"<>:\"/\\|?*";

bool is_ascii_case_insensitive_equal(std::wstring_view left, std::wstring_view right) {
  if (left.size() != right.size()) {
    return false;
  }

  for (std::size_t index = 0; index < left.size(); ++index) {
    if (std::towlower(left[index]) != std::towlower(right[index])) {
      return false;
    }
  }
  return true;
}

void trim_in_place(std::wstring& value) {
  const auto first = std::ranges::find_if(value, [](wchar_t ch) {
    return !std::iswspace(ch);
  });
  value.erase(value.begin(), first);

  const auto last = std::ranges::find_if(value.rbegin(), value.rend(), [](wchar_t ch) {
    return !std::iswspace(ch);
  });
  value.erase(last.base(), value.end());
}

bool ends_with_exe(std::wstring_view value) {
  if (value.size() < 4) {
    return false;
  }
  return is_ascii_case_insensitive_equal(value.substr(value.size() - 4), L".exe");
}

std::wstring base_name_for_reserved_check(std::wstring_view value) {
  const auto dot = value.find(L'.');
  std::wstring base{value.substr(0, dot)};
  std::ranges::transform(base, base.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towupper(ch));
  });
  return base;
}

} // namespace

Result<CommandName> normalize_command_name(std::wstring input) {
  trim_in_place(input);
  if (ends_with_exe(input)) {
    input.erase(input.size() - 4);
    trim_in_place(input);
  }

  auto valid = validate_command_name(input);
  if (!valid) {
    return valid.error();
  }

  return CommandName{.normalized = std::move(input)};
}

Result<void> validate_command_name(const std::wstring& normalized_name) {
  if (normalized_name.empty()) {
    return make_error(ErrorCode::command_name_empty, L"Command name cannot be empty.");
  }

  if (normalized_name.back() == L'.' || std::iswspace(normalized_name.back())) {
    return make_error(
      ErrorCode::command_name_invalid_suffix,
      L"Command name cannot end with a space or period.");
  }

  if (normalized_name.find_first_of(kInvalidFileNameChars) != std::wstring::npos) {
    return make_error(
      ErrorCode::command_name_invalid_character,
      L"Command name contains a character that is not valid in Windows file names.");
  }

  if (is_reserved_device_name(normalized_name)) {
    return make_error(
      ErrorCode::command_name_reserved,
      L"Command name cannot use a Windows reserved device name.");
  }

  return {};
}

std::wstring final_entry_name(const CommandName& command_name, LinkMode mode) {
  return final_entry_name(command_name.normalized, mode);
}

std::wstring final_entry_name(const std::wstring& normalized_name, LinkMode mode) {
  std::wstring result = normalized_name;
  result += entry_extension(mode);
  return result;
}

bool is_reserved_device_name(const std::wstring& name) {
  static constexpr std::array<std::wstring_view, 4> kReservedNames{
    L"CON",
    L"PRN",
    L"AUX",
    L"NUL",
  };

  const auto base = base_name_for_reserved_check(name);
  if (std::ranges::find(kReservedNames, base) != kReservedNames.end()) {
    return true;
  }

  if (base.size() == 4) {
    const auto prefix = base.substr(0, 3);
    const auto digit = base[3];
    return (prefix == L"COM" || prefix == L"LPT") && digit >= L'1' && digit <= L'9';
  }

  return false;
}

} // namespace binify::core

