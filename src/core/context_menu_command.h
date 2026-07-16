#pragma once

#include <filesystem>
#include <string>

#include "core/result.h"

namespace binify::core {

[[nodiscard]] Result<std::wstring> format_context_menu_command(const std::filesystem::path& executable_path);

} // namespace binify::core

