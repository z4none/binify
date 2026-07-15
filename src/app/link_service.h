#pragma once

#include <filesystem>
#include <vector>

#include "core/creation_policy.h"
#include "core/result.h"

namespace binify::app {

struct LinkCreationRequest {
  std::filesystem::path source_path;
  std::filesystem::path entry_path;
  core::LinkMode requested_mode = core::LinkMode::auto_select;
};

struct LinkCreationAttempt {
  core::LinkMode mode = core::LinkMode::auto_select;
  core::Error error;
};

struct LinkCreationResult {
  std::filesystem::path entry_path;
  core::LinkMode actual_mode = core::LinkMode::auto_select;
  std::vector<LinkCreationAttempt> failed_attempts;
};

class LinkService {
public:
  virtual ~LinkService() = default;

  [[nodiscard]] virtual core::Result<void> create_symbolic_link(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const = 0;

  [[nodiscard]] virtual core::Result<void> create_hard_link(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const = 0;

  [[nodiscard]] virtual core::Result<void> create_cmd_wrapper(
    const std::filesystem::path& source_path,
    const std::filesystem::path& entry_path) const = 0;
};

[[nodiscard]] core::Result<LinkCreationResult> create_link_entry(
  const LinkService& link_service,
  const LinkCreationRequest& request);

} // namespace binify::app

