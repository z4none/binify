#include "app/link_service.h"

#include "core/error.h"

namespace binify::app {

namespace {

core::Result<void> create_specific_mode(
  const LinkService& link_service,
  const LinkCreationRequest& request,
  core::LinkMode mode) {
  switch (mode) {
  case core::LinkMode::symbolic_link:
    return link_service.create_symbolic_link(request.source_path, request.entry_path);
  case core::LinkMode::hard_link:
    return link_service.create_hard_link(request.source_path, request.entry_path);
  case core::LinkMode::cmd_wrapper:
    return link_service.create_cmd_wrapper(request.source_path, request.entry_path);
  case core::LinkMode::auto_select:
    return core::make_error(core::ErrorCode::invalid_argument, L"Auto is not a concrete creation mode.");
  }

  return core::make_error(core::ErrorCode::invalid_argument, L"Unknown creation mode.");
}

LinkCreationResult success_result(const LinkCreationRequest& request, core::LinkMode actual_mode, std::vector<LinkCreationAttempt> attempts) {
  return LinkCreationResult{
    .entry_path = request.entry_path,
    .actual_mode = actual_mode,
    .failed_attempts = std::move(attempts),
  };
}

} // namespace

core::Result<LinkCreationResult> create_link_entry(
  const LinkService& link_service,
  const LinkCreationRequest& request) {
  if (request.source_path.empty() || request.entry_path.empty()) {
    return core::make_error(core::ErrorCode::invalid_argument, L"Source and entry paths are required.");
  }

  std::vector<LinkCreationAttempt> attempts;
  if (request.requested_mode == core::LinkMode::auto_select) {
    for (const auto mode : core::auto_mode_sequence()) {
      auto created = create_specific_mode(link_service, request, mode);
      if (created) {
        return success_result(request, mode, std::move(attempts));
      }
      attempts.push_back(LinkCreationAttempt{.mode = mode, .error = created.error()});
    }

    return core::make_error(core::ErrorCode::link_creation_failed, L"All link creation modes failed.");
  }

  if (!core::is_specific_mode(request.requested_mode)) {
    return core::make_error(core::ErrorCode::invalid_argument, L"Requested mode is invalid.");
  }

  auto created = create_specific_mode(link_service, request, request.requested_mode);
  if (!created) {
    return created.error();
  }

  return success_result(request, request.requested_mode, {});
}

} // namespace binify::app

