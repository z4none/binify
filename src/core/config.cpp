#include "core/config.h"

#include "core/error.h"

namespace binify::core {

bool is_configured(const Config& config) {
  return !config.bin_directory.empty();
}

Result<void> validate_config_model(const Config& config) {
  if (config.config_version <= 0) {
    return make_error(ErrorCode::config_invalid, L"Config version must be positive.");
  }

  if (config.config_version > kCurrentConfigVersion) {
    return make_error(ErrorCode::config_invalid, L"Config version is newer than this application supports.");
  }

  if (is_configured(config) && !config.bin_directory.is_absolute()) {
    return make_error(
      ErrorCode::path_not_absolute,
      L"Configured Bin directory must be an absolute path.",
      std::nullopt,
      config.bin_directory.wstring());
  }

  return {};
}

} // namespace binify::core
