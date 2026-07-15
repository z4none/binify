#include <gtest/gtest.h>

#include "core/creation_policy.h"

TEST(SmokeTests, LinkModeValuesAreStable) {
  EXPECT_EQ(static_cast<int>(binify::core::LinkMode::auto_select), 0);
  EXPECT_EQ(static_cast<int>(binify::core::LinkMode::symbolic_link), 1);
  EXPECT_EQ(static_cast<int>(binify::core::LinkMode::hard_link), 2);
  EXPECT_EQ(static_cast<int>(binify::core::LinkMode::cmd_wrapper), 3);
}

