#pragma once

#include <windows.h>
#include <shellapi.h>

#include "ui/ui_runtime.h"

#include "button.h"
#include "checkbox.h"
#include "label.h"
#include "window_main.h"

namespace binify::ui {

class UninstallWindow final : public wl::window_main {
public:
  explicit UninstallWindow(RuntimeContext& runtime);

private:
  void create_controls();
  void load_config();
  void run_cleanup();

  RuntimeContext& runtime_;
  core::Config config_;
  wl::label summary_label_;
  wl::checkbox path_checkbox_;
  wl::checkbox context_menu_checkbox_;
  wl::button cleanup_button_;
  wl::button cancel_button_;
};

} // namespace binify::ui
