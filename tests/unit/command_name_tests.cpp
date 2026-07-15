#include <gtest/gtest.h>

#include "core/command_name.h"

namespace {

using namespace binify::core;

TEST(CommandNameTests, TrimsWhitespaceAndRemovesExeSuffix) {
  const auto result = normalize_command_name(L"  Foo.EXE  ");

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().normalized, L"Foo");
}

TEST(CommandNameTests, RejectsEmptyNameAfterNormalization) {
  const auto result = normalize_command_name(L"  .exe  ");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::command_name_empty);
}

TEST(CommandNameTests, RejectsInvalidWindowsFilenameCharacters) {
  const auto result = normalize_command_name(L"foo/bar");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::command_name_invalid_character);
}

TEST(CommandNameTests, RejectsTrailingPeriodAfterNormalization) {
  const auto result = normalize_command_name(L"foo.");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::command_name_invalid_suffix);
}

TEST(CommandNameTests, RejectsReservedDeviceNamesCaseInsensitive) {
  EXPECT_FALSE(normalize_command_name(L"con"));
  EXPECT_FALSE(normalize_command_name(L"COM1"));
  EXPECT_FALSE(normalize_command_name(L"lpt9.txt"));
  EXPECT_TRUE(normalize_command_name(L"COM10"));
}

TEST(CommandNameTests, BuildsFinalEntryNameForExecutableLinkModes) {
  const auto command_name = normalize_command_name(L"rg.exe");
  ASSERT_TRUE(command_name);

  EXPECT_EQ(final_entry_name(command_name.value(), LinkMode::symbolic_link), L"rg.exe");
  EXPECT_EQ(final_entry_name(command_name.value(), LinkMode::hard_link), L"rg.exe");
}

TEST(CommandNameTests, BuildsFinalEntryNameForCmdWrapper) {
  const auto command_name = normalize_command_name(L"rg");
  ASSERT_TRUE(command_name);

  EXPECT_EQ(final_entry_name(command_name.value(), LinkMode::cmd_wrapper), L"rg.bat");
}

} // namespace

