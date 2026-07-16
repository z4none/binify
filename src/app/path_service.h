#pragma once

#include <filesystem>

#include "core/result.h"

namespace binify::app {

class PathService {
public:
  virtual ~PathService() = default;

  [[nodiscard]] virtual core::Result<void> add_user_path(const std::filesystem::path& path) const = 0;
  [[nodiscard]] virtual core::Result<void> remove_user_path(const std::filesystem::path& path) const = 0;
  [[nodiscard]] virtual core::Result<void> migrate_user_path(
    const std::filesystem::path& old_path,
    const std::filesystem::path& new_path) const = 0;
};

} // namespace binify::app

