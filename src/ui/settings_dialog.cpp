#include "ui/settings_dialog.h"

#include <commdlg.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <shlobj.h>
#include <objbase.h>
#include <vector>

#include "core/config.h"
#include "ui/add_command_dialog.h"

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
constexpr int kIdTab = 1100;
constexpr int kIdEntriesList = 1101;
constexpr int kIdRefreshEntries = 1102;
constexpr int kIdAddEntry = 1103;
constexpr int kIdRenameEntry = 1104;
constexpr int kIdDeleteEntry = 1105;
constexpr int kIdOpenBinEntries = 1106;
constexpr int kIdRenameText = 1107;
constexpr UINT_PTR kTabPollTimer = 1201;
constexpr int kMinWindowWidth = 720;
constexpr int kMinWindowHeight = 520;

void set_visible(HWND control, bool visible) {
  if (control != nullptr) {
    ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
  }
}

void move_window(HWND control, int x, int y, int width, int height) {
  if (control != nullptr) {
    SetWindowPos(control, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
  }
}

std::wstring modified_time_text(std::filesystem::file_time_type time) {
  const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
    time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
  const auto time_t = std::chrono::system_clock::to_time_t(system_time);
  std::tm local_time{};
  if (localtime_s(&local_time, &time_t) != 0) {
    return L"";
  }
  wchar_t buffer[32]{};
  if (wcsftime(buffer, _countof(buffer), L"%Y-%m-%d %H:%M", &local_time) == 0) {
    return L"";
  }
  return buffer;
}

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
  window_title_ = L"binify";
  setup.wndClassEx.lpszClassName = L"BINIFY_SETTINGS_WINDOW";
  setup.wndClassEx.hIcon = app_icon(32);
  setup.wndClassEx.hIconSm = app_icon(16);
  setup.title = window_title_.c_str();
  setup.size = scale_size_for_system_dpi(860, 620);
  setup.style |= WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    INITCOMMONCONTROLSEX controls{
      .dwSize = sizeof(INITCOMMONCONTROLSEX),
      .dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES,
    };
    InitCommonControlsEx(&controls);
    theme_.initialize(hwnd());
    create_tab_control();
    create_controls();
    create_entries_controls();
    load_config();
    show_tab(0);
    RECT client{};
    GetClientRect(hwnd(), &client);
    layout_controls(client.right - client.left, client.bottom - client.top);
    SetTimer(hwnd(), kTabPollTimer, 150, nullptr);
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

  on_message(WM_SIZE, [this](wl::params params) -> LRESULT {
    layout_controls(LOWORD(params.lParam), HIWORD(params.lParam));
    InvalidateRect(hwnd(), nullptr, TRUE);
    return 0;
  });

  on_message(WM_GETMINMAXINFO, [](wl::params params) -> LRESULT {
    auto* info = reinterpret_cast<MINMAXINFO*>(params.lParam);
    info->ptMinTrackSize.x = kMinWindowWidth;
    info->ptMinTrackSize.y = kMinWindowHeight;
    return 0;
  });

  on_notify(kIdTab, TCN_SELCHANGE, [this](wl::params) -> LRESULT {
    show_tab(TabCtrl_GetCurSel(tab_control_));
    return 0;
  });

  on_notify(kIdEntriesList, LVN_ITEMCHANGED, [this](wl::params) -> LRESULT {
    update_rename_text_from_selection();
    return 0;
  });

  on_message(WM_TIMER, [this](wl::params params) -> LRESULT {
    if (params.wParam == kTabPollTimer && tab_control_ != nullptr) {
      const auto selected = TabCtrl_GetCurSel(tab_control_);
      if (selected != active_tab_) {
        show_tab(selected);
      }
    }
    return 0;
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

  on_command(kIdRefreshEntries, [this](wl::params) -> LRESULT {
    refresh_entries();
    return 0;
  });

  on_command(kIdAddEntry, [this](wl::params) -> LRESULT {
    add_entry();
    return 0;
  });

  on_command(kIdRenameEntry, [this](wl::params) -> LRESULT {
    rename_selected_entry();
    return 0;
  });

  on_command(kIdDeleteEntry, [this](wl::params) -> LRESULT {
    delete_selected_entry();
    return 0;
  });

  on_command(kIdOpenBinEntries, [this](wl::params) -> LRESULT {
    open_bin_directory();
    return 0;
  });

}

void SettingsWindow::create_controls() {
  const auto s = [](int value) { return value; };

  bin_label_.create(this, -1, runtime_.text("settings.bin_directory").c_str(), {s(56), s(128)}, {s(150), s(22)});
  apply_font(bin_label_.hwnd(), theme_.body_font());
  make_transparent_control(bin_label_.hwnd());
  bin_text_.create(this, kIdBinText, wl::textbox::type::NORMAL, {s(56), s(156)}, s(344), s(25));
  apply_font(bin_text_.hwnd(), theme_.body_font());
  const auto browse_text = L"📁  " + runtime_.text("common.browse");
  browse_button_.create(this, kIdBrowse, browse_text.c_str(), {s(436), s(152)}, {s(128), s(36)});
  apply_font(browse_button_.hwnd(), theme_.body_font());
  make_modern_button(browse_button_.hwnd(), ButtonRole::secondary);
  const auto open_bin_text = L"📂  " + runtime_.text("common.open_bin");
  open_bin_button_.create(this, kIdOpenBin, open_bin_text.c_str(), {s(584), s(152)}, {s(128), s(36)});
  apply_font(open_bin_button_.hwnd(), theme_.body_font());
  make_modern_button(open_bin_button_.hwnd(), ButtonRole::secondary);

  path_checkbox_.create(this, kIdPath, runtime_.text("settings.path_toggle").c_str(), {s(56), s(224)}, {s(680), s(26)});
  apply_font(path_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(path_checkbox_.hwnd());
  context_menu_checkbox_.create(this, kIdContextMenu, runtime_.text("settings.context_menu_toggle").c_str(), {s(56), s(264)}, {s(680), s(26)});
  apply_font(context_menu_checkbox_.hwnd(), theme_.body_font());
  make_transparent_control(context_menu_checkbox_.hwnd());

  language_label_.create(this, -1, runtime_.text("settings.language").c_str(), {s(56), s(320)}, {s(140), s(22)});
  apply_font(language_label_.hwnd(), theme_.body_font());
  make_transparent_control(language_label_.hwnd());
  language_combo_.create(this, kIdLanguage, {s(56), s(348)}, s(320), wl::combobox::sort::UNSORTED);
  SetWindowPos(language_combo_.hwnd(), nullptr, 0, 0, s(320), s(140), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  apply_font(language_combo_.hwnd(), theme_.body_font());
  populate_languages();

  help_label_.create(this, -1, runtime_.text("settings.help").c_str(), {s(56), s(408)}, {s(720), s(28)});
  apply_font(help_label_.hwnd(), theme_.small_font());
  make_transparent_control(help_label_.hwnd());

  const auto save_text = L"✓  " + runtime_.text("common.save");
  save_button_.create(this, kIdSave, save_text.c_str(), {s(642), s(502)}, {s(90), s(36)});
  apply_font(save_button_.hwnd(), theme_.body_font());
  make_modern_button(save_button_.hwnd(), ButtonRole::primary);
  cancel_button_.create(this, kIdCancel, runtime_.text("common.cancel").c_str(), {s(746), s(502)}, {s(82), s(36)});
  apply_font(cancel_button_.hwnd(), theme_.body_font());
  make_modern_button(cancel_button_.hwnd(), ButtonRole::secondary);
}

void SettingsWindow::layout_controls(int width, int height) {
  width = MulDiv(width, 96, static_cast<int>(theme_.dpi()));
  height = MulDiv(height, 96, static_cast<int>(theme_.dpi()));
  width = std::max(width, kMinWindowWidth);
  height = std::max(height, kMinWindowHeight);

  move_window(tab_control_, 0, 0, width, height);
  if (tab_control_ != nullptr) {
    SetWindowPos(tab_control_, HWND_BOTTOM, 0, 0, width, height, SWP_NOACTIVATE);
  }

  const int margin = 32;
  const int page_top = 48;
  const int gap = 16;
  const int button_width = 128;
  const int button_height = 36;
  const int small_button_width = 90;
  const int small_button_height = 32;
  const int right = width - margin;
  const int bottom = height - margin;

  const int open_bin_x = right - button_width;
  const int browse_x = open_bin_x - gap - button_width;
  const int text_width = std::max(220, browse_x - margin - gap);
  move_window(bin_label_.hwnd(), margin, page_top, 150, 22);
  move_window(bin_text_.hwnd(), margin, page_top + 28, text_width, 25);
  move_window(browse_button_.hwnd(), browse_x, page_top + 24, button_width, button_height);
  move_window(open_bin_button_.hwnd(), open_bin_x, page_top + 24, button_width, button_height);

  move_window(path_checkbox_.hwnd(), margin, page_top + 96, std::max(300, width - margin * 2), 26);
  move_window(context_menu_checkbox_.hwnd(), margin, page_top + 136, std::max(300, width - margin * 2), 26);
  move_window(language_label_.hwnd(), margin, page_top + 192, 140, 22);
  move_window(language_combo_.hwnd(), margin, page_top + 220, 320, 140);
  move_window(help_label_.hwnd(), margin, std::min(page_top + 280, bottom - 100), std::max(300, width - margin * 2), 28);
  move_window(save_button_.hwnd(), right - 194, bottom - button_height, 90, button_height);
  move_window(cancel_button_.hwnd(), right - 82, bottom - button_height, 82, button_height);

  const int list_top = page_top;
  const int list_height = std::max(150, height - 236);
  move_window(entries_list_, margin, list_top, width - margin * 2, list_height);
  if (entries_list_ != nullptr) {
    const int list_width = width - margin * 2 - 4;
    ListView_SetColumnWidth(entries_list_, 0, 130);
    ListView_SetColumnWidth(entries_list_, 1, 110);
    ListView_SetColumnWidth(entries_list_, 2, std::max(180, list_width - 610));
    ListView_SetColumnWidth(entries_list_, 3, 110);
    ListView_SetColumnWidth(entries_list_, 4, 260);
  }

  const int entries_button_top = list_top + list_height + 24;
  move_window(refresh_entries_button_.hwnd(), margin, entries_button_top, small_button_width, small_button_height);
  move_window(add_entry_button_.hwnd(), margin + 104, entries_button_top, small_button_width, small_button_height);
  move_window(delete_entry_button_.hwnd(), margin + 208, entries_button_top, small_button_width, small_button_height);
  move_window(open_bin_entries_button_.hwnd(), margin + 312, entries_button_top, 104, small_button_height);
  move_window(rename_label_.hwnd(), margin, entries_button_top + 56, 120, 22);
  move_window(rename_text_.hwnd(), margin + 120, entries_button_top + 52, 220, 25);
  move_window(rename_entry_button_.hwnd(), margin + 354, entries_button_top + 48, small_button_width, small_button_height);

  const std::vector<HWND> config_controls{
    bin_label_.hwnd(), bin_text_.hwnd(), browse_button_.hwnd(), open_bin_button_.hwnd(),
    path_checkbox_.hwnd(), context_menu_checkbox_.hwnd(), language_label_.hwnd(),
    language_combo_.hwnd(), help_label_.hwnd(), save_button_.hwnd(), cancel_button_.hwnd(),
  };
  const std::vector<HWND> entry_controls{
    entries_list_, refresh_entries_button_.hwnd(), add_entry_button_.hwnd(),
    rename_entry_button_.hwnd(), delete_entry_button_.hwnd(), open_bin_entries_button_.hwnd(),
    rename_label_.hwnd(), rename_text_.hwnd(),
  };
  for (const auto control : active_tab_ == 0 ? config_controls : entry_controls) {
    SetWindowPos(control, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}

void SettingsWindow::draw(HDC dc) const {
  draw_window_background(hwnd(), dc, RGB(0xF6, 0xF8, 0xFB));
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

void SettingsWindow::create_tab_control() {
  const auto s = [](int value) { return value; };
  tab_control_ = CreateWindowExW(
    0,
    WC_TABCONTROLW,
    nullptr,
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
    s(0),
    s(0),
    s(860),
    s(620),
    hwnd(),
    reinterpret_cast<HMENU>(static_cast<UINT_PTR>(kIdTab)),
    reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hwnd(), GWLP_HINSTANCE)),
    nullptr);
  apply_font(tab_control_, theme_.body_font());

  TCITEMW item{};
  item.mask = TCIF_TEXT;
  auto config_text = runtime_.text("tabs.config");
  item.pszText = config_text.data();
  TabCtrl_InsertItem(tab_control_, 0, &item);
  auto entries_text = runtime_.text("tabs.entries");
  item.pszText = entries_text.data();
  TabCtrl_InsertItem(tab_control_, 1, &item);
}

void SettingsWindow::create_entries_controls() {
  const auto s = [](int value) { return value; };
  entries_list_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    s(56),
    s(128),
    s(748),
    s(260),
    hwnd(),
    reinterpret_cast<HMENU>(static_cast<UINT_PTR>(kIdEntriesList)),
    reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hwnd(), GWLP_HINSTANCE)),
    nullptr);
  apply_font(entries_list_, theme_.body_font());
  ListView_SetExtendedListViewStyleEx(entries_list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

  const std::vector<std::pair<std::wstring, int>> columns{
    {runtime_.text("entries.name"), s(130)},
    {runtime_.text("entries.type"), s(110)},
    {runtime_.text("entries.target"), s(210)},
    {runtime_.text("entries.modified"), s(110)},
    {runtime_.text("entries.path"), s(260)},
  };
  for (std::size_t index = 0; index < columns.size(); ++index) {
    LVCOLUMNW column{};
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.pszText = const_cast<wchar_t*>(columns[index].first.c_str());
    column.cx = columns[index].second;
    ListView_InsertColumn(entries_list_, static_cast<int>(index), &column);
  }

  const auto refresh_text = runtime_.text("entries.refresh");
  refresh_entries_button_.create(this, kIdRefreshEntries, refresh_text.c_str(), {s(56), s(412)}, {s(90), s(32)});
  apply_font(refresh_entries_button_.hwnd(), theme_.body_font());
  make_modern_button(refresh_entries_button_.hwnd(), ButtonRole::secondary);

  const auto add_text = runtime_.text("entries.add");
  add_entry_button_.create(this, kIdAddEntry, add_text.c_str(), {s(160), s(412)}, {s(90), s(32)});
  apply_font(add_entry_button_.hwnd(), theme_.body_font());
  make_modern_button(add_entry_button_.hwnd(), ButtonRole::secondary);

  const auto delete_text = runtime_.text("entries.delete");
  delete_entry_button_.create(this, kIdDeleteEntry, delete_text.c_str(), {s(264), s(412)}, {s(90), s(32)});
  apply_font(delete_entry_button_.hwnd(), theme_.body_font());
  make_modern_button(delete_entry_button_.hwnd(), ButtonRole::danger);

  const auto open_text = runtime_.text("common.open_bin");
  open_bin_entries_button_.create(this, kIdOpenBinEntries, open_text.c_str(), {s(368), s(412)}, {s(104), s(32)});
  apply_font(open_bin_entries_button_.hwnd(), theme_.body_font());
  make_modern_button(open_bin_entries_button_.hwnd(), ButtonRole::secondary);

  rename_label_.create(this, -1, runtime_.text("entries.new_name").c_str(), {s(56), s(468)}, {s(120), s(22)});
  apply_font(rename_label_.hwnd(), theme_.body_font());
  make_transparent_control(rename_label_.hwnd());
  rename_text_.create(this, kIdRenameText, wl::textbox::type::NORMAL, {s(176), s(464)}, s(220), s(25));
  apply_font(rename_text_.hwnd(), theme_.body_font());
  const auto rename_text = runtime_.text("entries.rename");
  rename_entry_button_.create(this, kIdRenameEntry, rename_text.c_str(), {s(410), s(460)}, {s(90), s(32)});
  apply_font(rename_entry_button_.hwnd(), theme_.body_font());
  make_modern_button(rename_entry_button_.hwnd(), ButtonRole::secondary);
}

void SettingsWindow::show_tab(int tab_index) {
  active_tab_ = tab_index == 1 ? 1 : 0;
  const bool config_visible = active_tab_ == 0;
  const bool entries_visible = active_tab_ == 1;
  const std::vector<HWND> config_controls{
    bin_label_.hwnd(), bin_text_.hwnd(), browse_button_.hwnd(), open_bin_button_.hwnd(),
    path_checkbox_.hwnd(), context_menu_checkbox_.hwnd(), language_label_.hwnd(),
    language_combo_.hwnd(), help_label_.hwnd(), save_button_.hwnd(), cancel_button_.hwnd(),
  };
  for (const auto control : config_controls) {
    set_visible(control, config_visible);
  }

  const std::vector<HWND> entry_controls{
    entries_list_, refresh_entries_button_.hwnd(), add_entry_button_.hwnd(),
    rename_entry_button_.hwnd(), delete_entry_button_.hwnd(), open_bin_entries_button_.hwnd(),
    rename_label_.hwnd(), rename_text_.hwnd(),
  };
  for (const auto control : entry_controls) {
    set_visible(control, entries_visible);
    if (entries_visible) {
      SetWindowPos(control, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
  }

  if (entries_visible) {
    refresh_entries();
  }
  RECT client{};
  GetClientRect(hwnd(), &client);
  layout_controls(client.right - client.left, client.bottom - client.top);
  InvalidateRect(hwnd(), nullptr, TRUE);
}

void SettingsWindow::refresh_entries() {
  entries_.clear();
  ListView_DeleteAllItems(entries_list_);
  const auto config = current_config_or_empty();
  if (!core::is_configured(config)) {
    show_info(hwnd(), runtime_.text("entries.bin_not_configured"));
    return;
  }

  const auto scanned = runtime_.bin_entry_file_system.scan(config.bin_directory);
  if (!scanned) {
    show_error(hwnd(), scanned.error());
    return;
  }

  entries_ = scanned.value();
  for (std::size_t index = 0; index < entries_.size(); ++index) {
    const auto& entry = entries_[index];
    LVITEMW item{};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = static_cast<int>(index);
    item.pszText = const_cast<wchar_t*>(entry.name.c_str());
    item.lParam = static_cast<LPARAM>(index);
    const auto item_index = ListView_InsertItem(entries_list_, &item);
    const auto type_text = std::wstring{app::display_name(entry.type)};
    const auto target_text = entry.target ? entry.target->wstring() : runtime_.text("entries.unknown");
    const auto modified_text = modified_time_text(entry.modified_time);
    const auto path_text = entry.path.wstring();
    ListView_SetItemText(entries_list_, item_index, 1, const_cast<wchar_t*>(type_text.c_str()));
    ListView_SetItemText(entries_list_, item_index, 2, const_cast<wchar_t*>(target_text.c_str()));
    ListView_SetItemText(entries_list_, item_index, 3, const_cast<wchar_t*>(modified_text.c_str()));
    ListView_SetItemText(entries_list_, item_index, 4, const_cast<wchar_t*>(path_text.c_str()));
  }
}

void SettingsWindow::add_entry() {
  const auto source = choose_source_executable();
  if (source.empty()) {
    return;
  }
  AddCommandWindow window{runtime_, source};
  window.winmain_run(reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hwnd(), GWLP_HINSTANCE)), SW_SHOW);
  refresh_entries();
}

void SettingsWindow::rename_selected_entry() {
  const auto index = selected_entry_index();
  if (index < 0 || static_cast<std::size_t>(index) >= entries_.size()) {
    show_info(hwnd(), runtime_.text("entries.select_first"));
    return;
  }

  const auto renamed = runtime_.bin_entry_file_system.rename_entry(entries_[index].path, rename_text_.get_text());
  if (!renamed) {
    show_error(hwnd(), renamed.error());
    return;
  }
  show_info(hwnd(), runtime_.text("entries.renamed"));
  refresh_entries();
}

void SettingsWindow::delete_selected_entry() {
  const auto index = selected_entry_index();
  if (index < 0 || static_cast<std::size_t>(index) >= entries_.size()) {
    show_info(hwnd(), runtime_.text("entries.select_first"));
    return;
  }

  if (MessageBoxW(hwnd(), runtime_.text("entries.delete_confirm").c_str(), runtime_.text("app.title").c_str(), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
    return;
  }

  const auto deleted = runtime_.bin_entry_file_system.delete_entry(entries_[index].path);
  if (!deleted) {
    show_error(hwnd(), deleted.error());
    return;
  }
  refresh_entries();
}

void SettingsWindow::update_rename_text_from_selection() {
  const auto index = selected_entry_index();
  if (index >= 0 && static_cast<std::size_t>(index) < entries_.size()) {
    rename_text_.set_text(entries_[index].path.stem().wstring());
  }
}

int SettingsWindow::selected_entry_index() const {
  const auto selected = ListView_GetNextItem(entries_list_, -1, LVNI_SELECTED);
  if (selected < 0) {
    return -1;
  }
  LVITEMW item{};
  item.mask = LVIF_PARAM;
  item.iItem = selected;
  if (!ListView_GetItem(entries_list_, &item)) {
    return -1;
  }
  return static_cast<int>(item.lParam);
}

core::Config SettingsWindow::current_config_or_empty() const {
  const auto loaded = runtime_.config_store.load();
  if (!loaded || !loaded.value()) {
    return {};
  }
  return *loaded.value();
}

std::filesystem::path SettingsWindow::choose_source_executable() const {
  std::wstring path(MAX_PATH, L'\0');
  OPENFILENAMEW open{};
  open.lStructSize = sizeof(open);
  open.hwndOwner = hwnd();
  open.lpstrFilter = L"Executable files (*.exe)\0*.exe\0All files (*.*)\0*.*\0";
  open.lpstrFile = path.data();
  open.nMaxFile = static_cast<DWORD>(path.size());
  const auto title = runtime_.text("entries.choose_source");
  open.lpstrTitle = title.c_str();
  open.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  if (!GetOpenFileNameW(&open)) {
    return {};
  }
  path.resize(lstrlenW(path.c_str()));
  return path;
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
