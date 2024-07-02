#pragma once

#include <Emulator.h>

void Debugger_Init(Debugger *debugger,Emulator *emulator);

void Debugger_OpenWindow(Debugger *debugger);

void Debugger_CloseWindow(Debugger *debugger);

void Debugger_Event(Debugger *debugger);

DWORD WINAPI Debugger_Main(LPVOID lParam);

void Debugger_Execute(Debugger *debugger);

void Debugger_Exit(Debugger *debugger);

void Debugger_Free(Debugger *debugger);