#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

#include "ui/ui_application.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int command_show) {
  int argc = 0;
  PWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  std::vector<std::wstring> arguments;
  if (argv != nullptr) {
    arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
      arguments.emplace_back(argv[index]);
    }
    LocalFree(argv);
  }

  return binify::ui::run_ui(instance, command_show, arguments);
}
