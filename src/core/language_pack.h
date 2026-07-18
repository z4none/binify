#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/result.h"

namespace binify::core {

inline constexpr std::wstring_view kSystemLanguageCode = L"system";
inline constexpr std::wstring_view kFallbackLanguageCode = L"en-US";

struct LanguagePack {
  std::wstring language;
  std::wstring display_name;
  std::map<std::string, std::wstring, std::less<>> strings;
};

struct AvailableLanguage {
  std::wstring language;
  std::wstring display_name;
};

[[nodiscard]] Result<LanguagePack> parse_language_pack_json(std::string_view json_text);
[[nodiscard]] std::vector<LanguagePack> load_language_packs(const std::filesystem::path& directory);
[[nodiscard]] const LanguagePack& built_in_english_language_pack();
[[nodiscard]] bool is_language_code_valid(std::wstring_view language);

class Translator {
public:
  Translator(std::vector<LanguagePack> language_packs, std::wstring requested_language, std::wstring system_language);

  [[nodiscard]] std::wstring text(std::string_view key) const;
  [[nodiscard]] std::wstring selected_language() const;
  [[nodiscard]] std::wstring requested_language() const;
  [[nodiscard]] std::vector<AvailableLanguage> available_languages() const;

private:
  [[nodiscard]] const LanguagePack* find_pack(std::wstring_view language) const;
  [[nodiscard]] std::optional<std::wstring> find_string(const LanguagePack& pack, std::string_view key) const;

  std::vector<LanguagePack> language_packs_;
  std::wstring requested_language_;
  std::wstring selected_language_;
};

} // namespace binify::core
