#pragma once

#include "app/add_command_workflow.h"
#include "core/language_pack.h"

#include <windows.h>

namespace binify::ui {

void show_add_success(HWND owner, const app::AddCommandResult& result, const core::Translator& translator);

} // namespace binify::ui
