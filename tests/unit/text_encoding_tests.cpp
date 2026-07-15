#include <gtest/gtest.h>

#include "core/text_encoding.h"

namespace {

using namespace binify::core;

TEST(TextEncodingTests, ConvertsUtf8ToWideAndBack) {
  const auto input = reinterpret_cast<const char*>(u8"binify 路径");
  const auto wide = utf8_to_wide(input);

  ASSERT_TRUE(wide);
  EXPECT_EQ(wide.value(), L"binify \u8def\u5f84");

  const auto utf8 = wide_to_utf8(wide.value());

  ASSERT_TRUE(utf8);
  EXPECT_EQ(utf8.value(), input);
}

TEST(TextEncodingTests, RejectsInvalidUtf8) {
  const std::string invalid{"\xC3\x28", 2};

  const auto result = utf8_to_wide(invalid);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::encoding_error);
}

} // namespace
