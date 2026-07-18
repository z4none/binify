#include "ui/conflict_dialog.h"

namespace binify::ui {

bool confirm_overwrite(
  HWND owner,
  const std::vector<std::filesystem::path>& conflict_paths,
  const core::Translator& translator) {
  std::wstring message = translator.text("conflict.overwrite_prompt");
  for (const auto& path : conflict_paths) {
    message += L"\r\n";
    message += path.wstring();
  }

  return MessageBoxW(owner, message.c_str(), translator.text("app.title").c_str(), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES;
}

} // namespace binify::ui
