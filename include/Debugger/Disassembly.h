#pragma once

#include <Emulator.h>

void Disassembly_Init(DebuggerDisassembly *disassembly);

void Disassembly_SetVisibleLines(DebuggerDisassembly *disassembly);

void Disassembly_ConfigVerticalScrollBar(DebuggerDisassembly *disassembly);

void Disassembly_ConfigRect(DebuggerDisassembly *disassembly);

void Disassembly_Event(DebuggerDisassembly *disassembly);

void Disassembly_Draw(DebuggerDisassembly *disassembly);

void Disassembly_Run(DebuggerDisassembly *disassembly);

void Disassembly_Free(DebuggerDisassembly *disassembly);