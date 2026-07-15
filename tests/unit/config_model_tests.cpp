#include <gtest/gtest.h>

#include "core/config.h"

namespace {

using namespace binify::core;

TEST(ConfigModelTests, EmptyBinDirectoryIsUnconfiguredButValid) {
  const Config config;

  EXPECT_FALSE(is_configured(config));
  EXPECT_TRUE(validate_config_model(config));
}

TEST(ConfigModelTests, RejectsUnsupportedFutureVersion) {
  Config config;
  config.config_version = kCurrentConfigVersion + 1;

  const auto result = validate_config_model(config);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::config_invalid);
}

TEST(ConfigModelTests, RejectsRelativeConfiguredBinDirectory) {
  Config config;
  config.bin_directory = L"relative\\bin";

  const auto result = validate_config_model(config);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::path_not_absolute);
}

TEST(ConfigModelTests, AcceptsAbsoluteConfiguredBinDirectory) {
  Config config;
  config.bin_directory = L"C:\\Users\\zi\\bin";
  config.context_menu_enabled = true;
  config.path_enabled = true;

  EXPECT_TRUE(is_configured(config));
  EXPECT_TRUE(validate_config_model(config));
}

} // namespace

