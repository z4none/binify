#include "ui/ui_application.h"

#include <exception>
#include <filesystem>

#include "core/config.h"
#include "ui/add_command_dialog.h"
#include "ui/settings_dialog.h"
#include "ui/ui_runtime.h"
#include "ui/uninstall_dialog.h"

namespace binify::ui {
namespace {

[[nodiscard]] bool is_option(const std::wstring& value, const wchar_t* option) {
  return _wcsicmp(value.c_str(), option) == 0;
}

[[nodiscard]] int show_exception(const wchar_t* message) noexcept {
  MessageBoxW(nullptr, message, L"binify", MB_ICONERROR | MB_OK);
  return 1;
}

[[nodiscard]] bool has_configured_bin(RuntimeContext& runtime) {
  const auto loaded = runtime.config_store.load();
  if (!loaded) {
    show_error(nullptr, loaded.error());
    return false;
  }
  return loaded.value().has_value() && core::is_configured(*loaded.value());
}

[[nodiscard]] int run_add_flow(
  HINSTANCE instance,
  int command_show,
  RuntimeContext& runtime,
  std::filesystem::path source_path) {
  if (!has_configured_bin(runtime)) {
    MessageBoxW(
      nullptr,
      runtime.text("add.configure_first_continue").c_str(),
      runtime.text("app.title").c_str(),
      MB_ICONINFORMATION | MB_OK);

    SettingsWindow settings{runtime, true};
    const int settings_result = settings.winmain_run(instance, command_show);
    if (settings_result != 0 || !has_configured_bin(runtime)) {
      return settings_result;
    }
  }

  AddCommandWindow window{runtime, std::move(source_path)};
  return window.winmain_run(instance, command_show);
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

    const auto executable_path = current_executable_path();
    RuntimeContext runtime{
      config_path.value(),
      log_directory.value(),
      default_language_directory(executable_path),
      executable_path};
    static_cast<void>(runtime.logger.write(app::LogLevel::info, L"binify UI started."));

    if (arguments.size() >= 3 && is_option(arguments[1], L"--add")) {
      return run_add_flow(instance, command_show, runtime, arguments[2]);
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
