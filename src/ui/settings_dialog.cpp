#include "ui/settings_dialog.h"

#include <shlobj.h>
#include <objbase.h>

#include "ui/ui_text.h"

namespace binify::ui {
namespace {

constexpr int kIdBinText = 1001;
constexpr int kIdBrowse = 1002;
constexpr int kIdPath = 1003;
constexpr int kIdContextMenu = 1004;
constexpr int kIdOpenBin = 1005;
constexpr int kIdSave = 1006;
constexpr int kIdCancel = 1007;

std::filesystem::path choose_folder(HWND owner) {
  const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  BROWSEINFOW browse_info{};
  browse_info.hwndOwner = owner;
  browse_info.lpszTitle = L"Select Bin directory";
  browse_info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

  PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&browse_info);
  if (item == nullptr) {
    if (SUCCEEDED(initialized)) {
      CoUninitialize();
    }
    return {};
  }

  wchar_t path[MAX_PATH]{};
  const bool ok = SHGetPathFromIDListW(item, path) == TRUE;
  CoTaskMemFree(item);
  if (SUCCEEDED(initialized)) {
    CoUninitialize();
  }
  return ok ? std::filesystem::path{path} : std::filesystem::path{};
}

} // namespace

SettingsWindow::SettingsWindow(RuntimeContext& runtime) : runtime_(runtime) {
  setup.wndClassEx.lpszClassName = L"BINIFY_SETTINGS_WINDOW";
  setup.title = text::kSettingsTitle;
  setup.size = scale_size_for_system_dpi(760, 540);
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

  on_command(kIdBrowse, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings Browse clicked."));
    browse_bin_directory();
    return 0;
  });

  on_command(kIdOpenBin, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings Open Bin clicked."));
    open_bin_directory();
    return 0;
  });

  on_command(kIdSave, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings Save clicked."));
    save_config();
    return 0;
  });

  on_command(kIdCancel, [this](wl::params) -> LRESULT {
    static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings Cancel clicked."));
    DestroyWindow(hwnd());
    return 0;
  });

}

void SettingsWindow::create_controls() {
  const auto s = [this](int value) { return theme_.scale(value); };

  title_label_.create(this, -1, L"⚙  binify settings", {s(24), s(18)}, {s(420), s(34)});
  apply_font(title_label_.hwnd(), theme_.title_font());
  make_transparent_control(title_label_.hwnd());

  bin_label_.create(this, -1, L"Bin directory", {s(44), s(88)}, {s(150), s(22)});
  apply_font(bin_label_.hwnd(), theme_.body_font());
  make_transparent_control(bin_label_.hwnd());
  bin_text_.create(this, kIdBinText, wl::textbox::type::NORMAL, {s(44), s(116)}, s(500), s(25));
  apply_font(bin_text_.hwnd(), theme_.body_font());
  browse_button_.create(this, kIdBrowse, L"📁  Browse", {s(565), s(114)}, {s(130), s(32)});
  apply_font(browse_button_.hwnd(), theme_.body_font());
  make_modern_button(browse_button_.hwnd(), ButtonRole::secondary);

  path_checkbox_.create(this, kIdPath, text::kPathToggle, {s(44), s(194)}, {s(520), s(26)});
  apply_font(path_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(path_checkbox_.hwnd());
  context_menu_checkbox_.create(this, kIdContextMenu, text::kContextMenuToggle, {s(44), s(230)}, {s(560), s(26)});
  apply_font(context_menu_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(context_menu_checkbox_.hwnd());

  help_label_.create(this, -1, L"Settings are saved for the current user. No administrator permission is required.", {s(44), s(326)}, {s(620), s(38)});
  apply_font(help_label_.hwnd(), theme_.small_font());
  make_transparent_control(help_label_.hwnd());
  open_bin_button_.create(this, kIdOpenBin, L"📂  Open Bin", {s(44), s(380)}, {s(150), s(34)});
  apply_font(open_bin_button_.hwnd(), theme_.body_font());
  make_modern_button(open_bin_button_.hwnd(), ButtonRole::secondary);

  save_button_.create(this, kIdSave, L"✓  Save", {s(550), s(462)}, {s(90), s(36)});
  apply_font(save_button_.hwnd(), theme_.body_font());
  make_modern_button(save_button_.hwnd(), ButtonRole::primary);
  cancel_button_.create(this, kIdCancel, text::kCancel, {s(650), s(462)}, {s(82), s(36)});
  apply_font(cancel_button_.hwnd(), theme_.body_font());
  make_modern_button(cancel_button_.hwnd(), ButtonRole::secondary);
}

void SettingsWindow::draw(HDC dc) const {
  const auto s = [this](int value) { return theme_.scale(value); };
  draw_window_background(hwnd(), dc, RGB(0xF6, 0xF8, 0xFB));
  draw_panel(dc, {s(24), s(72), s(720), s(160)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));
  draw_panel(dc, {s(24), s(178), s(720), s(278)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));
  draw_panel(dc, {s(24), s(302), s(720), s(426)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));

  RECT action_bar{s(0), s(444), s(760), s(540)};
  HBRUSH brush = CreateSolidBrush(RGB(0xF0, 0xF3, 0xF8));
  FillRect(dc, &action_bar, brush);
  DeleteObject(brush);
}

void SettingsWindow::load_config() {
  const auto loaded = runtime_.config_store.load();
  if (!loaded) {
    show_error(hwnd(), loaded.error());
    return;
  }

  if (!loaded.value()) {
    return;
  }

  const auto& config = *loaded.value();
  bin_text_.set_text(config.bin_directory.wstring());
  path_checkbox_.set_check(config.path_enabled);
  context_menu_checkbox_.set_check(config.context_menu_enabled);
}

void SettingsWindow::save_config() {
  core::Config config;
  config.bin_directory = bin_text_.get_text();
  config.path_enabled = path_checkbox_.is_checked();
  config.context_menu_enabled = context_menu_checkbox_.is_checked();

  const auto saved = runtime_.settings_workflow.save(app::SettingsSaveRequest{
    .config = std::move(config),
    .executable_path = runtime_.executable_path(),
  });
  if (!saved) {
    show_error(hwnd(), saved.error());
    return;
  }

  static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings saved."));
  show_info(hwnd(), L"Settings saved successfully.");
}

void SettingsWindow::browse_bin_directory() {
  const auto path = choose_folder(hwnd());
  if (!path.empty()) {
    bin_text_.set_text(path.wstring());
  }
}

void SettingsWindow::open_bin_directory() const {
  const auto path = bin_text_.get_text();
  if (path.empty()) {
    MessageBoxW(hwnd(), L"Configure a Bin directory first.", text::kAppTitle, MB_ICONWARNING | MB_OK);
    return;
  }

  const auto opened = runtime_.shell_service.open_directory(path);
  if (!opened) {
    show_error(hwnd(), opened.error());
  }
}

} // namespace binify::ui
