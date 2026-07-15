#pragma once

#include <string>
#include <string_view>

#include "core/result.h"

namespace binify::core {

[[nodiscard]] Result<std::wstring> utf8_to_wide(std::string_view input);
[[nodiscard]] Result<std::string> wide_to_utf8(std::wstring_view input);

} // namespace binify::core

