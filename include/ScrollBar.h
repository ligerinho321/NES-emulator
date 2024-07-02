#pragma once

#include <Emulator.h>

#define SCROLL_BAR_SIZE 10
#define SCROLL_BAR_MIN_SIZE 50

#define SCROLLBAR1_COLOR          (Color){150,150,150,50}
#define SCROLLBAR2_COLOR          (Color){255,255,255,100}
#define SCROLLBAR3_COLOR          (Color){255,255,255,150}

typedef struct _ScrollBar{
    Rect bar_rect;
    RectF scroll_rect;
    float offset;
    int length;
    bool visible;
    bool valid;
    struct{
        float diff;
        bool over;
        bool pressed;
    }mouse;
}ScrollBar;

float GetScrollBarHeight(ScrollBar scroll_bar);
float GetScrollBarWidth(ScrollBar scroll_bar);

void SetVerticalOffsetScrollBar(ScrollBar *scroll_bar);
void SetHorizontalOffsetScrollBar(ScrollBar *scroll_bar);

void AjustVerticalScrollBar(ScrollBar *scroll_bar);
void AjustHorizontalScrollBar(ScrollBar *scroll_bar);

void SetYScrollBar(ScrollBar *scroll_bar,float offset);
void SetXScrollBar(ScrollBar *scroll_bar,float offset);

void HandleVerticalScrollBar(ScrollBar *scroll_bar,Vector2 mouse_pos);
void HandleHorizontalScrollBar(ScrollBar *scroll_bar,Vector2 mouse_pos);

void DrawScrollBar(ScrollBar *scroll_bar,WG_Renderer *renderer);