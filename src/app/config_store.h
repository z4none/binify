#pragma once

#include <optional>

#include "core/config.h"

namespace binify::app {

class ConfigStore {
public:
  virtual ~ConfigStore() = default;

  [[nodiscard]] virtual core::Result<std::optional<core::Config>> load() const = 0;
  [[nodiscard]] virtual core::Result<void> save(const core::Config& config) const = 0;
};

} // namespace binify::app

