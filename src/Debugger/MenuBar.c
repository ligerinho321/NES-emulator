#include <Debugger/MenuBar.h>
#include <Utils.h>

void DebuggerMenuBar_Init(DebuggerMenuBar *menu_bar){
    
    menu_bar->_continue = CreateIconButton(menu_bar->debugger->resources.play,L"Continue",menu_bar->debugger->atlas);
    menu_bar->_break = CreateIconButton(menu_bar->debugger->resources.pause,L"Break",menu_bar->debugger->atlas);
    menu_bar->step_into = CreateIconButton(menu_bar->debugger->resources.step_into,L"Step into",menu_bar->debugger->atlas);
    menu_bar->step_over = CreateIconButton(menu_bar->debugger->resources.step_over,L"Step over",menu_bar->debugger->atlas);
    menu_bar->step_out = CreateIconButton(menu_bar->debugger->resources.step_out,L"Step out",menu_bar->debugger->atlas);

    menu_bar->config_rect = DebuggerMenuBar_ConfigRect;
}

void DebuggerMenuBar_ConfigRect(DebuggerMenuBar *menu_bar){
    menu_bar->rect = menu_bar->debugger->metrics.top.rect;
    int y = menu_bar->rect.y + menu_bar->rect.h * 0.5f - ICON_SIZE * 0.5f;

    menu_bar->_continue->button.rect = (Rect){
        .x = menu_bar->rect.x,
        .y = y,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    menu_bar->_break->button.rect = (Rect){
        .x = menu_bar->_continue->button.rect.x + menu_bar->_continue->button.rect.w,
        .y = y,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    menu_bar->step_into->button.rect = (Rect){
        .x = menu_bar->_break->button.rect.x + menu_bar->_break->button.rect.w,
        .y = y,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    menu_bar->step_over->button.rect = (Rect){
        .x = menu_bar->step_into->button.rect.x + menu_bar->step_into->button.rect.w,
        .y = y,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    menu_bar->step_out->button.rect = (Rect){
        .x = menu_bar->step_over->button.rect.x + menu_bar->step_over->button.rect.w,
        .y = y,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };
}

static void DebuggerMenuBar_MouseOver(DebuggerMenuBar *menu_bar){
    Vector2 mouse = menu_bar->debugger->mouse;

    if(!PointInRect(mouse,menu_bar->rect)) return;

    if(PointInRect(mouse,menu_bar->_continue->button.rect)){
        if(menu_bar->debugger->nes->paused){
            menu_bar->_continue->button.mouse.over = true;
        }
    }
    else if(PointInRect(mouse,menu_bar->_break->button.rect)){
        if(!menu_bar->debugger->nes->paused){
            menu_bar->_break->button.mouse.over = true;
        }
    }
    else if(PointInRect(mouse,menu_bar->step_into->button.rect)){
        menu_bar->step_into->button.mouse.over = true;
    }
    else if(PointInRect(mouse,menu_bar->step_over->button.rect)){
        menu_bar->step_over->button.mouse.over = true;
    }
    else if(PointInRect(mouse,menu_bar->step_out->button.rect)){
        menu_bar->step_out->button.mouse.over = true;
    }
}

static void DebuggerMenuBar_MouseButtonDown(DebuggerMenuBar *menu_bar){

    if(menu_bar->_continue->button.mouse.over){
        menu_bar->_continue->button.mouse.pressed = true;
    }
    else if(menu_bar->_break->button.mouse.over){
        menu_bar->_break->button.mouse.pressed = true;
    }
    else if(menu_bar->step_into->button.mouse.over){
        menu_bar->step_into->button.mouse.pressed = true;
    }
    else if(menu_bar->step_over->button.mouse.over){
        menu_bar->step_over->button.mouse.pressed = true;
    }
    else if(menu_bar->step_out->button.mouse.over){
        menu_bar->step_out->button.mouse.pressed = true;
    }
}

static void DebuggerMenuBar_MouseButtonUp(DebuggerMenuBar *menu_bar){

    if(menu_bar->_continue->button.mouse.pressed){

        if(menu_bar->_continue->button.mouse.over){
            menu_bar->debugger->nes->toggle_pause(menu_bar->debugger->nes);
        }

        menu_bar->_continue->button.mouse.pressed = false;
    }
    else if(menu_bar->_break->button.mouse.pressed){

        if(menu_bar->_break->button.mouse.over){
            menu_bar->debugger->nes->toggle_pause(menu_bar->debugger->nes);
        }

        menu_bar->_break->button.mouse.pressed = false;
    }
    else if(menu_bar->step_into->button.mouse.pressed){

        if(menu_bar->step_into->button.mouse.over){
            menu_bar->debugger->nes->step_into(menu_bar->debugger->nes);
        }

        menu_bar->step_into->button.mouse.pressed = false;
    }
    else if(menu_bar->step_over->button.mouse.pressed){

        if(menu_bar->step_over->button.mouse.over){

        }

        menu_bar->step_over->button.mouse.pressed = false;
    }
    else if(menu_bar->step_out->button.mouse.pressed){

        if(menu_bar->step_out->button.mouse.over){

        }

        menu_bar->step_out->button.mouse.pressed = false;
    }
}

static void DebuggerMenuBar_Event(DebuggerMenuBar *menu_bar){
    WG_Event event = menu_bar->debugger->event;

    DebuggerMenuBar_MouseOver(menu_bar);

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        DebuggerMenuBar_MouseButtonDown(menu_bar);
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        DebuggerMenuBar_MouseButtonUp(menu_bar);
    }
}

static void DebuggerMenuBar_Draw(DebuggerMenuBar *menu_bar){
    WG_Renderer *renderer = menu_bar->debugger->renderer;
    
    DrawIconButton(menu_bar->_continue,renderer);

    DrawIconButton(menu_bar->_break,renderer);

    DrawIconButton(menu_bar->step_into,renderer);

    DrawIconButton(menu_bar->step_over,renderer);

    DrawIconButton(menu_bar->step_out,renderer);
}

void DebuggerMenuBar_DrawPopUps(DebuggerMenuBar *menu_bar){
    
    WG_Renderer *renderer = menu_bar->debugger->renderer;

    Vector2 cursor = {
        .x = menu_bar->debugger->mouse.x + menu_bar->debugger->mouse_hotspot.x,
        .y = menu_bar->debugger->mouse.y + menu_bar->debugger->mouse_hotspot.y
    };

    DrawIconButtonTooltip(menu_bar->_continue,renderer,cursor);

    DrawIconButtonTooltip(menu_bar->_break,renderer,cursor);

    DrawIconButtonTooltip(menu_bar->step_into,renderer,cursor);

    DrawIconButtonTooltip(menu_bar->step_over,renderer,cursor);

    DrawIconButtonTooltip(menu_bar->step_out,renderer,cursor);
}

void DebuggerMenuBar_Run(DebuggerMenuBar *menu_bar){
    DebuggerMenuBar_Event(menu_bar);
    DebuggerMenuBar_Draw(menu_bar);
}

void DebuggerMenuBar_Free(DebuggerMenuBar *menu_bar){
    DestroyIconButton(menu_bar->_continue);
    DestroyIconButton(menu_bar->_break);
    DestroyIconButton(menu_bar->step_into);
    DestroyIconButton(menu_bar->step_over);
    DestroyIconButton(menu_bar->step_out);
}