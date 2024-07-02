#pragma once

#include <Emulator.h>

void NES_Init(NES *nes,Emulator *emulator);

void NES_ResizeScreen(NES *nes);

void NES_Input(NES *nes);

void NES_Run(NES *nes);

void NES_Reset(NES *nes,uint32_t type);

void NES_LoadPalette(NES *nes);

void NES_InsertCartridge(NES *nes,const uint16_t *fileName);

void NES_Write(NES *nes,uint8_t value,uint16_t address);

uint8_t NES_Read(NES *nes,uint16_t address);

void NES_Free(NES *nes);
