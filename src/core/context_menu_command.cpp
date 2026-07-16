#include "core/context_menu_command.h"

#include "core/error.h"

namespace binify::core {

Result<std::wstring> format_context_menu_command(const std::filesystem::path& executable_path) {
  if (executable_path.empty() || !executable_path.is_absolute()) {
    return make_error(
      ErrorCode::invalid_argument,
      L"Context menu executable path must be absolute.",
      std::nullopt,
      executable_path.wstring());
  }

  std::wstring command;
  command.reserve(executable_path.wstring().size() + 16);
  command += L"\"";
  command += executable_path.wstring();
  command += L"\" --add \"%1\"";
  return command;
}

} // namespace binify::core

