#pragma once

#include <span>
#include <string_view>

namespace binify::core {

enum class LinkMode {
  auto_select = 0,
  symbolic_link = 1,
  hard_link = 2,
  cmd_wrapper = 3
};

[[nodiscard]] std::wstring_view display_name(LinkMode mode) noexcept;
[[nodiscard]] std::wstring_view entry_extension(LinkMode mode) noexcept;
[[nodiscard]] bool is_specific_mode(LinkMode mode) noexcept;
[[nodiscard]] std::span<const LinkMode> auto_mode_sequence() noexcept;

} // namespace binify::core
