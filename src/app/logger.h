#pragma once

#include <string_view>

#include "core/result.h"

namespace binify::app {

enum class LogLevel {
  info,
  warning,
  error
};

class Logger {
public:
  virtual ~Logger() = default;

  [[nodiscard]] virtual core::Result<void> write(LogLevel level, std::wstring_view message) const = 0;
};

class NullLogger final : public Logger {
public:
  [[nodiscard]] core::Result<void> write(LogLevel, std::wstring_view) const override {
    return {};
  }
};

} // namespace binify::app
