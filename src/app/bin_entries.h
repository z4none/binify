#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "core/result.h"

namespace binify::app {

enum class BinEntryType {
  symbolic_link,
  hard_link,
  cmd_wrapper,
  file
};

struct BinEntry {
  std::wstring name;
  BinEntryType type = BinEntryType::file;
  std::filesystem::path path;
  std::optional<std::filesystem::path> target;
  std::filesystem::file_time_type modified_time{};
};

class BinEntryFileSystem {
public:
  virtual ~BinEntryFileSystem() = default;

  [[nodiscard]] virtual core::Result<std::vector<BinEntry>> scan(const std::filesystem::path& bin_directory) const = 0;
  [[nodiscard]] virtual core::Result<void> rename_entry(
    const std::filesystem::path& entry_path,
    const std::wstring& new_name) const = 0;
  [[nodiscard]] virtual core::Result<void> delete_entry(const std::filesystem::path& entry_path) const = 0;
};

class StdBinEntryFileSystem final : public BinEntryFileSystem {
public:
  [[nodiscard]] core::Result<std::vector<BinEntry>> scan(const std::filesystem::path& bin_directory) const override;
  [[nodiscard]] core::Result<void> rename_entry(
    const std::filesystem::path& entry_path,
    const std::wstring& new_name) const override;
  [[nodiscard]] core::Result<void> delete_entry(const std::filesystem::path& entry_path) const override;
};

[[nodiscard]] std::wstring_view display_name(BinEntryType type) noexcept;
[[nodiscard]] std::optional<std::filesystem::path> parse_cmd_wrapper_target(std::string_view content);

} // namespace binify::app
