#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

#include "platform/windows/shell_service.h"

namespace {

using namespace binify::core;
using binify::platform::windows::WindowsShellService;

std::filesystem::path unique_test_root() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() / (L"binify-shell-service-" + std::to_wstring(ticks));
}

TEST(ShellServiceTests, RejectsMissingDirectoryWithoutLaunchingExplorer) {
  const auto root = unique_test_root();
  const WindowsShellService service;

  const auto result = service.open_directory(root / L"missing");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::path_invalid);
}

TEST(ShellServiceTests, RejectsFilePathAsDirectory) {
  const auto root = unique_test_root();
  std::filesystem::create_directories(root);
  const auto file = root / L"file.txt";
  {
    std::ofstream stream{file};
    stream << "test";
  }
  const WindowsShellService service;

  const auto result = service.open_directory(file);

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::path_invalid);

  std::filesystem::remove_all(root);
}

} // namespace
