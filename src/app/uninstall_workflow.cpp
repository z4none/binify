#include "app/uninstall_workflow.h"

namespace binify::app {

UninstallWorkflow::UninstallWorkflow(const PathService& path_service, const ContextMenuService& context_menu_service)
  : path_service_(path_service), context_menu_service_(context_menu_service) {}

core::Result<UninstallCleanupResult> UninstallWorkflow::cleanup(const UninstallCleanupRequest& request) const {
  UninstallCleanupResult result;

  if (request.uninstall_context_menu) {
    auto uninstalled = context_menu_service_.uninstall();
    if (!uninstalled) {
      return uninstalled.error();
    }
    result.context_menu_removed = true;
  }

  if (request.remove_path_entry && !request.config.bin_directory.empty()) {
    auto removed = path_service_.remove_user_path(request.config.bin_directory);
    if (!removed) {
      return removed.error();
    }
    result.path_removed = true;
  }

  return result;
}

} // namespace binify::app

