#pragma once

#include <Emulator.h>

typedef struct _ComboBox        ComboBox;
typedef struct _ComboBoxElement ComboBoxElement;

typedef struct _ComboBoxMenu{
    ComboBox *combo_box;
    
    ComboBoxElement *selected_cbe;
    
    WG_Window *window_parent;
    WG_Renderer *share_renderer;

    WG_Window *window;
    WG_Renderer *renderer;

    WG_FontAtlas *atlas;

    WG_Event event;
    Vector2 window_size;
    Vector2 glyph_size;
    struct{
        Vector2 window;
        Vector2 desktop;
    }mouse;
    bool open;
}ComboBoxMenu;

void ComboBoxMenu_Init(ComboBoxMenu *combo_box_menu,ComboBox *combo_box,WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer);

void ComboBoxMenu_OpenWindow(ComboBoxMenu *combo_box_menu);

void ComboBoxMenu_CloseWindow(ComboBoxMenu *combo_box_menu);

void ComboBoxMenu_Run(ComboBoxMenu *combo_box_menu);

void ComboBoxMenu_Free(ComboBoxMenu *combo_box_menu);
