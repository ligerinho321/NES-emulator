#pragma once

#include <Emulator.h>

void DebuggerMenuBar_Init(DebuggerMenuBar *menu_bar);

void DebuggerMenuBar_ConfigRect(DebuggerMenuBar *menu_bar);

void DebuggerMenuBar_DrawPopUps(DebuggerMenuBar *menu_bar);

void DebuggerMenuBar_Run(DebuggerMenuBar *menu_bar);

void DebuggerMenuBar_Free(DebuggerMenuBar *menu_bar);