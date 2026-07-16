#include <gtest/gtest.h>

#include "core/path_value.h"

namespace {

using namespace binify::core;

TEST(PathValueTests, SplitsAndTrimsNonEmptyEntries) {
  const auto entries = split_path_value(L" C:\\A ; ;D:\\B\\ ");

  ASSERT_EQ(entries.size(), 2U);
  EXPECT_EQ(entries[0].text, L"C:\\A");
  EXPECT_EQ(entries[1].text, L"D:\\B\\");
}

TEST(PathValueTests, AddsPathOnlyWhenEquivalentEntryDoesNotExist) {
  const auto update = add_path_entry(L"C:\\Tools\\bin", L"c:\\tools\\bin\\");

  EXPECT_FALSE(update.changed);
  EXPECT_EQ(update.value, L"C:\\Tools\\bin");
}

TEST(PathValueTests, AppendsNewPathWhenMissing) {
  const auto update = add_path_entry(L"C:\\A", L"D:\\Bin\\");

  EXPECT_TRUE(update.changed);
  EXPECT_EQ(update.value, L"C:\\A;D:\\Bin");
}

TEST(PathValueTests, RemovesOnlyEquivalentPath) {
  const auto update = remove_path_entry(L"C:\\A;C:\\Tools\\bin;C:\\Tools\\binary", L"c:\\tools\\bin\\");

  EXPECT_TRUE(update.changed);
  EXPECT_EQ(update.value, L"C:\\A;C:\\Tools\\binary");
}

TEST(PathValueTests, MigratesOldPathToNewPathInSingleUpdate) {
  const auto update = migrate_path_entry(L"C:\\A;C:\\Old", L"C:\\Old\\", L"D:\\New\\");

  EXPECT_TRUE(update.changed);
  EXPECT_EQ(update.value, L"C:\\A;D:\\New");
}

TEST(PathValueTests, HandlesQuotedEntriesForComparison) {
  EXPECT_TRUE(path_entry_matches(L"\"C:\\Tools\\bin\\\"", L"c:\\tools\\bin"));
}

} // namespace

