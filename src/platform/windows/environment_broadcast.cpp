#include "platform/windows/environment_broadcast.h"

#include <windows.h>

#include "core/error.h"

namespace binify::platform::windows {

core::Result<void> broadcast_environment_change() {
  DWORD_PTR result = 0;
  const auto sent = SendMessageTimeoutW(
    HWND_BROADCAST,
    WM_SETTINGCHANGE,
    0,
    reinterpret_cast<LPARAM>(L"Environment"),
    SMTO_ABORTIFHUNG,
    5000,
    &result);

  if (sent == 0) {
    return core::make_error(
      core::ErrorCode::platform_error,
      L"Failed to broadcast environment change.",
      GetLastError(),
      std::nullopt);
  }

  return {};
}

} // namespace binify::platform::windows

