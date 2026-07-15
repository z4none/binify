#include <gtest/gtest.h>

#include "core/result.h"

namespace {

using binify::core::ErrorCode;
using binify::core::Result;
using binify::core::make_error;

TEST(ResultTests, StoresValue) {
  const Result<int> result{42};

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 42);
}

TEST(ResultTests, StoresError) {
  const Result<int> result{make_error(ErrorCode::invalid_argument, L"Bad input.")};

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::invalid_argument);
  EXPECT_EQ(result.error().technical_message, L"Bad input.");
}

TEST(ResultTests, VoidResultDefaultsToSuccess) {
  const Result<void> result;

  EXPECT_TRUE(result);
}

TEST(ResultTests, VoidResultStoresError) {
  const Result<void> result{make_error(ErrorCode::not_implemented, L"Missing.")};

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::not_implemented);
}

} // namespace

