#include <gtest/gtest.h>

#include "core/error.h"

namespace {

using namespace binify::core;

TEST(ErrorTests, BuildsErrorWithContext) {
  const auto error = make_error(
    ErrorCode::platform_error,
    L"Registry write failed.",
    5UL,
    std::wstring{L"C:\\Users\\zi\\bin"});

  EXPECT_EQ(error.code, ErrorCode::platform_error);
  EXPECT_EQ(error.technical_message, L"Registry write failed.");
  ASSERT_TRUE(error.native_code.has_value());
  EXPECT_EQ(*error.native_code, 5UL);
  ASSERT_TRUE(error.path.has_value());
  EXPECT_EQ(*error.path, L"C:\\Users\\zi\\bin");
}

TEST(ErrorTests, ConvertsCodeToStableText) {
  EXPECT_EQ(to_wstring(ErrorCode::config_invalid), L"config_invalid");
  EXPECT_EQ(to_wstring(ErrorCode::encoding_error), L"encoding_error");
}

TEST(ErrorTests, IdentifiesRecoverableConfigErrors) {
  EXPECT_TRUE(is_recoverable_config_error(ErrorCode::config_missing));
  EXPECT_TRUE(is_recoverable_config_error(ErrorCode::config_invalid));
  EXPECT_FALSE(is_recoverable_config_error(ErrorCode::platform_error));
}

} // namespace

