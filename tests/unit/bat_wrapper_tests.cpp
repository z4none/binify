#include <gtest/gtest.h>

#include "core/bat_wrapper.h"

namespace {

using namespace binify::core;

TEST(BatWrapperTests, GeneratesSimpleWrapperWithArgumentForwardingAndExitCode) {
  const auto content = generate_bat_wrapper(L"C:\\Tools\\My App\\tool.exe");

  ASSERT_TRUE(content);
  EXPECT_EQ(
    content.value(),
    "@echo off\r\n"
    "\"C:\\Tools\\My App\\tool.exe\" %*\r\n"
    "exit /b %ERRORLEVEL%\r\n");
}

TEST(BatWrapperTests, EscapesPercentSignsInSourcePath) {
  const auto content = generate_bat_wrapper(L"C:\\Tools\\100%\\tool.exe");

  ASSERT_TRUE(content);
  EXPECT_NE(content.value().find("100%%"), std::string::npos);
}

TEST(BatWrapperTests, RejectsRelativeSourcePath) {
  const auto content = generate_bat_wrapper(L"tool.exe");

  ASSERT_FALSE(content);
  EXPECT_EQ(content.error().code, ErrorCode::source_invalid);
}

} // namespace

