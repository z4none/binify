#include <gtest/gtest.h>

#include "core/language_pack.h"

namespace {

using namespace binify::core;

TEST(LanguagePackTests, ParsesJsonLanguagePack) {
  const auto pack = parse_language_pack_json(R"({
    "meta": {
      "language": "de-DE",
      "displayName": "Deutsch",
      "version": 1
    },
    "strings": {
      "common.save": "Speichern"
    }
  })");

  ASSERT_TRUE(pack);
  EXPECT_EQ(pack.value().language, L"de-DE");
  EXPECT_EQ(pack.value().display_name, L"Deutsch");
  EXPECT_EQ(pack.value().strings.at("common.save"), L"Speichern");
}

TEST(LanguagePackTests, RejectsMalformedPack) {
  const auto pack = parse_language_pack_json(R"({"meta":{},"strings":{}})");

  ASSERT_FALSE(pack);
  EXPECT_EQ(pack.error().code, ErrorCode::config_invalid);
}

TEST(LanguagePackTests, TranslatorFallsBackToEnglishForMissingKeys) {
  LanguagePack partial{
    .language = L"test-Lang",
    .display_name = L"Test",
    .strings = {
      {"common.save", L"Store"},
    },
  };
  Translator translator{{std::move(partial)}, L"test-Lang", L"en-US"};

  EXPECT_EQ(translator.text("common.save"), L"Store");
  EXPECT_EQ(translator.text("common.cancel"), L"Cancel");
  EXPECT_EQ(translator.text("missing.key"), L"!missing.key!");
}

TEST(LanguagePackTests, TranslatorUsesSystemLanguageWhenRequested) {
  LanguagePack zh{
    .language = L"zh-CN",
    .display_name = L"简体中文",
    .strings = {
      {"common.save", L"保存"},
    },
  };
  Translator translator{{std::move(zh)}, L"system", L"zh-CN"};

  EXPECT_EQ(translator.selected_language(), L"zh-CN");
  EXPECT_EQ(translator.text("common.save"), L"保存");
}

} // namespace
