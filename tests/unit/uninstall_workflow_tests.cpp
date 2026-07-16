#include <gtest/gtest.h>

#include "app/uninstall_workflow.h"
#include "core/error.h"

namespace {

using namespace binify::app;
using namespace binify::core;

class FakePathService final : public PathService {
public:
  Result<void> add_user_path(const std::filesystem::path&) const override {
    return {};
  }

  Result<void> remove_user_path(const std::filesystem::path& path) const override {
    removed_path = path;
    if (fail_remove) {
      return make_error(ErrorCode::path_registry_error, L"Injected PATH remove failure.");
    }
    return {};
  }

  Result<void> migrate_user_path(const std::filesystem::path&, const std::filesystem::path&) const override {
    return {};
  }

  mutable std::filesystem::path removed_path;
  bool fail_remove = false;
};

class FakeContextMenuService final : public ContextMenuService {
public:
  Result<void> install(const std::filesystem::path&) const override {
    return {};
  }

  Result<void> uninstall() const override {
    uninstall_called = true;
    if (fail_uninstall) {
      return make_error(ErrorCode::context_menu_registry_error, L"Injected context menu uninstall failure.");
    }
    return {};
  }

  mutable bool uninstall_called = false;
  bool fail_uninstall = false;
};

Config config_with_bin(std::wstring bin_directory) {
  Config value;
  value.bin_directory = std::move(bin_directory);
  value.path_enabled = true;
  value.context_menu_enabled = true;
  return value;
}

TEST(UninstallWorkflowTests, RemovesContextMenuAndPathWhenRequested) {
  FakePathService path;
  FakeContextMenuService context_menu;
  const UninstallWorkflow workflow{path, context_menu};

  const auto result = workflow.cleanup(UninstallCleanupRequest{
    .config = config_with_bin(L"C:\\Users\\zi\\bin"),
    .remove_path_entry = true,
    .uninstall_context_menu = true,
  });

  ASSERT_TRUE(result);
  EXPECT_EQ(path.removed_path, L"C:\\Users\\zi\\bin");
  EXPECT_TRUE(context_menu.uninstall_called);
  EXPECT_TRUE(result.value().path_removed);
  EXPECT_TRUE(result.value().context_menu_removed);
}

TEST(UninstallWorkflowTests, KeepsPathWhenUserDeclinesPathCleanup) {
  FakePathService path;
  FakeContextMenuService context_menu;
  const UninstallWorkflow workflow{path, context_menu};

  const auto result = workflow.cleanup(UninstallCleanupRequest{
    .config = config_with_bin(L"C:\\Users\\zi\\bin"),
    .remove_path_entry = false,
    .uninstall_context_menu = true,
  });

  ASSERT_TRUE(result);
  EXPECT_TRUE(path.removed_path.empty());
  EXPECT_TRUE(context_menu.uninstall_called);
  EXPECT_FALSE(result.value().path_removed);
  EXPECT_TRUE(result.value().context_menu_removed);
}

TEST(UninstallWorkflowTests, KeepsContextMenuWhenUserDeclinesContextMenuCleanup) {
  FakePathService path;
  FakeContextMenuService context_menu;
  const UninstallWorkflow workflow{path, context_menu};

  const auto result = workflow.cleanup(UninstallCleanupRequest{
    .config = config_with_bin(L"C:\\Users\\zi\\bin"),
    .remove_path_entry = true,
    .uninstall_context_menu = false,
  });

  ASSERT_TRUE(result);
  EXPECT_EQ(path.removed_path, L"C:\\Users\\zi\\bin");
  EXPECT_FALSE(context_menu.uninstall_called);
  EXPECT_TRUE(result.value().path_removed);
  EXPECT_FALSE(result.value().context_menu_removed);
}

TEST(UninstallWorkflowTests, ReturnsContextMenuFailureBeforePathMutation) {
  FakePathService path;
  FakeContextMenuService context_menu;
  context_menu.fail_uninstall = true;
  const UninstallWorkflow workflow{path, context_menu};

  const auto result = workflow.cleanup(UninstallCleanupRequest{
    .config = config_with_bin(L"C:\\Users\\zi\\bin"),
    .remove_path_entry = true,
    .uninstall_context_menu = true,
  });

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::context_menu_registry_error);
  EXPECT_TRUE(path.removed_path.empty());
}

TEST(UninstallWorkflowTests, ReturnsPathFailure) {
  FakePathService path;
  path.fail_remove = true;
  FakeContextMenuService context_menu;
  const UninstallWorkflow workflow{path, context_menu};

  const auto result = workflow.cleanup(UninstallCleanupRequest{
    .config = config_with_bin(L"C:\\Users\\zi\\bin"),
    .remove_path_entry = true,
    .uninstall_context_menu = false,
  });

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::path_registry_error);
}

} // namespace
