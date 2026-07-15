#pragma once

#include "app/link_service.h"

namespace binify::platform::windows {

class WindowsLinkService final : public app::LinkService {
public:
  [[nodiscard]] core::Result<void> create_symbolic_link(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const override;

  [[nodiscard]] core::Result<void> create_hard_link(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const override;

  [[nodiscard]] core::Result<void> create_cmd_wrapper(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const override;
};

} // namespace binify::platform::windows

