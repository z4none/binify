#pragma once

#include <filesystem>

#include "core/result.h"

namespace binify::app {

class ShellService {
public:
  virtual ~ShellService() = default;

  [[nodiscard]] virtual core::Result<void> open_directory(const std::filesystem::path& directory) const = 0;
};

} // namespace binify::app
