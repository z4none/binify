#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

#include "platform/windows/config_store.h"

namespace {

using namespace binify::core;
using binify::platform::windows::FileConfigStore;

std::filesystem::path unique_test_root() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() / (L"binify-config-store-" + std::to_wstring(ticks));
}

Config sample_config(const std::filesystem::path& root) {
  Config config;
  config.bin_directory = root / L"bin";
  config.context_menu_enabled = true;
  config.path_enabled = false;
  return config;
}

TEST(ConfigStoreTests, MissingConfigReturnsEmptyOptional) {
  const auto root = unique_test_root();
  const FileConfigStore store{root / L"config.json"};

  const auto loaded = store.load();

  ASSERT_TRUE(loaded);
  EXPECT_FALSE(loaded.value().has_value());
}

TEST(ConfigStoreTests, SavesAndLoadsConfig) {
  const auto root = unique_test_root();
  const FileConfigStore store{root / L"binify" / L"config.json"};
  const auto config = sample_config(root);

  const auto saved = store.save(config);
  ASSERT_TRUE(saved);

  const auto loaded = store.load();

  ASSERT_TRUE(loaded);
  ASSERT_TRUE(loaded.value().has_value());
  EXPECT_EQ(loaded.value()->bin_directory, config.bin_directory);
  EXPECT_TRUE(loaded.value()->context_menu_enabled);
  EXPECT_FALSE(loaded.value()->path_enabled);

  std::filesystem::remove_all(root);
}

TEST(ConfigStoreTests, InvalidConfigReturnsConfigInvalid) {
  const auto root = unique_test_root();
  const auto path = root / L"binify" / L"config.json";
  std::filesystem::create_directories(path.parent_path());
  {
    std::ofstream stream{path, std::ios::binary};
    stream << "{";
  }

  const FileConfigStore store{path};
  const auto loaded = store.load();

  ASSERT_FALSE(loaded);
  EXPECT_EQ(loaded.error().code, ErrorCode::config_invalid);

  std::filesystem::remove_all(root);
}

TEST(ConfigStoreTests, DefaultConfigPathUsesLocalAppData) {
  const auto path = binify::platform::windows::default_config_path();

  ASSERT_TRUE(path);
  EXPECT_EQ(path.value().filename(), L"config.json");
  EXPECT_EQ(path.value().parent_path().filename(), L"binify");
}

} // namespace

