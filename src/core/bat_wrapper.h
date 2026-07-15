#pragma once

#include <filesystem>
#include <string>

#include "core/result.h"

namespace binify::core {

[[nodiscard]] Result<std::string> generate_bat_wrapper(const std::filesystem::path& source_exe);

} // namespace binify::core

