#include "ui/add_command_dialog.h"

#include <array>

#include "ui/conflict_dialog.h"
#include "ui/result_dialog.h"
#include "ui/settings_dialog.h"
#include "ui/ui_text.h"

namespace binify::ui {
namespace {

constexpr int kIdName = 2001;
constexpr int kIdMode = 2002;
constexpr int kIdCreate = 2003;
constexpr int kIdCancel = 2004;

constexpr std::array<core::LinkMode, 4> kModes{
  core::LinkMode::auto_select,
  core::LinkMode::symbolic_link,
  core::LinkMode::hard_link,
  core::LinkMode::cmd_wrapper,
};

std::wstring default_command_name(const std::filesystem::path& source_path) {
  auto stem = source_path.stem().wstring();
  return stem.empty() ? source_path.filename().wstring() : stem;
}

} // namespace

AddCommandWindow::AddCommandWindow(RuntimeContext& runtime, std::filesystem::path source_path)
  : runtime_(runtime), source_path_(std::move(source_path)) {
  setup.wndClassEx.lpszClassName = L"BINIFY_ADD_COMMAND_WINDOW";
  setup.title = text::kAddCommandTitle;
  setup.size = {680, 360};
  setup.style |= WS_MINIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    create_controls();
    load_config();
    update_entry_preview();
    return 0;
  });

  on_message(WM_COMMAND, [this](wl::wm::command command) -> LRESULT {
    switch (command.control_id()) {
    case kIdName:
    case kIdMode:
      update_entry_preview();
      return 0;
    case kIdCreate:
      create_command();
      return 0;
    case kIdCancel:
      DestroyWindow(hwnd());
      return 0;
    default:
      return 0;
    }
  });
}

void AddCommandWindow::create_controls() {
  source_label_.create(this, -1, L"Source", {20, 24}, {90, 20});
  source_value_.create(this, -1, source_path_.wstring().c_str(), {120, 24}, {520, 40});

  name_label_.create(this, -1, L"Command name", {20, 82}, {100, 20});
  name_text_.create(this, kIdName, wl::textbox::type::NORMAL, {140, 78}, 260);
  name_text_.set_text(default_command_name(source_path_));

  mode_label_.create(this, -1, L"Link mode", {20, 122}, {100, 20});
  mode_combo_.create(this, kIdMode, {140, 118}, 220, wl::combobox::sort::UNSORTED);
  mode_combo_.add({L"Auto", L"Symbolic Link", L"Hard Link", L"CMD Wrapper"});
  mode_combo_.select(0);

  mode_help_.create(this, -1, text::kLinkModeHelp, {140, 154}, {480, 44});
  entry_preview_.create(this, -1, L"", {140, 210}, {480, 40});

  create_button_.create(this, kIdCreate, text::kCreate, {440, 270}, {90, 28});
  cancel_button_.create(this, kIdCancel, text::kCancel, {550, 270}, {90, 28});
}

void AddCommandWindow::load_config() {
  const auto loaded = runtime_.config_store.load();
  if (!loaded) {
    show_error(hwnd(), loaded.error());
    return;
  }

  if (!loaded.value() || !core::is_configured(*loaded.value())) {
    MessageBoxW(
      hwnd(),
      L"Bin directory is not configured. Open binify normally to configure settings first.",
      text::kAppTitle,
      MB_ICONWARNING | MB_OK);
    return;
  }

  config_ = *loaded.value();
}

void AddCommandWindow::update_entry_preview() {
  if (config_.bin_directory.empty()) {
    entry_preview_.set_text(L"Final entry: configure Bin directory first.");
    return;
  }

  const auto command_name = core::normalize_command_name(name_text_.get_text());
  if (!command_name) {
    entry_preview_.set_text(L"Final entry: invalid command name.");
    return;
  }

  const auto entry = config_.bin_directory / core::final_entry_name(command_name.value(), selected_link_mode());
  std::wstring preview = L"Final entry: ";
  preview += entry.wstring();
  entry_preview_.set_text(preview);
}

void AddCommandWindow::create_command() {
  if (config_.bin_directory.empty()) {
    MessageBoxW(hwnd(), L"Configure Bin directory before creating commands.", text::kAppTitle, MB_ICONWARNING | MB_OK);
    return;
  }

  if (!std::filesystem::exists(source_path_)) {
    MessageBoxW(hwnd(), L"Source executable does not exist.", text::kAppTitle, MB_ICONERROR | MB_OK);
    return;
  }

  auto command_name = core::normalize_command_name(name_text_.get_text());
  if (!command_name) {
    show_error(hwnd(), command_name.error());
    return;
  }

  auto entries = runtime_.directory_listing.list_entries(config_.bin_directory);
  if (!entries) {
    show_error(hwnd(), entries.error());
    return;
  }

  auto conflicts = core::find_command_conflicts(command_name.value().normalized, entries.value());
  app::ConflictDecision decision = app::ConflictDecision::cancel;
  if (!conflicts.empty()) {
    if (!confirm_overwrite(hwnd(), conflicts)) {
      return;
    }
    decision = app::ConflictDecision::overwrite;
  }

  const auto result = runtime_.add_command_workflow.add(app::AddCommandRequest{
    .source_path = source_path_,
    .bin_directory = config_.bin_directory,
    .command_name_input = name_text_.get_text(),
    .requested_mode = selected_link_mode(),
    .conflict_decision = decision,
  });
  if (!result) {
    show_error(hwnd(), result.error());
    return;
  }

  show_add_success(hwnd(), result.value());
}

core::LinkMode AddCommandWindow::selected_link_mode() const {
  const auto index = mode_combo_.get_selected_index();
  if (index >= kModes.size()) {
    return core::LinkMode::auto_select;
  }
  return kModes[index];
}

} // namespace binify::ui
