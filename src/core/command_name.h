#pragma once

#include <string>

#include "core/creation_policy.h"
#include "core/result.h"

namespace binify::core {

struct CommandName {
  std::wstring normalized;
};

[[nodiscard]] Result<CommandName> normalize_command_name(std::wstring input);
[[nodiscard]] Result<void> validate_command_name(const std::wstring& normalized_name);
[[nodiscard]] std::wstring final_entry_name(const CommandName& command_name, LinkMode mode);
[[nodiscard]] std::wstring final_entry_name(const std::wstring& normalized_name, LinkMode mode);
[[nodiscard]] bool is_reserved_device_name(const std::wstring& name);

} // namespace binify::core

