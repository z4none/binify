#include <windows.h>

#include "app/application.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  binify::app::Application application;
  const auto result = application.run();
  if (!result) {
    return 1;
  }
  return result.value();
}

