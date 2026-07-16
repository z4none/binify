#include "ui/uninstall_dialog.h"

#include "ui/ui_text.h"

namespace binify::ui {
namespace {

constexpr int kIdPath = 3001;
constexpr int kIdContextMenu = 3002;
constexpr int kIdCleanup = 3003;
constexpr int kIdCancel = 3004;

} // namespace

UninstallWindow::UninstallWindow(RuntimeContext& runtime) : runtime_(runtime) {
  setup.wndClassEx.lpszClassName = L"BINIFY_UNINSTALL_WINDOW";
  setup.title = text::kUninstallTitle;
  setup.size = {560, 260};
  setup.style |= WS_MINIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    create_controls();
    load_config();
    return 0;
  });

  on_message(WM_COMMAND, [this](wl::wm::command command) -> LRESULT {
    switch (command.control_id()) {
    case kIdCleanup:
      run_cleanup();
      return 0;
    case kIdCancel:
      DestroyWindow(hwnd());
      return 0;
    default:
      return 0;
    }
  });
}

void UninstallWindow::create_controls() {
  summary_label_.create(
    this,
    -1,
    L"Choose which current-user integrations binify should remove before uninstalling. Source executables are never deleted.",
    {20, 24},
    {500, 50});
  path_checkbox_.create(this, kIdPath, L"Remove Bin directory from current-user PATH", {40, 90}, {420, 24});
  context_menu_checkbox_.create(this, kIdContextMenu, L"Remove Explorer context menu registration", {40, 124}, {420, 24});
  cleanup_button_.create(this, kIdCleanup, L"Clean up", {330, 174}, {90, 28});
  cancel_button_.create(this, kIdCancel, text::kCancel, {440, 174}, {90, 28});
}

void UninstallWindow::load_config() {
  const auto loaded = runtime_.config_store.load();
  if (!loaded) {
    show_error(hwnd(), loaded.error());
    return;
  }

  if (loaded.value()) {
    config_ = *loaded.value();
  }

  path_checkbox_.set_check(config_.path_enabled);
  context_menu_checkbox_.set_check(true);
}

void UninstallWindow::run_cleanup() {
  const auto cleaned = runtime_.uninstall_workflow.cleanup(app::UninstallCleanupRequest{
    .config = config_,
    .remove_path_entry = path_checkbox_.is_checked(),
    .uninstall_context_menu = context_menu_checkbox_.is_checked(),
  });
  if (!cleaned) {
    show_error(hwnd(), cleaned.error());
    return;
  }

  std::wstring message = L"Cleanup completed.";
  message += cleaned.value().path_removed ? L"\r\nPATH entry removed." : L"\r\nPATH entry unchanged.";
  message += cleaned.value().context_menu_removed ? L"\r\nContext menu removed." : L"\r\nContext menu unchanged.";
  show_info(hwnd(), message);
}

} // namespace binify::ui
