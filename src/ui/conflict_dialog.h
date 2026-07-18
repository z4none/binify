#pragma once

#include <filesystem>
#include <vector>

#include <windows.h>

#include "core/language_pack.h"

namespace binify::ui {

[[nodiscard]] bool confirm_overwrite(
  HWND owner,
  const std::vector<std::filesystem::path>& conflict_paths,
  const core::Translator& translator);

} // namespace binify::ui
