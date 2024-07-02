#pragma once

#include <Emulator.h>

#define SCANLINE_CICLES 341
#define SCANLINES 262

#define RENDERING_ENABLED 0x18

//PPU register V and T flags

//000.0000.0001.1111
#define COARSE_X 0x001F

//000.0011.1110.0000
#define COARSE_Y 0x03E0

//000.0100.0000.0000
#define HORIZONTAL_NAMETABLE 0x400

//000.1000.0000.0000
#define VERTICAL_NAMETABLE 0x800

//111.0000.0000.0000
#define FINE_Y 0x7000

//000.0100.0001.1111
#define HORIZONTAL_BITS 0x041F

//111.1011.1110.0000
#define VERTICAL_BITS 0x7BE0


//PPUCTRL FLAGS
#define BASE_NAMETABLE_ADDRESS      (BIT_0 | BIT_1)
#define INCREMENT_MODE              BIT_2
#define SPRITE_TABLE_ADDRESS        BIT_3
#define BACKGROUND_TABLE_ADDRESS    BIT_4
#define SPRITE_SIZE                 BIT_5
#define MASTER_SLAVE_MODE           BIT_6
#define GENERATE_NMI                BIT_7

//PPUMASK FLAGS
#define GRAYSCALE                         BIT_0
#define SHOW_BACKGROUND_LEFTMOST_8_PIXELS BIT_1
#define SHOW_SPRITES_LEFTMOST_8_PIXELS    BIT_2
#define SHOW_BACKGROUND                   BIT_3
#define SHOW_SPRITES                      BIT_4
#define EMPHASIZE_RED                     BIT_5
#define EMPHASIZE_GREEN                   BIT_6
#define EMPHASIZE_BLUE                    BIT_7

//PPUSTATUS FLAGS
#define SPRITE_OVERFLOW BIT_5
#define SPRITE_0_HIT    BIT_6
#define VERTICAL_BLANK  BIT_7

#define OAM_ZERO BIT_2

void PPU_Init(PPU *ppu,NES *nes);

void PPU_Reset(PPU *ppu,uint8_t type);

void PPU_Execute(PPU *ppu,uint32_t cicles);


void PPU_Write(PPU *ppu,uint8_t value,uint16_t address);

uint8_t PPU_Read(PPU *ppu,uint16_t address);

//PPUCTRL $2000
void PPU_WriteControl(PPU *ppu,uint8_t value);

uint8_t PPU_ReadControl(PPU *ppu);

//PPUMASK $2001
void PPU_WriteMask(PPU *ppu,uint8_t value);

uint8_t PPU_ReadMask(PPU *ppu);

//PPUSTATUS $2002
void PPU_WriteStatus(PPU *ppu,uint8_t value);

uint8_t PPU_ReadStatus(PPU *ppu);

//OAMADDR $2003
void PPU_WriteOAMAddress(PPU *ppu,uint8_t value);

uint8_t PPU_ReadOAMAddress(PPU *ppu);

//OAMDATA $2004
void PPU_WriteOAMData(PPU *ppu,uint8_t value);

uint8_t PPU_ReadOAMData(PPU *ppu);

//PPUSCROLL $2005
void PPU_WriteScroll(PPU *ppu,uint8_t value);

uint8_t PPU_ReadScroll(PPU *ppu);

//PPUADDR $2006
void PPU_WriteAddress(PPU *ppu,uint8_t value);

uint8_t PPU_ReadAddress(PPU *ppu);

//PPUDATA $2007
void PPU_WriteData(PPU *ppu,uint8_t value);

uint8_t PPU_ReadData(PPU *ppu);