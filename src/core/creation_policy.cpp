#include "core/creation_policy.h"

#include <array>

namespace binify::core {

std::wstring_view display_name(LinkMode mode) noexcept {
  switch (mode) {
  case LinkMode::auto_select:
    return L"Auto";
  case LinkMode::symbolic_link:
    return L"Symbolic Link";
  case LinkMode::hard_link:
    return L"Hard Link";
  case LinkMode::cmd_wrapper:
    return L"CMD Wrapper";
  }

  return L"Unknown";
}

std::wstring_view entry_extension(LinkMode mode) noexcept {
  switch (mode) {
  case LinkMode::auto_select:
  case LinkMode::symbolic_link:
  case LinkMode::hard_link:
    return L".exe";
  case LinkMode::cmd_wrapper:
    return L".bat";
  }

  return L"";
}

bool is_specific_mode(LinkMode mode) noexcept {
  return mode == LinkMode::symbolic_link || mode == LinkMode::hard_link || mode == LinkMode::cmd_wrapper;
}

std::span<const LinkMode> auto_mode_sequence() noexcept {
  static constexpr std::array<LinkMode, 3> kSequence{
    LinkMode::symbolic_link,
    LinkMode::hard_link,
    LinkMode::cmd_wrapper,
  };
  return kSequence;
}

} // namespace binify::core
