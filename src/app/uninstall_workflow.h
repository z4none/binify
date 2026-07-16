#pragma once

#include "app/context_menu_service.h"
#include "app/path_service.h"
#include "core/config.h"

namespace binify::app {

struct UninstallCleanupRequest {
  core::Config config;
  bool remove_path_entry = true;
  bool uninstall_context_menu = true;
};

struct UninstallCleanupResult {
  bool path_removed = false;
  bool context_menu_removed = false;
};

class UninstallWorkflow {
public:
  UninstallWorkflow(const PathService& path_service, const ContextMenuService& context_menu_service);

  [[nodiscard]] core::Result<UninstallCleanupResult> cleanup(const UninstallCleanupRequest& request) const;

private:
  const PathService& path_service_;
  const ContextMenuService& context_menu_service_;
};

} // namespace binify::app

