#pragma once

#include <Emulator.h>

void MemoryViewer_Init(MemoryViewer *memory_viewer,Emulator *emulator);

void MemoryViewer_OpenWindow(MemoryViewer *memory_viewer);

void MemoryViewer_CloseWindow(MemoryViewer *memory_viewer);

DWORD WINAPI MemoryViewer_Main(LPVOID lParam);

void MemoryViewer_Execute(MemoryViewer *memory_viewer);

void MemoryViewer_Exit(MemoryViewer *memory_viewer);

void MemoryViewer_Free(MemoryViewer *memory_viewer);