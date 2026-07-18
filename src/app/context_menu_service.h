#pragma once

#include <filesystem>
#include <string>

#include "core/result.h"

namespace binify::app {

class ContextMenuService {
public:
  virtual ~ContextMenuService() = default;

  [[nodiscard]] virtual core::Result<void> install(
    const std::filesystem::path& executable_path,
    const std::wstring& menu_text) const = 0;
  [[nodiscard]] virtual core::Result<void> uninstall() const = 0;
};

} // namespace binify::app
