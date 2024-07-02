#pragma once

#include <Emulator.h>

void Memory_AjustVerticalScrollBarOffsetByAddress(MemoryViewer *memory_viewer);

void Memory_UpdateInputCursor(MemoryViewer *memory_viewer);

void Memory_WriteInput(MemoryViewer *memory_viewer);

void Memory_Input(MemoryViewer *memory_viewer,uint16_t character);

void Memory_SetInputAddress(MemoryViewer *memory_viewer,uint32_t address);

uint16_t* Memory_GetSelectionContent(MemoryViewer *memory_viewer);

void Memory_SetContent(MemoryViewer *memory_viewer,uint16_t *content);

void Memory_ConfigRect(MemoryViewer *memory_viewer);

void Memory_SetVisibleLines(MemoryViewer *memory_viewer);

void Memory_Draw(MemoryViewer *memory_viewer);

void Memory_SetAddress(void *userdata,const uint16_t *content,uint64_t length);

uint32_t Memory_GetPos(MemoryViewer *memory_viewer);

void Memory_DraggingMouse(MemoryViewer *memory_viewer);