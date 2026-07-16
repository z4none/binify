#pragma once

#include <filesystem>

#include "app/logger.h"

namespace binify::platform::windows {

struct FileLoggerOptions {
  std::filesystem::path log_directory;
  std::uintmax_t max_file_size_bytes = 1024 * 1024;
  std::size_t max_files = 3;
};

class FileLogger final : public app::Logger {
public:
  explicit FileLogger(FileLoggerOptions options);

  [[nodiscard]] core::Result<void> write(app::LogLevel level, std::wstring_view message) const override;

  [[nodiscard]] const std::filesystem::path& log_directory() const noexcept;
  [[nodiscard]] std::filesystem::path active_log_path() const;
  [[nodiscard]] std::filesystem::path rotated_log_path(std::size_t index) const;

private:
  [[nodiscard]] core::Result<void> rotate_if_needed(std::uintmax_t incoming_bytes) const;

  FileLoggerOptions options_;
};

[[nodiscard]] core::Result<std::filesystem::path> default_log_directory();

} // namespace binify::platform::windows
