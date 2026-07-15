#include <gtest/gtest.h>

#include "core/path_utils.h"

namespace {

using namespace binify::core;

TEST(PathUtilsTests, NormalizesLexicalSegments) {
  const auto path = normalize_path_lexically(L"C:\\Users\\zi\\bin\\..\\bin\\");

  EXPECT_EQ(path.wstring(), L"C:\\Users\\zi\\bin");
}

TEST(PathUtilsTests, KeepsDriveRootSeparator) {
  const auto path = remove_trailing_separators(L"C:\\");

  EXPECT_EQ(path.wstring(), L"C:\\");
}

TEST(PathUtilsTests, ComparesCaseInsensitiveAndSeparatorInsensitive) {
  EXPECT_TRUE(path_equal_case_insensitive(L"C:/Users/ZI/bin/", L"c:\\users\\zi\\bin"));
  EXPECT_FALSE(path_equal_case_insensitive(L"C:\\Users\\zi\\bin", L"C:\\Users\\zi\\other"));
}

} // namespace

