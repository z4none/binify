#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <windows.h>

#include "app/add_command_workflow.h"
#include "app/settings_workflow.h"
#include "app/uninstall_workflow.h"
#include "platform/windows/config_store.h"
#include "platform/windows/context_menu_service.h"
#include "platform/windows/link_service.h"
#include "platform/windows/path_service.h"

namespace binify::ui {

class WindowsDirectoryListing final : public app::DirectoryListing {
public:
  [[nodiscard]] core::Result<std::vector<core::DirectoryEntry>> list_entries(
    const std::filesystem::path& directory) const override;
};

class RuntimeContext final {
public:
  RuntimeContext(std::filesystem::path config_path, std::filesystem::path executable_path);
  RuntimeContext(const RuntimeContext&) = delete;
  RuntimeContext& operator=(const RuntimeContext&) = delete;

  [[nodiscard]] const std::filesystem::path& executable_path() const noexcept;

  platform::windows::FileConfigStore config_store;
  platform::windows::RegistryPathService path_service;
  platform::windows::RegistryContextMenuService context_menu_service;
  platform::windows::WindowsLinkService link_service;
  WindowsDirectoryListing directory_listing;
  app::SettingsWorkflow settings_workflow;
  app::AddCommandWorkflow add_command_workflow;
  app::UninstallWorkflow uninstall_workflow;

private:
  std::filesystem::path executable_path_;
};

[[nodiscard]] std::wstring error_message(const core::Error& error);
[[nodiscard]] std::filesystem::path current_executable_path();
[[nodiscard]] core::Result<std::filesystem::path> default_config_path();
void show_error(HWND owner, const core::Error& error);
void show_info(HWND owner, const std::wstring& message);

} // namespace binify::ui
