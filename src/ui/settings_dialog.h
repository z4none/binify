#pragma once

#include <windows.h>
#include <shellapi.h>

#include <vector>

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
  void draw(HDC dc) const;
  void load_config();
  void save_config();
  void browse_bin_directory();
  void open_bin_directory() const;
  void populate_languages();
  [[nodiscard]] std::wstring selected_language() const;

  RuntimeContext& runtime_;
  bool close_after_success_ = false;
  Theme theme_;
  wl::label title_label_;
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
  std::vector<core::AvailableLanguage> languages_;
  std::wstring window_title_;
};

} // namespace binify::ui
