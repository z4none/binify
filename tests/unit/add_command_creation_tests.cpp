#include <gtest/gtest.h>

#include <vector>

#include "app/link_service.h"

namespace {

using namespace binify::app;
using namespace binify::core;

class FakeLinkService final : public LinkService {
public:
  Result<void> create_symbolic_link(const std::filesystem::path&, const std::filesystem::path&) const override {
    calls.push_back(LinkMode::symbolic_link);
    if (symbolic_link_fails) {
      return make_error(ErrorCode::link_mode_unavailable, L"Symbolic link failed.");
    }
    return {};
  }

  Result<void> create_hard_link(const std::filesystem::path&, const std::filesystem::path&) const override {
    calls.push_back(LinkMode::hard_link);
    if (hard_link_fails) {
      return make_error(ErrorCode::link_mode_unavailable, L"Hard link failed.");
    }
    return {};
  }

  Result<void> create_cmd_wrapper(const std::filesystem::path&, const std::filesystem::path&) const override {
    calls.push_back(LinkMode::cmd_wrapper);
    if (cmd_wrapper_fails) {
      return make_error(ErrorCode::link_mode_unavailable, L"CMD wrapper failed.");
    }
    return {};
  }

  mutable std::vector<LinkMode> calls;
  bool symbolic_link_fails = true;
  bool hard_link_fails = true;
  bool cmd_wrapper_fails = false;
};

LinkCreationRequest sample_request(LinkMode mode) {
  return LinkCreationRequest{
    .source_path = L"C:\\Tools\\tool.exe",
    .entry_path = L"C:\\Users\\zi\\bin\\tool.exe",
    .requested_mode = mode,
  };
}

TEST(AddCommandCreationTests, AutoFallsBackInRequiredOrder) {
  FakeLinkService service;

  const auto result = create_link_entry(service, sample_request(LinkMode::auto_select));

  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().actual_mode, LinkMode::cmd_wrapper);
  EXPECT_EQ(service.calls, (std::vector<LinkMode>{
                             LinkMode::symbolic_link,
                             LinkMode::hard_link,
                             LinkMode::cmd_wrapper,
                           }));
  ASSERT_EQ(result.value().failed_attempts.size(), 2U);
}

TEST(AddCommandCreationTests, SpecificModeDoesNotFallBack) {
  FakeLinkService service;

  const auto result = create_link_entry(service, sample_request(LinkMode::symbolic_link));

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::link_mode_unavailable);
  EXPECT_EQ(service.calls, (std::vector<LinkMode>{LinkMode::symbolic_link}));
}

TEST(AddCommandCreationTests, AutoReturnsAggregateFailureWhenAllModesFail) {
  FakeLinkService service;
  service.cmd_wrapper_fails = true;

  const auto result = create_link_entry(service, sample_request(LinkMode::auto_select));

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::link_creation_failed);
  EXPECT_EQ(service.calls.size(), 3U);
}

} // namespace

