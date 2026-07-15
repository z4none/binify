#include <gtest/gtest.h>

#include "platform/windows/platform.h"

TEST(IntegrationSmokeTests, PlatformInitializationIsCallable) {
  binify::platform::windows::initialize_platform();
  SUCCEED();
}

