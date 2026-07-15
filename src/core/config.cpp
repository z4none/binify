#include "core/config.h"

#include <nlohmann/json.hpp>

#include "core/error.h"
#include "core/text_encoding.h"

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

Result<std::string> serialize_config_to_json(const Config& config) {
  auto valid = validate_config_model(config);
  if (!valid) {
    return valid.error();
  }

  auto bin_directory = wide_to_utf8(config.bin_directory.wstring());
  if (!bin_directory) {
    return bin_directory.error();
  }

  const nlohmann::json json{
    {"configVersion", config.config_version},
    {"binDirectory", bin_directory.value()},
    {"contextMenuEnabled", config.context_menu_enabled},
    {"pathEnabled", config.path_enabled},
  };
  return json.dump(2);
}

Result<Config> deserialize_config_from_json(const std::string& json_text) {
  try {
    const auto json = nlohmann::json::parse(json_text);
    if (!json.is_object()) {
      return make_error(ErrorCode::config_invalid, L"Config JSON root must be an object.");
    }

    const auto version = json.at("configVersion").get<int>();
    const auto bin_directory_utf8 = json.at("binDirectory").get<std::string>();
    const auto context_menu_enabled = json.at("contextMenuEnabled").get<bool>();
    const auto path_enabled = json.at("pathEnabled").get<bool>();

    auto bin_directory = utf8_to_wide(bin_directory_utf8);
    if (!bin_directory) {
      return bin_directory.error();
    }

    Config config{
      .config_version = version,
      .bin_directory = std::filesystem::path{bin_directory.value()},
      .context_menu_enabled = context_menu_enabled,
      .path_enabled = path_enabled,
    };

    auto valid = validate_config_model(config);
    if (!valid) {
      return valid.error();
    }

    return config;
  } catch (const nlohmann::json::exception&) {
    return make_error(ErrorCode::config_invalid, L"Config JSON is missing required fields or has invalid field types.");
  }
}

} // namespace binify::core
