#pragma once

#include <filesystem>
#include <vector>

#include "app/link_service.h"

namespace binify::app {

struct EntryTransactionRequest {
  LinkCreationRequest creation;
  std::vector<std::filesystem::path> conflict_paths;
};

struct EntryTransactionResult {
  LinkCreationResult creation;
  std::vector<std::filesystem::path> removed_backup_paths;
};

class EntryTransactionFileSystem {
public:
  virtual ~EntryTransactionFileSystem() = default;

  [[nodiscard]] virtual std::filesystem::path make_temporary_entry_path(
    const std::filesystem::path& final_entry_path) const = 0;
  [[nodiscard]] virtual std::filesystem::path make_backup_path(
    const std::filesystem::path& conflict_path,
    std::size_t index) const = 0;
  [[nodiscard]] virtual core::Result<void> rename(
    const std::filesystem::path& from,
    const std::filesystem::path& to) const = 0;
  [[nodiscard]] virtual core::Result<void> remove(const std::filesystem::path& path) const = 0;
};

class StdEntryTransactionFileSystem final : public EntryTransactionFileSystem {
public:
  [[nodiscard]] std::filesystem::path make_temporary_entry_path(
    const std::filesystem::path& final_entry_path) const override;
  [[nodiscard]] std::filesystem::path make_backup_path(
    const std::filesystem::path& conflict_path,
    std::size_t index) const override;
  [[nodiscard]] core::Result<void> rename(
    const std::filesystem::path& from,
    const std::filesystem::path& to) const override;
  [[nodiscard]] core::Result<void> remove(const std::filesystem::path& path) const override;
};

[[nodiscard]] core::Result<EntryTransactionResult> execute_entry_transaction(
  const LinkService& link_service,
  const EntryTransactionRequest& request,
  const EntryTransactionFileSystem& file_system);

[[nodiscard]] core::Result<EntryTransactionResult> execute_entry_transaction(
  const LinkService& link_service,
  const EntryTransactionRequest& request);

} // namespace binify::app

