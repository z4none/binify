#include <gtest/gtest.h>

#include "core/context_menu_command.h"

namespace {

using namespace binify::core;

TEST(ContextMenuCommandTests, FormatsQuotedAddCommand) {
  const auto command = format_context_menu_command(L"C:\\Program Files\\binify\\binify.exe");

  ASSERT_TRUE(command);
  EXPECT_EQ(command.value(), L"\"C:\\Program Files\\binify\\binify.exe\" --add \"%1\"");
}

TEST(ContextMenuCommandTests, RejectsRelativeExecutablePath) {
  const auto command = format_context_menu_command(L"binify.exe");

  ASSERT_FALSE(command);
  EXPECT_EQ(command.error().code, ErrorCode::invalid_argument);
}

} // namespace

