#include <gtest/gtest.h>

#include "core/config.h"

namespace {

using namespace binify::core;

Config sample_config() {
  Config config;
  config.bin_directory = L"C:\\Users\\zi\\bin 路径";
  config.context_menu_enabled = true;
  config.path_enabled = true;
  return config;
}

TEST(ConfigSerializationTests, SerializesConfigToExpectedJsonFields) {
  const auto json = serialize_config_to_json(sample_config());

  ASSERT_TRUE(json);
  EXPECT_NE(json.value().find("\"configVersion\": 1"), std::string::npos);
  EXPECT_NE(json.value().find("\"binDirectory\""), std::string::npos);
  EXPECT_NE(json.value().find("\"contextMenuEnabled\": true"), std::string::npos);
  EXPECT_NE(json.value().find("\"pathEnabled\": true"), std::string::npos);
}

TEST(ConfigSerializationTests, RoundTripsUnicodePath) {
  const auto json = serialize_config_to_json(sample_config());
  ASSERT_TRUE(json);

  const auto config = deserialize_config_from_json(json.value());

  ASSERT_TRUE(config);
  EXPECT_EQ(config.value().bin_directory.wstring(), L"C:\\Users\\zi\\bin \u8def\u5f84");
  EXPECT_TRUE(config.value().context_menu_enabled);
  EXPECT_TRUE(config.value().path_enabled);
}

TEST(ConfigSerializationTests, RejectsMalformedJson) {
  const auto config = deserialize_config_from_json("{");

  ASSERT_FALSE(config);
  EXPECT_EQ(config.error().code, ErrorCode::config_invalid);
}

TEST(ConfigSerializationTests, RejectsMissingRequiredField) {
  const auto config = deserialize_config_from_json(R"({"configVersion":1})");

  ASSERT_FALSE(config);
  EXPECT_EQ(config.error().code, ErrorCode::config_invalid);
}

TEST(ConfigSerializationTests, RejectsRelativeBinDirectory) {
  const auto config = deserialize_config_from_json(
    R"({"configVersion":1,"binDirectory":"relative/bin","contextMenuEnabled":false,"pathEnabled":false})");

  ASSERT_FALSE(config);
  EXPECT_EQ(config.error().code, ErrorCode::path_not_absolute);
}

} // namespace

