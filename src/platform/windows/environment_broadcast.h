#pragma once

#include "core/result.h"

namespace binify::platform::windows {

[[nodiscard]] core::Result<void> broadcast_environment_change();

} // namespace binify::platform::windows

