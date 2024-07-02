#pragma once

#include <Emulator.h>
#include <ComboBox/ComboBoxMenu.h>

typedef void (*ComboBox_Callback)(void *data);

typedef struct _ComboBoxElement{
    struct{
        wchar_t *string;
        Vector2 size;
        Vector2 position;
    }name;
    struct{
        bool over;
        bool pressed;
    }mouse;
    bool show;
    Rect rect;
    ComboBox_Callback callback;
    void *data;
    struct _ComboBoxElement *next;
}ComboBoxElement;

typedef struct _ComboBox{
    struct{
        struct{
            Vector2 size;
            Vector2 position;
            Rect rect;
        }name;
        struct{
            bool over;
            bool pressed;
        }mouse;
        struct{
            WG_Texture *arrow;
            Rect rect;
        }icon;
        Rect rect;
    }expand_button;
    ComboBoxElement *list;
    ComboBoxElement *cbe;
    ComboBoxMenu *menu;
    WG_FontAtlas *atlas;
    Vector2 glyph_size;
    Vector2 size;
}ComboBox;

ComboBox* ComboBox_Create(WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer,WG_Texture *arrow,WG_FontAtlas *atlas);

void ComboBox_SetCurrentElement(ComboBox *combo_box,ComboBoxElement *cbe);

void ComboBox_SetDefaultElement(ComboBox *combo_box);

ComboBoxElement* ComboBox_CreateString(const wchar_t *string,ComboBox_Callback callback,void *param);

void ComboBox_InsertString(ComboBox *combo_box,ComboBoxElement *cbe);

void ComboBox_SetSize(ComboBox *combo_box);

void ComboBox_ConfigRect(ComboBox *combo_box,Vector2 position);

void ComboBox_Draw(ComboBox *combo_box,WG_Renderer *renderer);

void ComboBox_Event(ComboBox *combo_box,WG_Event event,Vector2 mouse);

void ComboBox_Free(ComboBox *combo_box);