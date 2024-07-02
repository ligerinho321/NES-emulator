#ifndef PPUV_PPUV_H_
#define PPUV_PPUV_H_

#include <Emulator.h>

void PPUV_Init(PPUV *ppuv,Emulator *emulator);

void PPUV_OpenWindow(PPUV *ppuv);

void PPUV_CloseWindow(PPUV *ppuv);

void PPUV_ConfigRect(PPUV *ppuv);

void PPUV_Event(PPUV *ppuv);

void PPUV_Draw(PPUV *ppuv);

DWORD WINAPI PPUV_Main(LPVOID lParam);

void PPUV_Execute(PPUV *ppuv);

void PPUV_Exit(PPUV *ppuv);

void PPUV_Free(PPUV *ppuv);

#endif