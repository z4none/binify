#pragma once

#include "app/path_service.h"

namespace binify::platform::windows {

class RegistryPathService final : public app::PathService {
public:
  [[nodiscard]] core::Result<void> add_user_path(const std::filesystem::path& path) const override;
  [[nodiscard]] core::Result<void> remove_user_path(const std::filesystem::path& path) const override;
  [[nodiscard]] core::Result<void> migrate_user_path(
    const std::filesystem::path& old_path,
    const std::filesystem::path& new_path) const override;
};

} // namespace binify::platform::windows

