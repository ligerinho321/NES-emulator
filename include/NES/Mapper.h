#pragma once

#include <Emulator.h>

typedef enum _MapperID{
    MAPPER_NROM,
    MAPPER_MMC1,
    MAPPER_UXROM,
    MAPPER_CNROM,
    MAPPER_MMC3,
    MAPPER_MMC5,
    MAPPER_006,
    MAPPER_AXROM
}MapperID;

void Mapper_Init(Mapper *mapper,NES *nes);
bool Mapper_InsertCartridge(Mapper *mapper,Cartridge *cartridge);
void Mapper_Free(Mapper *mapper);

void NROM_Init(Mapper *mapper);
void MMC1_Init(Mapper *mapper);
void UXROM_Init(Mapper *mapper);
void CNROM_Init(Mapper *mapper);
void MMC3_Init(Mapper *mapper);
void AXROM_Init(Mapper *mapper);