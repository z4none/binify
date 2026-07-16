#pragma once

#include <filesystem>

#include "core/result.h"

namespace binify::app {

class ContextMenuService {
public:
  virtual ~ContextMenuService() = default;

  [[nodiscard]] virtual core::Result<void> install(const std::filesystem::path& executable_path) const = 0;
  [[nodiscard]] virtual core::Result<void> uninstall() const = 0;
};

} // namespace binify::app

