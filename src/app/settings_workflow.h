#pragma once

#include <filesystem>

#include "app/config_store.h"
#include "app/context_menu_service.h"
#include "app/path_service.h"

namespace binify::app {

struct SettingsSaveRequest {
  core::Config config;
  std::filesystem::path executable_path;
};

struct SettingsSaveResult {
  bool config_saved = false;
  bool path_changed = false;
  bool context_menu_changed = false;
};

class SettingsWorkflow {
public:
  SettingsWorkflow(
    const ConfigStore& config_store,
    const PathService& path_service,
    const ContextMenuService& context_menu_service);

  [[nodiscard]] core::Result<SettingsSaveResult> save(const SettingsSaveRequest& request) const;

private:
  const ConfigStore& config_store_;
  const PathService& path_service_;
  const ContextMenuService& context_menu_service_;
};

} // namespace binify::app

