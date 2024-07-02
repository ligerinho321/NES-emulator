#pragma once

#include <Emulator.h>

void CallStack_Init(DebuggerCallStack *callstack);

void CallStack_ConfigRect(DebuggerCallStack *callstack);

void CallStack_Event(DebuggerCallStack *callstack);

void CallStack_Draw(DebuggerCallStack *callstack);

void CallStack_Run(DebuggerCallStack *callstack);

void CallStack_Free(DebuggerCallStack *callstack);