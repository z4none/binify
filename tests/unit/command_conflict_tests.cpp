#include <gtest/gtest.h>

#include "core/command_conflict.h"

namespace {

using namespace binify::core;

std::vector<DirectoryEntry> sample_entries() {
  return {
    DirectoryEntry{.filename = L"rg.exe", .path = L"C:\\Users\\zi\\bin\\rg.exe"},
    DirectoryEntry{.filename = L"RG.cmd", .path = L"C:\\Users\\zi\\bin\\RG.cmd"},
    DirectoryEntry{.filename = L"rg.txt", .path = L"C:\\Users\\zi\\bin\\rg.txt"},
    DirectoryEntry{.filename = L"ripgrep.bat", .path = L"C:\\Users\\zi\\bin\\ripgrep.bat"},
    DirectoryEntry{.filename = L"rg.COM", .path = L"C:\\Users\\zi\\bin\\rg.COM"},
  };
}

TEST(CommandConflictTests, ExposesCommandConflictExtensionsInPathResolutionOrder) {
  const auto extensions = command_conflict_extensions();

  ASSERT_EQ(extensions.size(), 4U);
  EXPECT_EQ(extensions[0], L".exe");
  EXPECT_EQ(extensions[1], L".com");
  EXPECT_EQ(extensions[2], L".bat");
  EXPECT_EQ(extensions[3], L".cmd");
}

TEST(CommandConflictTests, FindsExeComBatCmdConflictsByBaseNameInPathResolutionOrder) {
  const auto conflicts = find_command_conflicts(L"rg", sample_entries());

  ASSERT_EQ(conflicts.size(), 3U);
  EXPECT_EQ(conflicts[0].wstring(), L"C:\\Users\\zi\\bin\\rg.exe");
  EXPECT_EQ(conflicts[1].wstring(), L"C:\\Users\\zi\\bin\\rg.COM");
  EXPECT_EQ(conflicts[2].wstring(), L"C:\\Users\\zi\\bin\\RG.cmd");
}

TEST(CommandConflictTests, IgnoresOtherExtensionsAndOtherBaseNames) {
  const auto conflicts = find_command_conflicts(L"ripgrep", sample_entries());

  ASSERT_EQ(conflicts.size(), 1U);
  EXPECT_EQ(conflicts[0].wstring(), L"C:\\Users\\zi\\bin\\ripgrep.bat");
}

TEST(CommandConflictTests, ReportsPresenceWithoutFilesystemAccess) {
  EXPECT_TRUE(has_command_conflict(L"rg", sample_entries()));
  EXPECT_FALSE(has_command_conflict(L"fd", sample_entries()));
}

} // namespace
