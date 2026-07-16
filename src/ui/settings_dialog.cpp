#include "ui/settings_dialog.h"

#include <shlobj.h>
#include <shellapi.h>

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
  BROWSEINFOW browse_info{};
  browse_info.hwndOwner = owner;
  browse_info.lpszTitle = L"Select Bin directory";
  browse_info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

  PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&browse_info);
  if (item == nullptr) {
    return {};
  }

  wchar_t path[MAX_PATH]{};
  const bool ok = SHGetPathFromIDListW(item, path) == TRUE;
  CoTaskMemFree(item);
  return ok ? std::filesystem::path{path} : std::filesystem::path{};
}

} // namespace

SettingsWindow::SettingsWindow(RuntimeContext& runtime) : runtime_(runtime) {
  setup.wndClassEx.lpszClassName = L"BINIFY_SETTINGS_WINDOW";
  setup.title = text::kSettingsTitle;
  setup.size = {620, 320};
  setup.style |= WS_MINIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    create_controls();
    load_config();
    return 0;
  });

  on_message(WM_COMMAND, [this](wl::wm::command command) -> LRESULT {
    switch (command.control_id()) {
    case kIdBrowse:
      browse_bin_directory();
      return 0;
    case kIdOpenBin:
      open_bin_directory();
      return 0;
    case kIdSave:
      save_config();
      return 0;
    case kIdCancel:
      DestroyWindow(hwnd());
      return 0;
    default:
      return 0;
    }
  });
}

void SettingsWindow::create_controls() {
  bin_label_.create(this, -1, L"Bin directory", {20, 24}, {120, 20});
  bin_text_.create(this, kIdBinText, wl::textbox::type::NORMAL, {140, 20}, 340);
  browse_button_.create(this, kIdBrowse, text::kBrowse, {500, 19}, {90, 24});

  path_checkbox_.create(this, kIdPath, text::kPathToggle, {140, 58}, {380, 24});
  context_menu_checkbox_.create(this, kIdContextMenu, text::kContextMenuToggle, {140, 90}, {420, 24});

  help_label_.create(this, -1, L"Settings are saved for the current user. No administrator permission is required.", {20, 132}, {560, 40});
  open_bin_button_.create(this, kIdOpenBin, text::kOpenBin, {140, 190}, {150, 28});
  save_button_.create(this, kIdSave, text::kSave, {380, 230}, {90, 28});
  cancel_button_.create(this, kIdCancel, text::kCancel, {500, 230}, {90, 28});
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

  const auto result = reinterpret_cast<INT_PTR>(
    ShellExecuteW(hwnd(), L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
  if (result <= 32) {
    MessageBoxW(hwnd(), L"Failed to open the Bin directory.", text::kAppTitle, MB_ICONERROR | MB_OK);
  }
}

} // namespace binify::ui
