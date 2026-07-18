#include "ui/settings_dialog.h"

#include <shlobj.h>
#include <objbase.h>

namespace binify::ui {
namespace {

constexpr int kIdBinText = 1001;
constexpr int kIdBrowse = 1002;
constexpr int kIdPath = 1003;
constexpr int kIdContextMenu = 1004;
constexpr int kIdOpenBin = 1005;
constexpr int kIdLanguage = 1006;
constexpr int kIdSave = 1007;
constexpr int kIdCancel = 1008;

std::filesystem::path choose_folder(HWND owner, const std::wstring& title) {
  const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  BROWSEINFOW browse_info{};
  browse_info.hwndOwner = owner;
  browse_info.lpszTitle = title.c_str();
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

SettingsWindow::SettingsWindow(RuntimeContext& runtime) : SettingsWindow(runtime, false) {}

SettingsWindow::SettingsWindow(RuntimeContext& runtime, bool close_after_success)
  : runtime_(runtime), close_after_success_(close_after_success) {
  window_title_ = runtime_.text("settings.title");
  setup.wndClassEx.lpszClassName = L"BINIFY_SETTINGS_WINDOW";
  setup.wndClassEx.hIcon = app_icon(32);
  setup.wndClassEx.hIconSm = app_icon(16);
  setup.title = window_title_.c_str();
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

  title_label_.create(this, -1, runtime_.text("settings.heading").c_str(), {s(24), s(18)}, {s(420), s(34)});
  apply_font(title_label_.hwnd(), theme_.title_font());
  make_transparent_control(title_label_.hwnd());

  bin_label_.create(this, -1, runtime_.text("settings.bin_directory").c_str(), {s(44), s(88)}, {s(150), s(22)});
  apply_font(bin_label_.hwnd(), theme_.body_font());
  make_transparent_control(bin_label_.hwnd());
  bin_text_.create(this, kIdBinText, wl::textbox::type::NORMAL, {s(44), s(116)}, s(500), s(25));
  apply_font(bin_text_.hwnd(), theme_.body_font());
  const auto browse_text = L"📁  " + runtime_.text("common.browse");
  browse_button_.create(this, kIdBrowse, browse_text.c_str(), {s(565), s(114)}, {s(130), s(32)});
  apply_font(browse_button_.hwnd(), theme_.body_font());
  make_modern_button(browse_button_.hwnd(), ButtonRole::secondary);

  path_checkbox_.create(this, kIdPath, runtime_.text("settings.path_toggle").c_str(), {s(44), s(194)}, {s(520), s(26)});
  apply_font(path_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(path_checkbox_.hwnd());
  context_menu_checkbox_.create(this, kIdContextMenu, runtime_.text("settings.context_menu_toggle").c_str(), {s(44), s(230)}, {s(560), s(26)});
  apply_font(context_menu_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(context_menu_checkbox_.hwnd());

  language_label_.create(this, -1, runtime_.text("settings.language").c_str(), {s(44), s(268)}, {s(140), s(22)});
  apply_font(language_label_.hwnd(), theme_.body_font());
  make_transparent_control(language_label_.hwnd());
  language_combo_.create(this, kIdLanguage, {s(190), s(264)}, s(260), wl::combobox::sort::UNSORTED);
  SetWindowPos(language_combo_.hwnd(), nullptr, 0, 0, s(260), s(140), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  apply_font(language_combo_.hwnd(), theme_.body_font());
  populate_languages();

  help_label_.create(this, -1, runtime_.text("settings.help").c_str(), {s(44), s(326)}, {s(620), s(38)});
  apply_font(help_label_.hwnd(), theme_.small_font());
  make_transparent_control(help_label_.hwnd());
  const auto open_bin_text = L"📂  " + runtime_.text("common.open_bin");
  open_bin_button_.create(this, kIdOpenBin, open_bin_text.c_str(), {s(44), s(380)}, {s(150), s(34)});
  apply_font(open_bin_button_.hwnd(), theme_.body_font());
  make_modern_button(open_bin_button_.hwnd(), ButtonRole::secondary);

  const auto save_text = L"✓  " + runtime_.text("common.save");
  save_button_.create(this, kIdSave, save_text.c_str(), {s(550), s(462)}, {s(90), s(36)});
  apply_font(save_button_.hwnd(), theme_.body_font());
  make_modern_button(save_button_.hwnd(), ButtonRole::primary);
  cancel_button_.create(this, kIdCancel, runtime_.text("common.cancel").c_str(), {s(650), s(462)}, {s(82), s(36)});
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
  for (std::size_t index = 0; index < languages_.size(); ++index) {
    if (languages_[index].language == config.language) {
      language_combo_.select(index);
      break;
    }
  }
}

void SettingsWindow::save_config() {
  core::Config config;
  config.bin_directory = bin_text_.get_text();
  config.language = selected_language();
  config.path_enabled = path_checkbox_.is_checked();
  config.context_menu_enabled = context_menu_checkbox_.is_checked();
  const auto language = config.language;

  const auto saved = runtime_.settings_workflow.save(app::SettingsSaveRequest{
    .config = std::move(config),
    .executable_path = runtime_.executable_path(),
    .context_menu_text = runtime_.text_for_language(language, "context_menu.add"),
  });
  if (!saved) {
    show_error(hwnd(), saved.error());
    return;
  }

  static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Settings saved."));
  show_info(hwnd(), runtime_.text_for_language(language, "settings.saved"));
  if (close_after_success_) {
    DestroyWindow(hwnd());
  }
}

void SettingsWindow::browse_bin_directory() {
  const auto path = choose_folder(hwnd(), runtime_.text("settings.select_bin_directory"));
  if (!path.empty()) {
    bin_text_.set_text(path.wstring());
  }
}

void SettingsWindow::open_bin_directory() const {
  const auto path = bin_text_.get_text();
  if (path.empty()) {
    MessageBoxW(hwnd(), runtime_.text("settings.configure_bin_first").c_str(), runtime_.text("app.title").c_str(), MB_ICONWARNING | MB_OK);
    return;
  }

  const auto opened = runtime_.shell_service.open_directory(path);
  if (!opened) {
    show_error(hwnd(), opened.error());
  }
}

void SettingsWindow::populate_languages() {
  languages_ = runtime_.translator.available_languages();
  for (const auto& language : languages_) {
    SendMessageW(language_combo_.hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(language.display_name.c_str()));
  }
  language_combo_.select(0);
}

std::wstring SettingsWindow::selected_language() const {
  const auto index = language_combo_.get_selected_index();
  if (index >= languages_.size()) {
    return std::wstring{core::kSystemLanguageCode};
  }
  return languages_[index].language;
}

} // namespace binify::ui
