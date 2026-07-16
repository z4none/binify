#include "app/entry_transaction.h"

#include <chrono>
#include <system_error>

#include "core/error.h"

namespace binify::app {

namespace {

struct BackupMove {
  std::filesystem::path original_path;
  std::filesystem::path backup_path;
};

core::Error filesystem_error(
  core::ErrorCode code,
  std::wstring message,
  const std::filesystem::path& path,
  const std::error_code& error) {
  return core::make_error(
    code,
    std::move(message),
    error.value() == 0 ? std::nullopt : std::optional<unsigned long>{static_cast<unsigned long>(error.value())},
    path.wstring());
}

core::Result<void> restore_backups(
  const EntryTransactionFileSystem& file_system,
  const std::vector<BackupMove>& backups) {
  for (auto iterator = backups.rbegin(); iterator != backups.rend(); ++iterator) {
    auto restored = file_system.rename(iterator->backup_path, iterator->original_path);
    if (!restored) {
      return core::make_error(
        core::ErrorCode::entry_transaction_rollback_failed,
        L"Failed to restore a backed up entry during rollback.",
        restored.error().native_code,
        iterator->original_path.wstring());
    }
  }
  return {};
}

core::Result<void> cleanup_temp_and_restore(
  const EntryTransactionFileSystem& file_system,
  const std::filesystem::path& temporary_entry_path,
  const std::vector<BackupMove>& backups) {
  (void)file_system.remove(temporary_entry_path);
  return restore_backups(file_system, backups);
}

} // namespace

std::filesystem::path StdEntryTransactionFileSystem::make_temporary_entry_path(
  const std::filesystem::path& final_entry_path) const {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return final_entry_path.parent_path() / (final_entry_path.filename().wstring() + L".new." + std::to_wstring(ticks));
}

std::filesystem::path StdEntryTransactionFileSystem::make_backup_path(
  const std::filesystem::path& conflict_path,
  std::size_t index) const {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return conflict_path.parent_path() / (conflict_path.filename().wstring() + L".bak." + std::to_wstring(ticks) + L"." + std::to_wstring(index));
}

core::Result<void> StdEntryTransactionFileSystem::rename(
  const std::filesystem::path& from,
  const std::filesystem::path& to) const {
  std::error_code error;
  std::filesystem::rename(from, to, error);
  if (error) {
    return filesystem_error(core::ErrorCode::entry_transaction_failed, L"Failed to rename an entry transaction path.", to, error);
  }
  return {};
}

core::Result<void> StdEntryTransactionFileSystem::remove(const std::filesystem::path& path) const {
  std::error_code error;
  std::filesystem::remove(path, error);
  if (error) {
    return filesystem_error(core::ErrorCode::entry_transaction_failed, L"Failed to remove an entry transaction path.", path, error);
  }
  return {};
}

core::Result<EntryTransactionResult> execute_entry_transaction(
  const LinkService& link_service,
  const EntryTransactionRequest& request,
  const EntryTransactionFileSystem& file_system) {
  if (request.creation.entry_path.empty()) {
    return core::make_error(core::ErrorCode::invalid_argument, L"Final entry path is required.");
  }

  std::vector<BackupMove> backups;
  backups.reserve(request.conflict_paths.size());
  for (std::size_t index = 0; index < request.conflict_paths.size(); ++index) {
    const auto& conflict_path = request.conflict_paths[index];
    const auto backup_path = file_system.make_backup_path(conflict_path, index);
    auto moved = file_system.rename(conflict_path, backup_path);
    if (!moved) {
      auto rollback = restore_backups(file_system, backups);
      if (!rollback) {
        return rollback.error();
      }
      return moved.error();
    }
    backups.push_back(BackupMove{.original_path = conflict_path, .backup_path = backup_path});
  }

  const auto final_entry_path = request.creation.entry_path;
  const auto temporary_entry_path = file_system.make_temporary_entry_path(final_entry_path);
  LinkCreationRequest temporary_request = request.creation;
  temporary_request.entry_path = temporary_entry_path;

  auto created = create_link_entry(link_service, temporary_request);
  if (!created) {
    auto rollback = cleanup_temp_and_restore(file_system, temporary_entry_path, backups);
    if (!rollback) {
      return rollback.error();
    }
    return created.error();
  }

  auto promoted = file_system.rename(temporary_entry_path, final_entry_path);
  if (!promoted) {
    auto rollback = cleanup_temp_and_restore(file_system, temporary_entry_path, backups);
    if (!rollback) {
      return rollback.error();
    }
    return promoted.error();
  }

  std::vector<std::filesystem::path> removed_backups;
  removed_backups.reserve(backups.size());
  for (const auto& backup : backups) {
    auto removed = file_system.remove(backup.backup_path);
    if (!removed) {
      return core::make_error(
        core::ErrorCode::entry_transaction_failed,
        L"Entry was created, but removing a backup entry failed.",
        removed.error().native_code,
        backup.backup_path.wstring());
    }
    removed_backups.push_back(backup.backup_path);
  }

  auto result = created.value();
  result.entry_path = final_entry_path;
  return EntryTransactionResult{
    .creation = std::move(result),
    .removed_backup_paths = std::move(removed_backups),
  };
}

core::Result<EntryTransactionResult> execute_entry_transaction(
  const LinkService& link_service,
  const EntryTransactionRequest& request) {
  const StdEntryTransactionFileSystem file_system;
  return execute_entry_transaction(link_service, request, file_system);
}

} // namespace binify::app

