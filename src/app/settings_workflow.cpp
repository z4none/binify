#include "app/settings_workflow.h"

namespace binify::app {

SettingsWorkflow::SettingsWorkflow(
  const ConfigStore& config_store,
  const PathService& path_service,
  const ContextMenuService& context_menu_service)
  : config_store_(config_store),
    path_service_(path_service),
    context_menu_service_(context_menu_service) {}

core::Result<SettingsSaveResult> SettingsWorkflow::save(const SettingsSaveRequest& request) const {
  auto valid = core::validate_config_model(request.config);
  if (!valid) {
    return valid.error();
  }

  auto loaded = config_store_.load();
  if (!loaded) {
    return loaded.error();
  }
  const auto previous = loaded.value();

  SettingsSaveResult result;
  if (request.config.path_enabled) {
    core::Result<void> path_result;
    if (previous && previous->path_enabled && previous->bin_directory != request.config.bin_directory) {
      path_result = path_service_.migrate_user_path(previous->bin_directory, request.config.bin_directory);
    } else {
      path_result = path_service_.add_user_path(request.config.bin_directory);
    }
    if (!path_result) {
      return path_result.error();
    }
    result.path_changed = true;
  } else if (previous && previous->path_enabled) {
    auto removed = path_service_.remove_user_path(previous->bin_directory);
    if (!removed) {
      return removed.error();
    }
    result.path_changed = true;
  }

  if (request.config.context_menu_enabled) {
    auto installed = context_menu_service_.install(request.executable_path);
    if (!installed) {
      return installed.error();
    }
    result.context_menu_changed = true;
  } else if (previous && previous->context_menu_enabled) {
    auto uninstalled = context_menu_service_.uninstall();
    if (!uninstalled) {
      return uninstalled.error();
    }
    result.context_menu_changed = true;
  }

  auto saved = config_store_.save(request.config);
  if (!saved) {
    return saved.error();
  }
  result.config_saved = true;
  return result;
}

} // namespace binify::app

