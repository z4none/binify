#include <gtest/gtest.h>

#include "app/entry_transaction.h"

namespace {

using namespace binify::app;
using namespace binify::core;

class FakeLinkService final : public LinkService {
public:
  Result<void> create_symbolic_link(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    calls.push_back(entry_path);
    if (fail_create) {
      return make_error(ErrorCode::link_creation_failed, L"Injected create failure.");
    }
    return {};
  }

  Result<void> create_hard_link(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    calls.push_back(entry_path);
    return {};
  }

  Result<void> create_cmd_wrapper(const std::filesystem::path&, const std::filesystem::path& entry_path) const override {
    calls.push_back(entry_path);
    return {};
  }

  mutable std::vector<std::filesystem::path> calls;
  bool fail_create = false;
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
    if (fail_first_backup && from == first_conflict_path) {
      return make_error(ErrorCode::entry_transaction_failed, L"Injected backup failure.");
    }
    if (fail_promote && from == temporary_path && to == final_path) {
      return make_error(ErrorCode::entry_transaction_failed, L"Injected promote failure.");
    }
    return {};
  }

  Result<void> remove(const std::filesystem::path& path) const override {
    removes.push_back(path);
    return {};
  }

  mutable std::vector<std::pair<std::filesystem::path, std::filesystem::path>> renames;
  mutable std::vector<std::filesystem::path> removes;
  std::filesystem::path final_path = L"C:\\bin\\tool.exe";
  std::filesystem::path temporary_path = L"C:\\bin\\tool.exe.new";
  std::filesystem::path first_conflict_path = L"C:\\bin\\tool.exe";
  bool fail_promote = false;
  bool fail_first_backup = false;
};

EntryTransactionRequest sample_request() {
  return EntryTransactionRequest{
    .creation = LinkCreationRequest{
      .source_path = L"C:\\tools\\tool.exe",
      .entry_path = L"C:\\bin\\tool.exe",
      .requested_mode = LinkMode::symbolic_link,
    },
    .conflict_paths = {
      L"C:\\bin\\tool.exe",
      L"C:\\bin\\tool.cmd",
    },
  };
}

TEST(EntryTransactionTests, BacksUpConflictsCreatesTemporaryEntryAndPromotesFinalEntry) {
  FakeLinkService link_service;
  FakeTransactionFileSystem file_system;

  const auto result = execute_entry_transaction(link_service, sample_request(), file_system);

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().creation.entry_path, L"C:\\bin\\tool.exe");
  ASSERT_EQ(link_service.calls.size(), 1U);
  EXPECT_EQ(link_service.calls[0], L"C:\\bin\\tool.exe.new");
  ASSERT_EQ(file_system.renames.size(), 3U);
  EXPECT_EQ(file_system.renames[0], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.exe", L"C:\\bin\\tool.exe.bak.0"}));
  EXPECT_EQ(file_system.renames[1], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.cmd", L"C:\\bin\\tool.cmd.bak.1"}));
  EXPECT_EQ(file_system.renames[2], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.exe.new", L"C:\\bin\\tool.exe"}));
  EXPECT_EQ(file_system.removes.size(), 2U);
}

TEST(EntryTransactionTests, RestoresBackupsWhenCreationFails) {
  FakeLinkService link_service;
  link_service.fail_create = true;
  FakeTransactionFileSystem file_system;

  const auto result = execute_entry_transaction(link_service, sample_request(), file_system);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::link_creation_failed);
  ASSERT_EQ(file_system.renames.size(), 4U);
  EXPECT_EQ(file_system.renames[2], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.cmd.bak.1", L"C:\\bin\\tool.cmd"}));
  EXPECT_EQ(file_system.renames[3], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.exe.bak.0", L"C:\\bin\\tool.exe"}));
  ASSERT_EQ(file_system.removes.size(), 1U);
  EXPECT_EQ(file_system.removes[0], L"C:\\bin\\tool.exe.new");
}

TEST(EntryTransactionTests, RestoresBackupsWhenPromotionFails) {
  FakeLinkService link_service;
  FakeTransactionFileSystem file_system;
  file_system.fail_promote = true;

  const auto result = execute_entry_transaction(link_service, sample_request(), file_system);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::entry_transaction_failed);
  ASSERT_EQ(file_system.renames.size(), 5U);
  EXPECT_EQ(file_system.renames[3], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.cmd.bak.1", L"C:\\bin\\tool.cmd"}));
  EXPECT_EQ(file_system.renames[4], (std::pair<std::filesystem::path, std::filesystem::path>{L"C:\\bin\\tool.exe.bak.0", L"C:\\bin\\tool.exe"}));
}

TEST(EntryTransactionTests, StopsWhenFirstBackupMoveFails) {
  FakeLinkService link_service;
  FakeTransactionFileSystem file_system;
  file_system.fail_first_backup = true;

  const auto result = execute_entry_transaction(link_service, sample_request(), file_system);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::entry_transaction_failed);
  EXPECT_TRUE(link_service.calls.empty());
  ASSERT_EQ(file_system.renames.size(), 1U);
}

} // namespace
