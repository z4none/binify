#include "ui/result_dialog.h"

#include "core/creation_policy.h"
#include "ui/ui_text.h"

namespace binify::ui {

void show_add_success(HWND owner, const app::AddCommandResult& result) {
  std::wstring message = L"Command entry created successfully.\r\n\r\nName: ";
  message += result.command_name.normalized;
  message += L"\r\nMode: ";
  message += core::display_name(result.actual_mode);
  MessageBoxW(owner, message.c_str(), text::kAppTitle, MB_ICONINFORMATION | MB_OK);
}

} // namespace binify::ui
