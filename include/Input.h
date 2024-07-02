#pragma once

#include <Emulator.h>

typedef void (*Input_Callback)(void *userdata,const uint16_t *content,uint64_t length);

typedef struct _Input{
    GapBuffer *gap_buffer;
    WG_FontAtlas *atlas;
    Vector2 glyph_size;
    Rect rect;
    uint64_t max_length;
    bool mouse_over;
    uint32_t flags;
    bool on;
    struct{
        uint32_t time;
        bool visible;
        bool update;
        int index;
    }cursor;
    int offset;
    Input_Callback callback;
    void *userdata;
}Input;

Input* CreateInput(uint32_t format,WG_FontAtlas *atlas,Input_Callback callback,void *userdata);

void UpdateInputCursor(Input *input);

void DrawInput(Input *input,WG_Renderer *renderer,Color background,Color border,Color on,Color off);

void InputEvent(Input *input,WG_Event event,Vector2 mouse_pos);

void DestroyInput(Input *input);