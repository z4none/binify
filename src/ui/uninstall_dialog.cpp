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
  setup.wndClassEx.hIcon = app_icon(32);
  setup.wndClassEx.hIconSm = app_icon(16);
  setup.title = text::kUninstallTitle;
  setup.size = scale_size_for_system_dpi(640, 380);
  setup.style |= WS_MINIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    theme_.initialize(hwnd());
    create_controls();
    load_config();
    return 0;
  });

  on_message(WM_PAINT, [this](wl::wm::paint) -> LRESULT {
    PAINTSTRUCT paint{};
    HDC dc = BeginPaint(hwnd(), &paint);
    draw(dc);
    EndPaint(hwnd(), &paint);
    return 0;
  });

  on_message(WM_CTLCOLORSTATIC, [](wl::params params) -> LRESULT {
    return reinterpret_cast<LRESULT>(transparent_control_background(reinterpret_cast<HDC>(params.wParam)));
  });

  on_command(kIdCleanup, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Uninstall cleanup button clicked."));
    run_cleanup();
    return 0;
  });

  on_command(kIdCancel, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Uninstall cancel clicked."));
    DestroyWindow(hwnd());
    return 0;
  });
}

void UninstallWindow::create_controls() {
  const auto s = [this](int value) { return theme_.scale(value); };

  title_label_.create(this, -1, L"🗑  Uninstall cleanup", {s(24), s(18)}, {s(420), s(34)});
  apply_font(title_label_.hwnd(), theme_.title_font());
  make_transparent_control(title_label_.hwnd());
  summary_label_.create(
    this,
    -1,
    L"Choose which current-user integrations binify should remove before uninstalling. Source executables are never deleted.",
    {s(44), s(86)},
    {s(540), s(54)});
  apply_font(summary_label_.hwnd(), theme_.body_font());
  make_transparent_control(summary_label_.hwnd());
  path_checkbox_.create(this, kIdPath, L"Remove Bin directory from current-user PATH", {s(44), s(184)}, {s(500), s(26)});
  apply_font(path_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(path_checkbox_.hwnd());
  context_menu_checkbox_.create(this, kIdContextMenu, L"Remove Explorer context menu registration", {s(44), s(222)}, {s(500), s(26)});
  apply_font(context_menu_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(context_menu_checkbox_.hwnd());
  cleanup_button_.create(this, kIdCleanup, L"🗑  Clean up", {s(410), s(302)}, {s(110), s(36)});
  apply_font(cleanup_button_.hwnd(), theme_.body_font());
  make_modern_button(cleanup_button_.hwnd(), ButtonRole::danger);
  cancel_button_.create(this, kIdCancel, text::kCancel, {s(534), s(302)}, {s(82), s(36)});
  apply_font(cancel_button_.hwnd(), theme_.body_font());
  make_modern_button(cancel_button_.hwnd(), ButtonRole::secondary);
}

void UninstallWindow::draw(HDC dc) const {
  const auto s = [this](int value) { return theme_.scale(value); };
  draw_window_background(hwnd(), dc, RGB(0xF6, 0xF8, 0xFB));
  draw_panel(dc, {s(24), s(72), s(616), s(152)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));
  draw_panel(dc, {s(24), s(172), s(616), s(266)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));

  RECT action_bar{s(0), s(284), s(640), s(380)};
  HBRUSH brush = CreateSolidBrush(RGB(0xF0, 0xF3, 0xF8));
  FillRect(dc, &action_bar, brush);
  DeleteObject(brush);
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

  static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Uninstall cleanup completed."));
  std::wstring message = L"Cleanup completed.";
  message += cleaned.value().path_removed ? L"\r\nPATH entry removed." : L"\r\nPATH entry unchanged.";
  message += cleaned.value().context_menu_removed ? L"\r\nContext menu removed." : L"\r\nContext menu unchanged.";
  show_info(hwnd(), message);
}

} // namespace binify::ui
