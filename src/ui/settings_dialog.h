#pragma once

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#include <vector>

#include "app/bin_entries.h"
#include "ui/ui_runtime.h"
#include "ui/ui_theme.h"

#include "button.h"
#include "checkbox.h"
#include "combobox.h"
#include "label.h"
#include "textbox.h"
#include "window_main.h"

namespace binify::ui {

class SettingsWindow final : public wl::window_main {
public:
  explicit SettingsWindow(RuntimeContext& runtime);
  SettingsWindow(RuntimeContext& runtime, bool close_after_success);

private:
  void create_controls();
  void layout_controls();
  void draw(HDC dc) const;
  void load_config();
  void save_config();
  void browse_bin_directory();
  void open_bin_directory() const;
  void populate_languages();
  void create_tab_control();
  void create_entries_controls();
  void show_tab(int tab_index);
  void refresh_entries();
  void add_entry();
  void rename_selected_entry();
  void delete_selected_entry();
  void update_rename_text_from_selection();
  [[nodiscard]] int selected_entry_index() const;
  [[nodiscard]] core::Config current_config_or_empty() const;
  [[nodiscard]] std::filesystem::path choose_source_executable() const;
  [[nodiscard]] std::wstring selected_language() const;

  RuntimeContext& runtime_;
  bool close_after_success_ = false;
  Theme theme_;
  wl::label bin_label_;
  wl::textbox bin_text_;
  wl::button browse_button_;
  wl::checkbox path_checkbox_;
  wl::checkbox context_menu_checkbox_;
  wl::label language_label_;
  wl::combobox language_combo_;
  wl::label help_label_;
  wl::button open_bin_button_;
  wl::button save_button_;
  wl::button cancel_button_;
  HWND entries_list_ = nullptr;
  wl::button refresh_entries_button_;
  wl::button add_entry_button_;
  wl::button rename_entry_button_;
  wl::button delete_entry_button_;
  wl::button open_bin_entries_button_;
  wl::label rename_label_;
  wl::textbox rename_text_;
  std::vector<core::AvailableLanguage> languages_;
  std::vector<app::BinEntry> entries_;
  HWND tab_control_ = nullptr;
  int active_tab_ = 0;
  std::wstring window_title_;
};

} // namespace binify::ui
