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
  setup.size = scale_size_for_system_dpi(780, 560);
  setup.style |= WS_MINIMIZEBOX;

  on_message(WM_CREATE, [this](wl::wm::create) -> LRESULT {
    theme_.initialize(hwnd());
    create_controls();
    load_config();
    update_entry_preview();
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

  on_message(WM_COMMAND, [this](wl::wm::command command) -> LRESULT {
    switch (command.control_id()) {
    case kIdName:
    case kIdMode:
      update_entry_preview();
      return 0;
    case kIdCreate:
      static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Add command Create clicked."));
      create_command();
      return 0;
    case kIdCancel:
      static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Add command Cancel clicked."));
      DestroyWindow(hwnd());
      return 0;
    default:
      return 0;
    }
  });
}

void AddCommandWindow::create_controls() {
  const auto s = [this](int value) { return theme_.scale(value); };

  title_label_.create(this, -1, L"➕  Add command", {s(24), s(18)}, {s(420), s(34)});
  apply_font(title_label_.hwnd(), theme_.title_font());
  make_transparent_control(title_label_.hwnd());

  source_label_.create(this, -1, L"Source executable", {s(44), s(88)}, {s(160), s(22)});
  apply_font(source_label_.hwnd(), theme_.body_font());
  make_transparent_control(source_label_.hwnd());
  source_value_.create(this, -1, source_path_.wstring().c_str(), {s(44), s(118)}, {s(660), s(42)});
  apply_font(source_value_.hwnd(), theme_.small_font());
  make_transparent_control(source_value_.hwnd());

  name_label_.create(this, -1, L"Command name", {s(44), s(208)}, {s(140), s(22)});
  apply_font(name_label_.hwnd(), theme_.body_font());
  make_transparent_control(name_label_.hwnd());
  name_text_.create(this, kIdName, wl::textbox::type::NORMAL, {s(190), s(204)}, s(260), s(25));
  apply_font(name_text_.hwnd(), theme_.body_font());
  name_text_.set_text(default_command_name(source_path_));

  mode_label_.create(this, -1, L"Link mode", {s(44), s(252)}, {s(140), s(22)});
  apply_font(mode_label_.hwnd(), theme_.body_font());
  make_transparent_control(mode_label_.hwnd());
  mode_combo_.create(this, kIdMode, {s(190), s(248)}, s(260), wl::combobox::sort::UNSORTED);
  apply_font(mode_combo_.hwnd(), theme_.body_font());
  mode_combo_.add({L"Auto", L"Symbolic Link", L"Hard Link", L"CMD Wrapper"});
  mode_combo_.select(0);

  mode_help_.create(this, -1, text::kLinkModeHelp, {s(190), s(286)}, {s(500), s(46)});
  apply_font(mode_help_.hwnd(), theme_.small_font());
  make_transparent_control(mode_help_.hwnd());
  entry_preview_.create(this, -1, L"", {s(44), s(382)}, {s(680), s(44)});
  apply_font(entry_preview_.hwnd(), theme_.body_font());
  make_transparent_control(entry_preview_.hwnd());

  create_button_.create(this, kIdCreate, L"✓  Create", {s(560), s(482)}, {s(100), s(36)});
  apply_font(create_button_.hwnd(), theme_.body_font());
  make_modern_button(create_button_.hwnd(), ButtonRole::primary);
  cancel_button_.create(this, kIdCancel, text::kCancel, {s(674), s(482)}, {s(82), s(36)});
  apply_font(cancel_button_.hwnd(), theme_.body_font());
  make_modern_button(cancel_button_.hwnd(), ButtonRole::secondary);
}

void AddCommandWindow::draw(HDC dc) const {
  const auto s = [this](int value) { return theme_.scale(value); };
  draw_window_background(hwnd(), dc, RGB(0xF6, 0xF8, 0xFB));
  draw_panel(dc, {s(24), s(72), s(744), s(172)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));
  draw_panel(dc, {s(24), s(192), s(744), s(342)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));
  draw_panel(dc, {s(24), s(366), s(744), s(440)}, RGB(0xFF, 0xFF, 0xFF), RGB(0xE3, 0xE8, 0xF0), s(18));

  RECT action_bar{s(0), s(464), s(780), s(560)};
  HBRUSH brush = CreateSolidBrush(RGB(0xF0, 0xF3, 0xF8));
  FillRect(dc, &action_bar, brush);
  DeleteObject(brush);
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

  static_cast<void>(runtime_.logger.write(app::LogLevel::info, L"Command entry created."));
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
