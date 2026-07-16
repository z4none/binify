#pragma once

#include <windows.h>
#include <shellapi.h>

#include "ui/ui_runtime.h"

#include "button.h"
#include "checkbox.h"
#include "label.h"
#include "textbox.h"
#include "window_main.h"

namespace binify::ui {

class SettingsWindow final : public wl::window_main {
public:
  explicit SettingsWindow(RuntimeContext& runtime);

private:
  void create_controls();
  void load_config();
  void save_config();
  void browse_bin_directory();
  void open_bin_directory() const;

  RuntimeContext& runtime_;
  wl::label bin_label_;
  wl::textbox bin_text_;
  wl::button browse_button_;
  wl::checkbox path_checkbox_;
  wl::checkbox context_menu_checkbox_;
  wl::label help_label_;
  wl::button open_bin_button_;
  wl::button save_button_;
  wl::button cancel_button_;
};

} // namespace binify::ui
