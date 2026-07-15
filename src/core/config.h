#pragma once

#include <filesystem>

#include "core/result.h"

namespace binify::core {

inline constexpr int kCurrentConfigVersion = 1;

struct Config {
  int config_version = kCurrentConfigVersion;
  std::filesystem::path bin_directory;
  bool context_menu_enabled = false;
  bool path_enabled = false;
};

[[nodiscard]] bool is_configured(const Config& config);
[[nodiscard]] Result<void> validate_config_model(const Config& config);

} // namespace binify::core
