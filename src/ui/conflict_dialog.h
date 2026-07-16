#pragma once

#include <filesystem>
#include <vector>

#include <windows.h>

namespace binify::ui {

[[nodiscard]] bool confirm_overwrite(HWND owner, const std::vector<std::filesystem::path>& conflict_paths);

} // namespace binify::ui
