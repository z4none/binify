#pragma once

#include <filesystem>
#include <vector>

#include "app/entry_transaction.h"
#include "core/command_conflict.h"
#include "core/command_name.h"

namespace binify::app {

enum class ConflictDecision {
  cancel,
  overwrite
};

struct AddCommandRequest {
  std::filesystem::path source_path;
  std::filesystem::path bin_directory;
  std::wstring command_name_input;
  core::LinkMode requested_mode = core::LinkMode::auto_select;
  ConflictDecision conflict_decision = ConflictDecision::cancel;
};

struct AddCommandResult {
  core::CommandName command_name;
  std::filesystem::path entry_path;
  core::LinkMode actual_mode = core::LinkMode::auto_select;
  std::vector<std::filesystem::path> conflict_paths;
};

class DirectoryListing {
public:
  virtual ~DirectoryListing() = default;

  [[nodiscard]] virtual core::Result<std::vector<core::DirectoryEntry>> list_entries(
    const std::filesystem::path& directory) const = 0;
};

class AddCommandWorkflow {
public:
  AddCommandWorkflow(
    const LinkService& link_service,
    const DirectoryListing& directory_listing,
    const EntryTransactionFileSystem* transaction_file_system = nullptr);
  AddCommandWorkflow(const AddCommandWorkflow&) = delete;
  AddCommandWorkflow& operator=(const AddCommandWorkflow&) = delete;
  AddCommandWorkflow(AddCommandWorkflow&&) = delete;
  AddCommandWorkflow& operator=(AddCommandWorkflow&&) = delete;

  [[nodiscard]] core::Result<AddCommandResult> add(const AddCommandRequest& request) const;

private:
  const LinkService& link_service_;
  const DirectoryListing& directory_listing_;
  StdEntryTransactionFileSystem default_transaction_file_system_;
  const EntryTransactionFileSystem* transaction_file_system_;
};

} // namespace binify::app
