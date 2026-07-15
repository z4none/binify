#pragma once

namespace binify::core {

enum class LinkMode {
  auto_select = 0,
  symbolic_link = 1,
  hard_link = 2,
  cmd_wrapper = 3
};

} // namespace binify::core

