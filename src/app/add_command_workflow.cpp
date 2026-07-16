#include "app/add_command_workflow.h"

#include "core/error.h"

namespace binify::app {

AddCommandWorkflow::AddCommandWorkflow(
  const LinkService& link_service,
  const DirectoryListing& directory_listing,
  const EntryTransactionFileSystem* transaction_file_system)
  : link_service_(link_service),
    directory_listing_(directory_listing),
    transaction_file_system_(transaction_file_system != nullptr ? transaction_file_system : &default_transaction_file_system_) {}

core::Result<AddCommandResult> AddCommandWorkflow::add(const AddCommandRequest& request) const {
  if (request.source_path.empty() || request.bin_directory.empty()) {
    return core::make_error(core::ErrorCode::invalid_argument, L"Source path and Bin directory are required.");
  }

  auto command_name = core::normalize_command_name(request.command_name_input);
  if (!command_name) {
    return command_name.error();
  }

  const auto entry_path = request.bin_directory / core::final_entry_name(command_name.value(), request.requested_mode);
  auto entries = directory_listing_.list_entries(request.bin_directory);
  if (!entries) {
    return entries.error();
  }

  auto conflicts = core::find_command_conflicts(command_name.value().normalized, entries.value());
  if (!conflicts.empty() && request.conflict_decision != ConflictDecision::overwrite) {
    return core::make_error(core::ErrorCode::operation_cancelled, L"Command entry already exists and overwrite was not confirmed.");
  }

  auto created = execute_entry_transaction(
    link_service_,
    EntryTransactionRequest{
      .creation = LinkCreationRequest{
        .source_path = request.source_path,
        .entry_path = entry_path,
        .requested_mode = request.requested_mode,
      },
      .conflict_paths = conflicts,
    },
    *transaction_file_system_);
  if (!created) {
    return created.error();
  }

  return AddCommandResult{
    .command_name = command_name.value(),
    .entry_path = created.value().creation.entry_path,
    .actual_mode = created.value().creation.actual_mode,
    .conflict_paths = std::move(conflicts),
  };
}

} // namespace binify::app
