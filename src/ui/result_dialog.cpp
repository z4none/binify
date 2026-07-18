#include "ui/result_dialog.h"

#include "core/creation_policy.h"

namespace binify::ui {

void show_add_success(HWND owner, const app::AddCommandResult& result, const core::Translator& translator) {
  std::wstring message = translator.text("add.success_message");
  message += L"\r\n\r\n";
  message += translator.text("add.success_name");
  message += L": ";
  message += result.command_name.normalized;
  message += L"\r\n";
  message += translator.text("add.success_mode");
  message += L": ";
  message += core::display_name(result.actual_mode);
  MessageBoxW(owner, message.c_str(), translator.text("app.title").c_str(), MB_ICONINFORMATION | MB_OK);
}

} // namespace binify::ui
