#include <gtest/gtest.h>

#include "app/settings_workflow.h"

namespace {

using namespace binify::app;
using namespace binify::core;

class FakeConfigStore final : public ConfigStore {
public:
  Result<std::optional<Config>> load() const override {
    return loaded_config;
  }

  Result<void> save(const Config& config) const override {
    saved_config = config;
    save_called = true;
    return {};
  }

  mutable std::optional<Config> loaded_config;
  mutable std::optional<Config> saved_config;
  mutable bool save_called = false;
};

class FakePathService final : public PathService {
public:
  Result<void> add_user_path(const std::filesystem::path& path) const override {
    added_path = path;
    return {};
  }

  Result<void> remove_user_path(const std::filesystem::path& path) const override {
    removed_path = path;
    return {};
  }

  Result<void> migrate_user_path(const std::filesystem::path& old_path, const std::filesystem::path& new_path) const override {
    migrated_from = old_path;
    migrated_to = new_path;
    return {};
  }

  mutable std::filesystem::path added_path;
  mutable std::filesystem::path removed_path;
  mutable std::filesystem::path migrated_from;
  mutable std::filesystem::path migrated_to;
};

class FakeContextMenuService final : public ContextMenuService {
public:
  Result<void> install(const std::filesystem::path& executable_path) const override {
    installed_executable = executable_path;
    return {};
  }

  Result<void> uninstall() const override {
    uninstall_called = true;
    return {};
  }

  mutable std::filesystem::path installed_executable;
  mutable bool uninstall_called = false;
};

Config config(std::wstring bin_directory, bool path_enabled, bool context_menu_enabled) {
  Config value;
  value.bin_directory = std::move(bin_directory);
  value.path_enabled = path_enabled;
  value.context_menu_enabled = context_menu_enabled;
  return value;
}

TEST(SettingsWorkflowTests, SavesNewConfigAndAppliesEnabledIntegrations) {
  FakeConfigStore store;
  FakePathService path;
  FakeContextMenuService context_menu;
  const SettingsWorkflow workflow{store, path, context_menu};

  const auto result = workflow.save(SettingsSaveRequest{
    .config = config(L"C:\\Users\\zi\\bin", true, true),
    .executable_path = L"C:\\Program Files\\binify\\binify.exe",
  });

  ASSERT_TRUE(result);
  EXPECT_TRUE(store.save_called);
  EXPECT_EQ(path.added_path, L"C:\\Users\\zi\\bin");
  EXPECT_EQ(context_menu.installed_executable, L"C:\\Program Files\\binify\\binify.exe");
  EXPECT_TRUE(result.value().config_saved);
  EXPECT_TRUE(result.value().path_changed);
  EXPECT_TRUE(result.value().context_menu_changed);
}

TEST(SettingsWorkflowTests, MigratesPathWhenBinDirectoryChanges) {
  FakeConfigStore store;
  store.loaded_config = config(L"C:\\OldBin", true, false);
  FakePathService path;
  FakeContextMenuService context_menu;
  const SettingsWorkflow workflow{store, path, context_menu};

  const auto result = workflow.save(SettingsSaveRequest{
    .config = config(L"D:\\NewBin", true, false),
    .executable_path = L"C:\\Program Files\\binify\\binify.exe",
  });

  ASSERT_TRUE(result);
  EXPECT_EQ(path.migrated_from, L"C:\\OldBin");
  EXPECT_EQ(path.migrated_to, L"D:\\NewBin");
}

TEST(SettingsWorkflowTests, RemovesDisabledIntegrationsFromPreviousConfig) {
  FakeConfigStore store;
  store.loaded_config = config(L"C:\\OldBin", true, true);
  FakePathService path;
  FakeContextMenuService context_menu;
  const SettingsWorkflow workflow{store, path, context_menu};

  const auto result = workflow.save(SettingsSaveRequest{
    .config = config(L"C:\\OldBin", false, false),
    .executable_path = L"C:\\Program Files\\binify\\binify.exe",
  });

  ASSERT_TRUE(result);
  EXPECT_EQ(path.removed_path, L"C:\\OldBin");
  EXPECT_TRUE(context_menu.uninstall_called);
}

} // namespace

