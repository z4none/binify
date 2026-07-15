#include <gtest/gtest.h>

#include <vector>

#include "core/creation_policy.h"

namespace {

using namespace binify::core;

TEST(CreationPolicyTests, DisplayNamesAreStable) {
  EXPECT_EQ(display_name(LinkMode::auto_select), L"Auto");
  EXPECT_EQ(display_name(LinkMode::symbolic_link), L"Symbolic Link");
  EXPECT_EQ(display_name(LinkMode::hard_link), L"Hard Link");
  EXPECT_EQ(display_name(LinkMode::cmd_wrapper), L"CMD Wrapper");
}

TEST(CreationPolicyTests, EntryExtensionsMatchCreatedArtifact) {
  EXPECT_EQ(entry_extension(LinkMode::symbolic_link), L".exe");
  EXPECT_EQ(entry_extension(LinkMode::hard_link), L".exe");
  EXPECT_EQ(entry_extension(LinkMode::cmd_wrapper), L".bat");
}

TEST(CreationPolicyTests, AutoSequenceMatchesRequirement) {
  const auto sequence = auto_mode_sequence();
  const std::vector<LinkMode> actual{sequence.begin(), sequence.end()};

  EXPECT_EQ(actual, (std::vector<LinkMode>{
                      LinkMode::symbolic_link,
                      LinkMode::hard_link,
                      LinkMode::cmd_wrapper,
                    }));
}

} // namespace

