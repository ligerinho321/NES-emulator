#ifndef PPUV_PPUVI_H_
#define PPUV_PPUVI_H_

#include <Emulator.h>

void PPUVI_Init(PPUVI *ppuvi,PPUV *ppuv);

void PPUVI_OpenWindow(PPUVI *ppuvi);

void PPUVI_CloseWindow(PPUVI *ppuvi);

void PPUVI_Event(PPUVI *ppuvi);

void PPUVI_Draw(PPUVI *ppuvi);

DWORD WINAPI PPUVI_Main(LPVOID lParam);

void PPUVI_Execute(PPUVI *ppuvi);

void PPUVI_Exit(PPUVI *ppuvi);

void PPUVI_Free(PPUVI *ppuvi);

#endif