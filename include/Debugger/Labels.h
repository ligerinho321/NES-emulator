#pragma once

#include <Emulator.h>

void Labels_Init(DebuggerLabels *labels);

void Labels_ConfigRect(DebuggerLabels *labels);

void Labels_Event(DebuggerLabels *labels);

void Labels_Draw(DebuggerLabels *labels);

void Labels_Run(DebuggerLabels *labels);

void Labels_Free(DebuggerLabels *labels);