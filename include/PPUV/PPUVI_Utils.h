#ifndef PPUV_PPUVI_UTILS_H_
#define PPUV_PPUVI_UTILS_H_

#include <Emulator.h>

void PPUVI_ShowWindow(PPUVI *ppuvi);

void PPUVI_HideWindow(PPUVI *ppuvi);


void InitNameTableTileInfo(PPUVI *ppuvi);

void UpdateNameTableTileInfo(PPUVI *ppuvi,uint16_t base_address,uint8_t column,uint8_t row);

void DrawNameTableTileInfo(PPUVI *ppuvi);


void InitPatternTableTileInfo(PPUVI *ppuvi);

void UpdatePatternTableTileInfo(PPUVI *ppuvi,uint16_t base_address,uint8_t column,uint8_t row);

void DrawPatternTableTileInfo(PPUVI *ppuvi);


void InitOAMSpriteInfo(PPUVI *ppuvi);

void UpdateOAMSpriteInfo(PPUVI *ppuvi,int column,int row);

void DrawOAMSpriteInfo(PPUVI *ppuvi);


void InitPaletteInfo(PPUVI *ppuvi);

void UpdatePaletteInfo(PPUVI *ppuvi,int column,int row);

void DrawPaletteInfo(PPUVI *ppuvi);

#endif