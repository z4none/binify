#pragma once

#include <utility>
#include <variant>

#include "core/error.h"

namespace binify::core {

template <typename T>
class Result {
public:
  Result(T value) : value_(std::move(value)) {}
  Result(Error error) : value_(std::move(error)) {}

  [[nodiscard]] bool has_value() const noexcept {
    return std::holds_alternative<T>(value_);
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return has_value();
  }

  [[nodiscard]] const T& value() const {
    return std::get<T>(value_);
  }

  [[nodiscard]] T& value() {
    return std::get<T>(value_);
  }

  [[nodiscard]] const Error& error() const {
    return std::get<Error>(value_);
  }

private:
  std::variant<T, Error> value_;
};

template <>
class Result<void> {
public:
  Result() = default;
  Result(Error error) : error_(std::move(error)), has_error_(true) {}

  [[nodiscard]] bool has_value() const noexcept {
    return !has_error_;
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return has_value();
  }

  [[nodiscard]] const Error& error() const {
    return error_;
  }

private:
  Error error_{};
  bool has_error_ = false;
};

} // namespace binify::core

