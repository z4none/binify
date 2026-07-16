#include <gtest/gtest.h>

#include "app/add_command_workflow.h"

namespace {

using namespace binify::app;
using namespace binify::core;

class FakeLinkService final : public LinkService {
public:
  Result<void> create_symbolic_link(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    created_entry = entry_path;
    return {};
  }

  Result<void> create_hard_link(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    created_entry = entry_path;
    return {};
  }

  Result<void> create_cmd_wrapper(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    created_entry = entry_path;
    return {};
  }

  mutable std::filesystem::path created_entry;
};

class FakeDirectoryListing final : public DirectoryListing {
public:
  Result<std::vector<DirectoryEntry>> list_entries(const std::filesystem::path& directory) const override {
    listed_directory = directory;
    return entries;
  }

  mutable std::filesystem::path listed_directory;
  std::vector<DirectoryEntry> entries;
};

class FakeTransactionFileSystem final : public EntryTransactionFileSystem {
public:
  std::filesystem::path make_temporary_entry_path(const std::filesystem::path& final_entry_path) const override {
    return final_entry_path.parent_path() / (final_entry_path.filename().wstring() + L".new");
  }

  std::filesystem::path make_backup_path(const std::filesystem::path& conflict_path, std::size_t index) const override {
    return conflict_path.parent_path() / (conflict_path.filename().wstring() + L".bak." + std::to_wstring(index));
  }

  Result<void> rename(const std::filesystem::path& from, const std::filesystem::path& to) const override {
    renames.emplace_back(from, to);
    return {};
  }

  Result<void> remove(const std::filesystem::path& path) const override {
    removes.push_back(path);
    return {};
  }

  mutable std::vector<std::pair<std::filesystem::path, std::filesystem::path>> renames;
  mutable std::vector<std::filesystem::path> removes;
};

AddCommandRequest request(std::wstring command_name, LinkMode mode = LinkMode::symbolic_link) {
  return AddCommandRequest{
    .source_path = L"C:\\Tools\\rg.exe",
    .bin_directory = L"C:\\Users\\zi\\bin",
    .command_name_input = std::move(command_name),
    .requested_mode = mode,
    .conflict_decision = ConflictDecision::cancel,
  };
}

TEST(AddCommandWorkflowTests, NormalizesNameAndCreatesEntry) {
  FakeLinkService links;
  FakeDirectoryListing listing;
  FakeTransactionFileSystem file_system;
  const AddCommandWorkflow workflow{links, listing, &file_system};

  const auto result = workflow.add(request(L" rg.exe "));

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().command_name.normalized, L"rg");
  EXPECT_EQ(result.value().entry_path, L"C:\\Users\\zi\\bin\\rg.exe");
  EXPECT_EQ(links.created_entry.extension(), L".new");
  ASSERT_EQ(file_system.renames.size(), 1U);
  EXPECT_EQ(file_system.renames[0], (std::pair<std::filesystem::path, std::filesystem::path>{
                                      L"C:\\Users\\zi\\bin\\rg.exe.new",
                                      L"C:\\Users\\zi\\bin\\rg.exe"}));
  EXPECT_EQ(listing.listed_directory, L"C:\\Users\\zi\\bin");
}

TEST(AddCommandWorkflowTests, CancelsWhenConflictExistsWithoutOverwriteDecision) {
  FakeLinkService links;
  FakeDirectoryListing listing;
  listing.entries.push_back(DirectoryEntry{.filename = L"rg.exe", .path = L"C:\\Users\\zi\\bin\\rg.exe"});
  FakeTransactionFileSystem file_system;
  const AddCommandWorkflow workflow{links, listing, &file_system};

  const auto result = workflow.add(request(L"rg"));

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::operation_cancelled);
  EXPECT_TRUE(links.created_entry.empty());
  EXPECT_TRUE(file_system.renames.empty());
}

TEST(AddCommandWorkflowTests, OverwritesWhenConflictDecisionAllowsIt) {
  FakeLinkService links;
  FakeDirectoryListing listing;
  listing.entries.push_back(DirectoryEntry{.filename = L"rg.cmd", .path = L"C:\\Users\\zi\\bin\\rg.cmd"});
  FakeTransactionFileSystem file_system;
  const AddCommandWorkflow workflow{links, listing, &file_system};
  auto add_request = request(L"rg", LinkMode::cmd_wrapper);
  add_request.conflict_decision = ConflictDecision::overwrite;

  const auto result = workflow.add(add_request);

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().entry_path, L"C:\\Users\\zi\\bin\\rg.bat");
  ASSERT_EQ(result.value().conflict_paths.size(), 1U);
  EXPECT_EQ(result.value().conflict_paths[0], L"C:\\Users\\zi\\bin\\rg.cmd");
  ASSERT_EQ(file_system.renames.size(), 2U);
  EXPECT_EQ(file_system.renames[0], (std::pair<std::filesystem::path, std::filesystem::path>{
                                      L"C:\\Users\\zi\\bin\\rg.cmd",
                                      L"C:\\Users\\zi\\bin\\rg.cmd.bak.0"}));
  EXPECT_EQ(file_system.renames[1], (std::pair<std::filesystem::path, std::filesystem::path>{
                                      L"C:\\Users\\zi\\bin\\rg.bat.new",
                                      L"C:\\Users\\zi\\bin\\rg.bat"}));
  ASSERT_EQ(file_system.removes.size(), 1U);
  EXPECT_EQ(file_system.removes[0], L"C:\\Users\\zi\\bin\\rg.cmd.bak.0");
}

} // namespace
