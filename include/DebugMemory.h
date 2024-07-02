#pragma once

#include <Emulator.h>

#define Debug_CPU_Read8(nes,address) (Debug_CPU_ReadMemory(nes,address))

#define Debug_CPU_Read16(nes,address) ((uint16_t)(Debug_CPU_ReadMemory(nes,address + 1) << 8) | (uint16_t)(Debug_CPU_ReadMemory(nes,address)))

void Debug_CPU_WriteMemory(NES *nes,uint16_t address,uint8_t value);

uint8_t Debug_CPU_ReadMemory(NES *nes,uint16_t address);