#pragma once

#include <filesystem>
#include <optional>

#include "app/config_store.h"

namespace binify::platform::windows {

class FileConfigStore final : public app::ConfigStore {
public:
  explicit FileConfigStore(std::filesystem::path config_path);

  [[nodiscard]] core::Result<std::optional<core::Config>> load() const override;
  [[nodiscard]] core::Result<void> save(const core::Config& config) const override;

  [[nodiscard]] const std::filesystem::path& config_path() const noexcept;

private:
  std::filesystem::path config_path_;
};

[[nodiscard]] core::Result<std::filesystem::path> default_config_path();

} // namespace binify::platform::windows

