#include "core/error.h"

#include <utility>

namespace binify::core {

Error make_error(ErrorCode code, std::wstring message) {
  Error error;
  error.code = code;
  error.technical_message = std::move(message);
  return error;
}

} // namespace binify::core

