#pragma once

#include <filesystem>

#include <windows.h>
#include <shellapi.h>

#include "ui/ui_runtime.h"
#include "ui/ui_theme.h"

#include "button.h"
#include "combobox.h"
#include "label.h"
#include "textbox.h"
#include "window_main.h"

namespace binify::ui {

class AddCommandWindow final : public wl::window_main {
public:
  AddCommandWindow(RuntimeContext& runtime, std::filesystem::path source_path);

private:
  void create_controls();
  void draw(HDC dc) const;
  void load_config();
  void update_entry_preview();
  void create_command();
  [[nodiscard]] core::LinkMode selected_link_mode() const;

  RuntimeContext& runtime_;
  Theme theme_;
  std::filesystem::path source_path_;
  core::Config config_;

  wl::label title_label_;
  wl::label source_label_;
  wl::label source_value_;
  wl::label name_label_;
  wl::textbox name_text_;
  wl::label mode_label_;
  wl::combobox mode_combo_;
  wl::label mode_help_;
  wl::label entry_preview_;
  wl::button create_button_;
  wl::button cancel_button_;
};

} // namespace binify::ui
