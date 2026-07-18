#include "core/language_pack.h"

#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

#include "core/error.h"
#include "core/text_encoding.h"

namespace binify::core {
namespace {

LanguagePack make_built_in_english() {
  LanguagePack pack;
  pack.language = L"en-US";
  pack.display_name = L"English";
  pack.strings = {
    {"app.title", L"binify"},
    {"common.cancel", L"Cancel"},
    {"common.save", L"Save"},
    {"common.create", L"Create"},
    {"common.browse", L"Browse..."},
    {"common.open_bin", L"Open Bin"},
    {"common.success", L"Success"},
    {"settings.title", L"binify settings"},
    {"settings.heading", L"⚙  binify settings"},
    {"settings.bin_directory", L"Bin directory"},
    {"settings.select_bin_directory", L"Select Bin directory"},
    {"settings.path_toggle", L"Add Bin directory to current-user PATH"},
    {"settings.context_menu_toggle", L"Enable Explorer context menu for .exe files"},
    {"settings.language", L"Language"},
    {"settings.language_system", L"System Default"},
    {"settings.help", L"Settings are saved for the current user. No administrator permission is required."},
    {"settings.saved", L"Settings saved successfully. Restart binify to apply language changes to all open windows."},
    {"settings.configure_bin_first", L"Configure a Bin directory first."},
    {"add.title", L"Add command with binify"},
    {"add.heading", L"➕  Add command"},
    {"add.source_executable", L"Source executable"},
    {"add.command_name", L"Command name"},
    {"add.link_mode", L"Link mode"},
    {"add.link_mode_help", L"Auto tries Symbolic Link, then Hard Link, then CMD Wrapper. Explicit modes do not fall back."},
    {"add.bin_not_configured", L"Bin directory is not configured. Open binify normally to configure settings first."},
    {"add.configure_first_continue", L"Bin directory is not configured. Configure settings first; after saving, binify will continue adding the selected executable."},
    {"add.configure_bin_before_create", L"Configure Bin directory before creating commands."},
    {"add.source_missing", L"Source executable does not exist."},
    {"add.success_message", L"Command entry created successfully."},
    {"add.success_name", L"Name"},
    {"add.success_mode", L"Mode"},
    {"conflict.overwrite_prompt", L"The command name already exists. Overwrite the following entry?"},
    {"uninstall.title", L"binify uninstall cleanup"},
    {"uninstall.heading", L"🗑  Uninstall cleanup"},
    {"uninstall.summary", L"Choose which current-user integrations binify should remove before uninstalling. Source executables are never deleted."},
    {"uninstall.remove_path", L"Remove Bin directory from current-user PATH"},
    {"uninstall.remove_context_menu", L"Remove Explorer context menu registration"},
    {"uninstall.cleanup", L"Clean up"},
    {"uninstall.completed", L"Cleanup completed."},
    {"uninstall.path_removed", L"PATH entry removed."},
    {"uninstall.path_unchanged", L"PATH entry unchanged."},
    {"uninstall.context_menu_removed", L"Context menu removed."},
    {"uninstall.context_menu_unchanged", L"Context menu unchanged."},
    {"context_menu.add", L"Add to Binify..."},
    {"error.title", L"binify error"},
    {"error.unexpected", L"binify failed with an unexpected error."},
    {"error.unknown", L"binify failed with an unknown error."},
  };
  return pack;
}

Result<std::wstring> required_wide_string(const nlohmann::json& json, const char* key) {
  const auto value = json.at(key).get<std::string>();
  return utf8_to_wide(value);
}

} // namespace

Result<LanguagePack> parse_language_pack_json(std::string_view json_text) {
  try {
    const auto json = nlohmann::json::parse(json_text);
    if (!json.is_object()) {
      return make_error(ErrorCode::config_invalid, L"Language pack JSON root must be an object.");
    }

    const auto& meta = json.at("meta");
    const auto& strings = json.at("strings");
    if (!meta.is_object() || !strings.is_object()) {
      return make_error(ErrorCode::config_invalid, L"Language pack must contain object fields: meta and strings.");
    }

    auto language = required_wide_string(meta, "language");
    if (!language) {
      return language.error();
    }
    auto display_name = required_wide_string(meta, "displayName");
    if (!display_name) {
      return display_name.error();
    }

    LanguagePack pack{
      .language = language.value(),
      .display_name = display_name.value(),
    };

    if (!is_language_code_valid(pack.language)) {
      return make_error(ErrorCode::config_invalid, L"Language pack has an invalid language code.");
    }

    for (const auto& [key, value] : strings.items()) {
      if (!value.is_string()) {
        return make_error(ErrorCode::config_invalid, L"Language pack string values must be strings.");
      }
      auto wide_value = utf8_to_wide(value.get<std::string>());
      if (!wide_value) {
        return wide_value.error();
      }
      pack.strings.emplace(key, wide_value.value());
    }

    return pack;
  } catch (const nlohmann::json::exception&) {
    return make_error(ErrorCode::config_invalid, L"Language pack JSON is missing required fields or has invalid field types.");
  }
}

std::vector<LanguagePack> load_language_packs(const std::filesystem::path& directory) {
  std::vector<LanguagePack> packs;
  std::error_code error_code;
  if (!std::filesystem::exists(directory, error_code) || !std::filesystem::is_directory(directory, error_code)) {
    return packs;
  }

  for (const auto& entry : std::filesystem::directory_iterator(directory, error_code)) {
    if (error_code) {
      break;
    }
    if (!entry.is_regular_file(error_code) || entry.path().extension() != L".json") {
      continue;
    }

    std::ifstream stream(entry.path(), std::ios::binary);
    if (!stream) {
      continue;
    }
    const std::string json_text{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    auto parsed = parse_language_pack_json(json_text);
    if (parsed) {
      packs.push_back(std::move(parsed.value()));
    }
  }

  return packs;
}

const LanguagePack& built_in_english_language_pack() {
  static const LanguagePack pack = make_built_in_english();
  return pack;
}

bool is_language_code_valid(std::wstring_view language) {
  if (language == kSystemLanguageCode) {
    return true;
  }
  if (language.size() < 2 || language.size() > 16) {
    return false;
  }
  for (const auto ch : language) {
    const bool ok =
      (ch >= L'a' && ch <= L'z') ||
      (ch >= L'A' && ch <= L'Z') ||
      (ch >= L'0' && ch <= L'9') ||
      ch == L'-';
    if (!ok) {
      return false;
    }
  }
  return true;
}

Translator::Translator(std::vector<LanguagePack> language_packs, std::wstring requested_language, std::wstring system_language)
  : language_packs_(std::move(language_packs)),
    requested_language_(std::move(requested_language)) {
  if (requested_language_.empty() || !is_language_code_valid(requested_language_)) {
    requested_language_ = std::wstring{kSystemLanguageCode};
  }

  const auto desired_language = requested_language_ == kSystemLanguageCode ? system_language : requested_language_;
  selected_language_ = find_pack(desired_language) != nullptr ? desired_language : std::wstring{kFallbackLanguageCode};
}

std::wstring Translator::text(std::string_view key) const {
  if (const auto* selected = find_pack(selected_language_)) {
    if (auto value = find_string(*selected, key)) {
      return *value;
    }
  }

  if (const auto* fallback = find_pack(kFallbackLanguageCode)) {
    if (auto value = find_string(*fallback, key)) {
      return *value;
    }
  }

  const auto& built_in = built_in_english_language_pack();
  if (auto value = find_string(built_in, key)) {
    return *value;
  }

  return L"!" + std::wstring{key.begin(), key.end()} + L"!";
}

std::wstring Translator::selected_language() const {
  return selected_language_;
}

std::wstring Translator::requested_language() const {
  return requested_language_;
}

std::vector<AvailableLanguage> Translator::available_languages() const {
  std::vector<AvailableLanguage> languages;
  languages.push_back(AvailableLanguage{
    .language = std::wstring{kSystemLanguageCode},
    .display_name = text("settings.language_system"),
  });

  bool has_english = false;
  for (const auto& pack : language_packs_) {
    languages.push_back(AvailableLanguage{
      .language = pack.language,
      .display_name = pack.display_name,
    });
    if (pack.language == kFallbackLanguageCode) {
      has_english = true;
    }
  }

  if (!has_english) {
    const auto& english = built_in_english_language_pack();
    languages.push_back(AvailableLanguage{
      .language = english.language,
      .display_name = english.display_name,
    });
  }
  return languages;
}

const LanguagePack* Translator::find_pack(std::wstring_view language) const {
  for (const auto& pack : language_packs_) {
    if (pack.language == language) {
      return &pack;
    }
  }
  if (language == kFallbackLanguageCode) {
    return &built_in_english_language_pack();
  }
  return nullptr;
}

std::optional<std::wstring> Translator::find_string(const LanguagePack& pack, std::string_view key) const {
  const auto found = pack.strings.find(key);
  if (found == pack.strings.end()) {
    return std::nullopt;
  }
  return found->second;
}

} // namespace binify::core
