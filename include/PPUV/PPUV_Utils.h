#ifndef PPUV_PPUVUTILS_H_
#define PPUV_PPUVUTILS_H_

#include <Emulator.h>

void UpdatePatternTable(NES *nes,uint8_t *pixels,int32_t pitch,uint8_t *ptr);

void UpdatePatternTables(PPUV *ppu_viewer);


void UpdateNameTable(NES *nes,uint8_t *pixels,int32_t pitch,uint16_t base_address);

void UpdateNameTables(PPUV *ppu_viewer);


void UpdateOAMTable(PPUV *ppu_viewer);


void UpdatePaletteTable(PPUV *ppuv);


#endif