#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

#include "platform/windows/link_service.h"

namespace {

using namespace binify::core;
using binify::platform::windows::WindowsLinkService;

std::filesystem::path unique_test_root() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() / (L"binify-link-service-" + std::to_wstring(ticks));
}

std::filesystem::path create_source_file(const std::filesystem::path& root) {
  const auto source = root / L"source.exe";
  std::filesystem::create_directories(source.parent_path());
  std::ofstream stream{source, std::ios::binary};
  stream << "test";
  return source;
}

TEST(LinkServiceTests, CreatesCmdWrapperFile) {
  const auto root = unique_test_root();
  const auto source = create_source_file(root);
  const auto entry = root / L"bin" / L"tool.bat";
  const WindowsLinkService service;

  const auto result = service.create_cmd_wrapper(source, entry);

  ASSERT_TRUE(result);
  ASSERT_TRUE(std::filesystem::exists(entry));

  {
    std::ifstream stream{entry, std::ios::binary};
    const std::string content{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    EXPECT_NE(content.find("@echo off\r\n"), std::string::npos);
    EXPECT_NE(content.find("exit /b %ERRORLEVEL%\r\n"), std::string::npos);
  }

  std::filesystem::remove_all(root);
}

TEST(LinkServiceTests, CreatesHardLinkOnSameVolume) {
  const auto root = unique_test_root();
  const auto source = create_source_file(root);
  const auto entry = root / L"bin" / L"tool.exe";
  const WindowsLinkService service;

  const auto result = service.create_hard_link(source, entry);

  ASSERT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(entry));
  EXPECT_EQ(std::filesystem::file_size(entry), std::filesystem::file_size(source));

  std::filesystem::remove_all(root);
}

TEST(LinkServiceTests, RejectsMissingSource) {
  const auto root = unique_test_root();
  const WindowsLinkService service;

  const auto result = service.create_cmd_wrapper(root / L"missing.exe", root / L"bin" / L"missing.bat");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::source_invalid);
}

} // namespace
