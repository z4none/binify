#pragma once

#include <filesystem>

namespace binify::core {

struct Config {
  int config_version = 1;
  std::filesystem::path bin_directory;
  bool context_menu_enabled = false;
  bool path_enabled = false;
};

} // namespace binify::core

