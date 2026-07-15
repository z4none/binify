#pragma once

#include "core/result.h"

namespace binify::app {

class Application {
public:
  [[nodiscard]] core::Result<int> run();
};

} // namespace binify::app

