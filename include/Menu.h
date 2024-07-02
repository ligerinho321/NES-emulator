#pragma once

#include <Emulator.h>

typedef void (*Menu_Callback)(void *userdata);

typedef struct _Shortcut{
    struct{
        uint16_t *string;
        Vector2 size;
        Vector2 position;
    }s;
    WG_KeyMod keymod;
    WG_ScanCode key;
}Shortcut;

typedef struct _MenuOption{
    struct{
        uint16_t *string;
        Vector2 size;
        Vector2 position;
    }s;
    struct{
        bool over;
        bool pressed;
    }m;
    Shortcut *shortcut;
    bool enabled;
    Rect rect;
    Menu_Callback callback;
    void *userdata;
    struct _MenuOption *next;
}MenuOption;

typedef struct _Menu{
    WG_Window *window_parent;
    WG_Renderer *share_renderer;

    WG_Window *window;
    WG_Renderer *renderer;
    
    WG_FontAtlas *atlas;

    MenuOption *options;
    MenuOption *option_selected;
    
    Vector2 window_size;
    Vector2 glyph_size;
    struct{
        Vector2 window;
        Vector2 desktop;
    }mouse;
    Vector2 position;
    WG_Event event;
    bool open;
}Menu;


Shortcut* Menu_CreateShortcut(const uint16_t *text,WG_KeyMod keymod,WG_ScanCode key);

MenuOption* Menu_CreateOption(const uint16_t *name,Shortcut *shortcut,Menu_Callback callback,void *userdata);

void Menu_InsertOption(Menu *menu,MenuOption *option);


void Menu_Init(Menu *menu,WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer,WG_FontAtlas *atlas);

void Menu_OpenWindow(Menu *menu,Vector2 position);

void Menu_CloseWindow(Menu *menu);

void Menu_ShortcutEvent(Menu *menu,WG_Event event);

void Menu_Run(Menu *menu);

void Menu_Free(Menu *menu);