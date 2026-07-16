#pragma once

#include "app/shell_service.h"

namespace binify::platform::windows {

class WindowsShellService final : public app::ShellService {
public:
  [[nodiscard]] core::Result<void> open_directory(const std::filesystem::path& directory) const override;
};

} // namespace binify::platform::windows
