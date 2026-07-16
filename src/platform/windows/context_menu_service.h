#pragma once

#include <string>

#include "app/context_menu_service.h"

namespace binify::platform::windows {

class RegistryContextMenuService final : public app::ContextMenuService {
public:
  RegistryContextMenuService();
  explicit RegistryContextMenuService(std::wstring shell_key_path);

  [[nodiscard]] core::Result<void> install(const std::filesystem::path& executable_path) const override;
  [[nodiscard]] core::Result<void> uninstall() const override;

private:
  std::wstring shell_key_path_;
};

} // namespace binify::platform::windows

