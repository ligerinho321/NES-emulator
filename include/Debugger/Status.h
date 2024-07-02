#pragma once

#include <Emulator.h>

void Status_Init(DebuggerStatus *status);

void Status_ConfigRect(DebuggerStatus *status);

void Status_Event(DebuggerStatus *status);

void Status_Draw(DebuggerStatus *status);

void Status_Run(DebuggerStatus *status);

void Status_Free(DebuggerStatus *status);