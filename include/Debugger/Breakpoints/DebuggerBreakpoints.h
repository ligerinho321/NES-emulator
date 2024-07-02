#pragma once

#include <Emulator.h>

bool DebuggerBreakpoints_SearchBreakpoint(DebuggerBreakpoints *debugger_breakpoints,uint32_t memory_type,uint32_t break_flag,Range address);

void DebuggerBreakpoints_Init(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_ConfigRect(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_Event(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_Draw(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_Run(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_RunChildrenWindow(DebuggerBreakpoints *debugger_breakpoints);

void DebuggerBreakpoints_Free(DebuggerBreakpoints *debugger_breakpoints);