#include <gtest/gtest.h>

#include <fstream>
#include <chrono>

#include "app/bin_entries.h"
#include "core/bat_wrapper.h"

namespace {

using namespace binify::app;

std::filesystem::path unique_temp_directory() {
  const auto name = L"binify-bin-entries-" + std::to_wstring(std::chrono::steady_clock::now().time_since_epoch().count());
  return std::filesystem::temp_directory_path() / name;
}

TEST(BinEntriesTests, ParsesGeneratedCmdWrapperTarget) {
  const auto content = binify::core::generate_bat_wrapper(L"C:\\Tools\\Percent%App.exe");
  ASSERT_TRUE(content);

  const auto target = parse_cmd_wrapper_target(content.value());

  ASSERT_TRUE(target);
  EXPECT_EQ(target->wstring(), L"C:\\Tools\\Percent%App.exe");
}

TEST(BinEntriesTests, ScansCmdWrapperAndRegularExeEntries) {
  const auto directory = unique_temp_directory();
  std::filesystem::create_directories(directory);
  const auto wrapper = directory / L"foo.bat";
  const auto exe = directory / L"bar.exe";

  const auto content = binify::core::generate_bat_wrapper(L"C:\\Tools\\foo.exe");
  ASSERT_TRUE(content);
  {
    std::ofstream stream(wrapper, std::ios::binary);
    stream << content.value();
  }
  {
    std::ofstream stream(exe, std::ios::binary);
    stream << "fake";
  }

  const StdBinEntryFileSystem file_system;
  const auto entries = file_system.scan(directory);

  ASSERT_TRUE(entries);
  ASSERT_EQ(entries.value().size(), 2);
  EXPECT_EQ(entries.value()[0].name, L"bar.exe");
  EXPECT_EQ(entries.value()[0].type, BinEntryType::hard_link);
  EXPECT_EQ(entries.value()[1].name, L"foo.bat");
  EXPECT_EQ(entries.value()[1].type, BinEntryType::cmd_wrapper);
  ASSERT_TRUE(entries.value()[1].target);
  EXPECT_EQ(entries.value()[1].target->wstring(), L"C:\\Tools\\foo.exe");

  std::filesystem::remove_all(directory);
}

TEST(BinEntriesTests, RenameRejectsExistingTarget) {
  const auto directory = unique_temp_directory();
  std::filesystem::create_directories(directory);
  {
    std::ofstream stream(directory / L"old.bat");
    stream << "old";
  }
  {
    std::ofstream stream(directory / L"new.bat");
    stream << "new";
  }

  const StdBinEntryFileSystem file_system;
  const auto renamed = file_system.rename_entry(directory / L"old.bat", L"new");

  ASSERT_FALSE(renamed);
  EXPECT_TRUE(std::filesystem::exists(directory / L"old.bat"));
  EXPECT_TRUE(std::filesystem::exists(directory / L"new.bat"));

  std::filesystem::remove_all(directory);
}

TEST(BinEntriesTests, DeleteRemovesOnlyEntryFile) {
  const auto directory = unique_temp_directory();
  std::filesystem::create_directories(directory);
  const auto source = directory / L"source.exe";
  const auto entry = directory / L"entry.bat";
  {
    std::ofstream stream(source);
    stream << "source";
  }
  {
    std::ofstream stream(entry);
    stream << "entry";
  }

  const StdBinEntryFileSystem file_system;
  const auto deleted = file_system.delete_entry(entry);

  ASSERT_TRUE(deleted);
  EXPECT_FALSE(std::filesystem::exists(entry));
  EXPECT_TRUE(std::filesystem::exists(source));

  std::filesystem::remove_all(directory);
}

} // namespace
