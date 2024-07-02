#pragma once

#include <Emulator.h>
#include <WG/UT/UTarray.h>

typedef struct _BreakpointEdit{
    DebuggerBreakpoints *debugger_breakpoints;

    WG_Window *window_parent;
    WG_Renderer *share_renderer;

    WG_Window *window;
    WG_Renderer *renderer;

    WG_FontAtlas *atlas;
    Vector2 glyph_size;

    uint32_t operation;
    BreakpointView *breakpoint_view;

    uint16_t *buffer;

    struct{
        WG_Texture *arrow;
        WG_Texture *checked;
        WG_Texture *unchecked;
        WG_Texture *help;
    }resources;

    struct{
        Label *label;
        ComboBoxElement *cpu_memory;
        ComboBoxElement *ppu_memory;
        ComboBoxElement *prg_rom;
        ComboBoxElement *system_ram;
        ComboBoxElement *work_ram;
        ComboBoxElement *nametable_ram;
        ComboBoxElement *sprite_ram;
        ComboBoxElement *palette_ram;
        ComboBoxElement *chr_rom;
        ComboBoxElement *chr_ram;
        ComboBox *combo_box;
        uint32_t type;
        uint32_t length;
        Label *max;
    }memory_type;
    
    struct{
        Label *label;
        struct{
            ToggleButton *check_button;
            Label *label;
        }write;
        struct{
            ToggleButton *check_button;
            Label *label;
        }read;
        struct{
            bool enabled;
            ToggleButton *check_button;
            Label *label;
        }execute;
    }break_flags;

    struct{
        Label *label;
        struct{
            Input *input;
            uint32_t value;
        }start;
        Label *separation;
        struct{
            Input *input;
            uint32_t value;
        }end;
        Label *format;
        Label *invalid_address_range;
        bool is_valid;
    }address;

    struct{
        Label *label;
        Input *input;
        IconButton *help;
        Label *invalid_expression;
        bool is_valid;
        UT_array postfix;
    }condition;

    struct{
        ToggleButton *check_button;
        Label *label;
    }enabled;

    Button *ok;
    Button *cancel;

    struct{
        Vector2 window;
        Vector2 desktop;
    }mouse;

    WG_Event event;
    Rect window_size;
    bool open;
}BreakpointEdit;

void BreakpointEdit_Init(BreakpointEdit *breakpoint_edit,DebuggerBreakpoints *debugger_breakpoints);

void BreakpointEdit_OpenWindow(BreakpointEdit *breakpoint_edit,uint32_t operation,BreakpointView *breakpoint_view);

void BreakpointEdit_CloseWindow(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Run(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Free(BreakpointEdit *breakpoint_edit);