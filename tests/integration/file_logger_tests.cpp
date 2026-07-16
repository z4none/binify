#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "platform/windows/file_logger.h"

namespace {

using namespace binify::app;
using binify::platform::windows::FileLogger;
using binify::platform::windows::FileLoggerOptions;

std::filesystem::path unique_test_root() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() / (L"binify-file-logger-" + std::to_wstring(ticks));
}

std::string read_file(const std::filesystem::path& path) {
  std::ifstream stream{path, std::ios::binary};
  return {std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
}

TEST(FileLoggerTests, WritesUtf8LogFile) {
  const auto root = unique_test_root();
  const FileLogger logger{FileLoggerOptions{.log_directory = root / L"logs", .max_file_size_bytes = 1024, .max_files = 3}};

  const auto result = logger.write(LogLevel::info, L"hello 日志");

  ASSERT_TRUE(result);
  ASSERT_TRUE(std::filesystem::exists(logger.active_log_path()));
  const auto content = read_file(logger.active_log_path());
  EXPECT_NE(content.find("[INFO] "), std::string::npos);
  EXPECT_NE(content.find("hello "), std::string::npos);

  std::filesystem::remove_all(root);
}

TEST(FileLoggerTests, RotatesWhenActiveFileWouldExceedLimit) {
  const auto root = unique_test_root();
  const FileLogger logger{FileLoggerOptions{.log_directory = root / L"logs", .max_file_size_bytes = 32, .max_files = 3}};

  ASSERT_TRUE(logger.write(LogLevel::info, L"first message that is long"));
  ASSERT_TRUE(logger.write(LogLevel::warning, L"second message that rotates"));

  EXPECT_TRUE(std::filesystem::exists(logger.active_log_path()));
  EXPECT_TRUE(std::filesystem::exists(logger.rotated_log_path(1)));
  EXPECT_NE(read_file(logger.rotated_log_path(1)).find("first message"), std::string::npos);
  EXPECT_NE(read_file(logger.active_log_path()).find("second message"), std::string::npos);

  std::filesystem::remove_all(root);
}

TEST(FileLoggerTests, DefaultLogDirectoryUsesLocalAppData) {
  const auto directory = binify::platform::windows::default_log_directory();

  ASSERT_TRUE(directory);
  EXPECT_EQ(directory.value().filename(), L"logs");
  EXPECT_EQ(directory.value().parent_path().filename(), L"binify");
}

} // namespace
