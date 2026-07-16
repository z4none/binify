#include "ui/ui_application.h"

#include <exception>

#include "ui/add_command_dialog.h"
#include "ui/settings_dialog.h"
#include "ui/ui_runtime.h"
#include "ui/ui_text.h"
#include "ui/uninstall_dialog.h"

namespace binify::ui {
namespace {

[[nodiscard]] bool is_option(const std::wstring& value, const wchar_t* option) {
  return _wcsicmp(value.c_str(), option) == 0;
}

[[nodiscard]] int show_exception(const wchar_t* message) noexcept {
  MessageBoxW(nullptr, message, text::kAppTitle, MB_ICONERROR | MB_OK);
  return 1;
}

} // namespace

int run_ui(HINSTANCE instance, int command_show, const std::vector<std::wstring>& arguments) noexcept {
  try {
    auto config_path = default_config_path();
    if (!config_path) {
      show_error(nullptr, config_path.error());
      return 1;
    }
    auto log_directory = default_log_directory();
    if (!log_directory) {
      show_error(nullptr, log_directory.error());
      return 1;
    }

    RuntimeContext runtime{config_path.value(), log_directory.value(), current_executable_path()};
    static_cast<void>(runtime.logger.write(app::LogLevel::info, L"binify UI started."));

    if (arguments.size() >= 3 && is_option(arguments[1], L"--add")) {
      AddCommandWindow window{runtime, arguments[2]};
      return window.winmain_run(instance, command_show);
    }

    if (arguments.size() >= 2 && is_option(arguments[1], L"--uninstall-cleanup")) {
      UninstallWindow window{runtime};
      return window.winmain_run(instance, command_show);
    }

    SettingsWindow window{runtime};
    return window.winmain_run(instance, command_show);
  } catch (const std::exception&) {
    return show_exception(L"binify failed with an unexpected error.");
  } catch (...) {
    return show_exception(L"binify failed with an unknown error.");
  }
}

} // namespace binify::ui
