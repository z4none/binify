#include <gtest/gtest.h>

#include "platform/windows/environment_broadcast.h"

namespace {

TEST(PathServiceTests, EnvironmentBroadcastIsCallable) {
  const auto result = binify::platform::windows::broadcast_environment_change();

  EXPECT_TRUE(result);
}

} // namespace

