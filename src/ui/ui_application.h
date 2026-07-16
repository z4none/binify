#pragma once

#include <string>
#include <vector>

#include <windows.h>

namespace binify::ui {

[[nodiscard]] int run_ui(HINSTANCE instance, int command_show, const std::vector<std::wstring>& arguments) noexcept;

} // namespace binify::ui
