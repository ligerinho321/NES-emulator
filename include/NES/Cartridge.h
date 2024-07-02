#pragma once

#include <Emulator.h>
#include <utils.h>

#define KB 1024

//Header Format
#define iNES 0x01
#define NES20 0x02

//Console type
#define NINTENDO_DEFAULT       0x00
#define NINTENDO_VS_SYSTEM     0x01
#define NINTENDO_PLAYCHOICE_10 0x02
#define EXTENDED_CONSOLE_TYPE  0x03

Cartridge* Cartridge_Load(const uint16_t *fileName);

void Cartridge_Free(Cartridge *cartridge);

void Cartridge_PrintInformations(Cartridge *cartridge);
