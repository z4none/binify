#pragma once

#include "app/add_command_workflow.h"

#include <windows.h>

namespace binify::ui {

void show_add_success(HWND owner, const app::AddCommandResult& result);

} // namespace binify::ui
