#ifndef PPUV_PPUVIE_H_
#define PPUV_PPUVIE_H_

#include <Emulator.h>

PPUVIE* PPUVIE_CreateString(uint16_t *variable,uint16_t *string,WG_FontAtlas *atlas);

PPUVIE* PPUVIE_CreateScreen(uint16_t *variable,WG_Renderer *renderer,int src_w,int src_h,int dst_w,int dst_h,PixelFormat f,WG_FontAtlas *atlas);

int PPUVIE_GetLeftWidth(PPUVIE *list);

void PPUVIE_ConfigRect(PPUVIE *list,PPUVI *ppuvi);

void PPUVIE_Draw(PPUVIE *list,WG_Renderer *renderer);

void PPUVIE_Free(PPUVIE *list);

#endif