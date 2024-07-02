#include <Debugger/Breakpoints/DebuggerBreakpoints.h>
#include <Debugger/Breakpoints/Menu.h>
#include <Debugger/Breakpoints/BreakpointView.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Debugger/Metrics.h>
#include <Menu.h>
#include <ScrollBar.h>
#include <Utils.h>

void DebuggerBreakpoints_Init(DebuggerBreakpoints *debugger_breakpoints){
    
    debugger_breakpoints->expand_button = CreateExpandButton(
        L"Breakpoints",
        debugger_breakpoints->debugger->resources.arrow,
        debugger_breakpoints->debugger->atlas
    );

    BreakpointsView_Init(debugger_breakpoints);

    Breakpoints_Menu_Init(debugger_breakpoints);
    
    debugger_breakpoints->list.vertical_scroll_bar = calloc(1,sizeof(ScrollBar));

    debugger_breakpoints->breakpoint_edit = malloc(sizeof(BreakpointEdit));
    BreakpointEdit_Init(debugger_breakpoints->breakpoint_edit,debugger_breakpoints);

    debugger_breakpoints->config_rect = DebuggerBreakpoints_ConfigRect;
}

void DebuggerBreakpoints_ConfigRect(DebuggerBreakpoints *debugger_breakpoints){
    
    debugger_breakpoints->expand_button->rect = (Rect){
        .x = debugger_breakpoints->rect.x,
        .y = debugger_breakpoints->rect.y,
        .w = debugger_breakpoints->rect.w,
        .h = ELEMENT_HEIGHT
    };

    ExpandButtonConfigRect(debugger_breakpoints->expand_button);

    if(debugger_breakpoints->expand_button->open){
        
        BreakpointsView_ConfigRect(debugger_breakpoints);

        debugger_breakpoints->list.rect = (Rect){
            .x = debugger_breakpoints->rect.x,
            .y = debugger_breakpoints->expand_button->rect.y + debugger_breakpoints->expand_button->rect.h,
            .w = debugger_breakpoints->rect.w,
            .h = debugger_breakpoints->rect.h - debugger_breakpoints->expand_button->rect.h
        };

        debugger_breakpoints->list.vertical_scroll_bar->bar_rect = (Rect){
            .x = debugger_breakpoints->list.rect.x + debugger_breakpoints->list.rect.w - SCROLL_BAR_SIZE,
            .y = debugger_breakpoints->list.rect.y,
            .w = SCROLL_BAR_SIZE,
            .h = debugger_breakpoints->list.rect.h
        };

        debugger_breakpoints->list.vertical_scroll_bar->scroll_rect = (RectF){
            .x = debugger_breakpoints->list.vertical_scroll_bar->bar_rect.x,
            .y = debugger_breakpoints->list.vertical_scroll_bar->bar_rect.y,
            .w = SCROLL_BAR_SIZE,
            .h = 0.0f
        };

        BreakpointsView_ConfigVerticalScrollBar(debugger_breakpoints);
    }
}

static void DebuggerBreakpoints_MouseOver(DebuggerBreakpoints *debugger_breakpoints){
    
    Vector2 mouse = debugger_breakpoints->debugger->mouse;

    if(PointInRect(mouse,debugger_breakpoints->expand_button->rect)){
        debugger_breakpoints->expand_button->mouse.over = true;
    }
    else if(debugger_breakpoints->expand_button->open && PointInRect(mouse,debugger_breakpoints->list.rect)){

        if(PointInRect(mouse,debugger_breakpoints->list.vertical_scroll_bar->scroll_rect) && debugger_breakpoints->list.vertical_scroll_bar->valid){
            debugger_breakpoints->list.vertical_scroll_bar->mouse.over = true;
        }
        else{

            mouse.x -= debugger_breakpoints->list.rect.x;
            mouse.y -= debugger_breakpoints->list.rect.y;

            BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
            Rect rect;

            while(breakpoint_view){
                
                rect = breakpoint_view->rect;
                rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

                if(PointInRect(mouse,rect)){

                    rect = breakpoint_view->enabled->rect;
                    rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

                    if(PointInRect(mouse,rect)){
                        breakpoint_view->enabled->mouse.over = true;
                    }

                    breakpoint_view->mouse.over = true;
                    break;
                }

                breakpoint_view = breakpoint_view->next;
            }
        }

        debugger_breakpoints->list.mouse.over = true;
    }
}

static void DebuggerBreakpoints_MouseLeftButtonDown(DebuggerBreakpoints *debugger_breakpoints){

    Vector2 mouse = debugger_breakpoints->debugger->mouse;

    if(debugger_breakpoints->expand_button->mouse.over){

        debugger_breakpoints->expand_button->mouse.pressed = true;
    }
    else if(debugger_breakpoints->list.vertical_scroll_bar->mouse.over){

        debugger_breakpoints->list.vertical_scroll_bar->mouse.pressed = true;
        debugger_breakpoints->list.vertical_scroll_bar->mouse.diff = mouse.y - debugger_breakpoints->list.vertical_scroll_bar->scroll_rect.y;
    }
    else if(debugger_breakpoints->list.mouse.over){

        BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
        Rect rect;

        while(breakpoint_view){
                
            rect = breakpoint_view->enabled->rect;
            rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

            if(breakpoint_view->mouse.over){

                if(breakpoint_view->enabled->mouse.over){
                    breakpoint_view->enabled->mouse.pressed = true;
                }
                else{
                    breakpoint_view->mouse.left_pressed = true;
                }

                break;
            }

            breakpoint_view = breakpoint_view->next;
        }

        debugger_breakpoints->list.mouse.pressed = true;
    }
}

static void DebuggerBreakpoints_MouseRightButtonDown(DebuggerBreakpoints *debugger_breakpoints){
    
    if(debugger_breakpoints->list.mouse.over){

        BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
        Rect rect;

        while(breakpoint_view){
                
            rect = breakpoint_view->rect;
            rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

            if(breakpoint_view->mouse.over){
                breakpoint_view->mouse.right_pressed = true;
                break;
            }

            breakpoint_view = breakpoint_view->next;
        }

        debugger_breakpoints->list.mouse.pressed = true;
    }
}

static void DebuggerBreakpoints_MouseLeftButtonUp(DebuggerBreakpoints *debugger_breakpoints){

    if(debugger_breakpoints->expand_button->mouse.pressed){

        if(debugger_breakpoints->expand_button->mouse.over){
            debugger_breakpoints->expand_button->open = !debugger_breakpoints->expand_button->open;
            ConfigMetricLeft(debugger_breakpoints->debugger);
        }

        debugger_breakpoints->expand_button->mouse.pressed = false;
    }
    else if(debugger_breakpoints->list.vertical_scroll_bar->mouse.pressed){
        
        debugger_breakpoints->list.vertical_scroll_bar->mouse.pressed = false;
    }
    else if(debugger_breakpoints->list.mouse.pressed){

        BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
        Rect rect;
            
        while(breakpoint_view){
                
            rect = breakpoint_view->enabled->rect;
            rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;
                
            if(breakpoint_view->mouse.left_pressed){

                if(breakpoint_view->mouse.over){
                    debugger_breakpoints->list.breakpoints.current = breakpoint_view;
                }

                breakpoint_view->mouse.left_pressed = false;
                break;
            }
            else if(breakpoint_view->enabled->mouse.pressed){
                    
                if(breakpoint_view->enabled->mouse.over){
                    debugger_breakpoints->list.breakpoints.current = breakpoint_view;
                    breakpoint_view->enabled->is_enabled = !breakpoint_view->enabled->is_enabled;
                    breakpoint_view->data->enabled = breakpoint_view->enabled->is_enabled;
                }
                    
                breakpoint_view->enabled->mouse.pressed = false;
                break;
            }
                
            breakpoint_view = breakpoint_view->next;
        }

        debugger_breakpoints->list.mouse.pressed = false;
    }
}

static void DebuggerBreakpoints_MouseRightButtonUp(DebuggerBreakpoints *debugger_breakpoints){
    
    if(debugger_breakpoints->list.mouse.pressed){

        BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
        Rect rect;

        while(breakpoint_view){
                
            rect = breakpoint_view->rect;
            rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

            if(breakpoint_view->mouse.right_pressed){

                breakpoint_view->mouse.right_pressed = false;

                if(breakpoint_view->mouse.over){
                    debugger_breakpoints->list.breakpoints.current = breakpoint_view;
                }
                else{
                    breakpoint_view = NULL;
                }

                break;
            }

            breakpoint_view = breakpoint_view->next;
        }

        if(breakpoint_view || debugger_breakpoints->list.mouse.over){

            Vector2 mouse = debugger_breakpoints->debugger->mouse;
            
            POINT point = {
                .x = mouse.x,
                .y = mouse.y
            };
            
            ClientToScreen(WG_GetWindowHandle(debugger_breakpoints->debugger->window),&point);

            if(debugger_breakpoints->list.breakpoints.current){
                debugger_breakpoints->menu.edit->enabled = true;
                debugger_breakpoints->menu.delete->enabled = true;
            }
            else{
                debugger_breakpoints->menu.edit->enabled = false;
                debugger_breakpoints->menu.delete->enabled = false;
            }
            
            Menu_OpenWindow(debugger_breakpoints->menu.menu,(Vector2){point.x,point.y});
        }

        debugger_breakpoints->list.mouse.pressed = false;
    }
}

void DebuggerBreakpoints_Event(DebuggerBreakpoints *debugger_breakpoints){

    WG_Event event = debugger_breakpoints->debugger->event;

    DebuggerBreakpoints_MouseOver(debugger_breakpoints);

    Menu_ShortcutEvent(debugger_breakpoints->menu.menu,event);
    
    if(event.type == WG_MOUSEWHEEL && debugger_breakpoints->list.mouse.over && debugger_breakpoints->list.vertical_scroll_bar->valid){
        
        if(event.mouse.wheel < 0.0f){
            SetYScrollBar(debugger_breakpoints->list.vertical_scroll_bar,debugger_breakpoints->list.vertical_scroll_bar->offset - (ELEMENT_HEIGHT * 3));
        }
        else if(event.mouse.wheel > 0.0f){
            SetYScrollBar(debugger_breakpoints->list.vertical_scroll_bar,debugger_breakpoints->list.vertical_scroll_bar->offset + (ELEMENT_HEIGHT * 3));
        }
    }
    else if(event.type == WG_MOUSEBUTTONDOWN){

        if(event.mouse.button == WG_MOUSEBUTTON_LEFT){
            DebuggerBreakpoints_MouseLeftButtonDown(debugger_breakpoints);
        }
        else if(event.mouse.button == WG_MOUSEBUTTON_RIGHT){
            DebuggerBreakpoints_MouseRightButtonDown(debugger_breakpoints);
        }
    }
    else if(event.type == WG_MOUSEBUTTONUP){

        if(event.mouse.button == WG_MOUSEBUTTON_LEFT){
            DebuggerBreakpoints_MouseLeftButtonUp(debugger_breakpoints);
        }
        else if(event.mouse.button == WG_MOUSEBUTTON_RIGHT){
            DebuggerBreakpoints_MouseRightButtonUp(debugger_breakpoints);
        }
    }
    else if(WG_GetMouseState() & WG_MOUSEBUTTON_LEFT && debugger_breakpoints->list.vertical_scroll_bar->mouse.pressed){

        HandleVerticalScrollBar(debugger_breakpoints->list.vertical_scroll_bar,debugger_breakpoints->debugger->mouse);
    }

    debugger_breakpoints->list.mouse.over = false;
}

void DebuggerBreakpoints_Draw(DebuggerBreakpoints *debugger_breakpoints){
    WG_Renderer *renderer = debugger_breakpoints->debugger->renderer;
    
    DrawExpandButton(debugger_breakpoints->expand_button,renderer);

    if(debugger_breakpoints->expand_button->open){
        
        WG_RendererSetViewport(renderer,NULL,&debugger_breakpoints->list.rect);
        
        BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
        Rect rect;
        Vector2 position;

        while(breakpoint_view){

            if(breakpoint_view->mouse.over || breakpoint_view == debugger_breakpoints->list.breakpoints.current){

                rect = breakpoint_view->rect;
                rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

                WG_DrawFilledRectangle(renderer,&rect,BACKGROUND_SELECTED_COLOR);

                breakpoint_view->mouse.over = false;
            }

            rect = breakpoint_view->enabled->rect;
            rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

            if(breakpoint_view->enabled->is_enabled){

                if(breakpoint_view->enabled->mouse.over){
                    breakpoint_view->enabled->mouse.over = false;
                    WG_BlitColor(renderer,breakpoint_view->enabled->icon.on,NULL,&rect,ON_COLOR);
                }
                else{
                    WG_BlitColor(renderer,breakpoint_view->enabled->icon.on,NULL,&rect,OFF_COLOR);
                }
            }
            else{
                if(breakpoint_view->enabled->mouse.over){
                    breakpoint_view->enabled->mouse.over = false;
                    WG_BlitColor(renderer,breakpoint_view->enabled->icon.off,NULL,&rect,ON_COLOR);
                }
                else{
                    WG_BlitColor(renderer,breakpoint_view->enabled->icon.off,NULL,&rect,OFF_COLOR);
                }
            }

            position = breakpoint_view->description->position;
            position.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

            WG_DrawText(renderer,breakpoint_view->description->atlas,breakpoint_view->description->text,position,OFF_COLOR);
            
            if(breakpoint_view == debugger_breakpoints->list.breakpoints.current){
                rect = breakpoint_view->rect;
                rect.y += debugger_breakpoints->list.vertical_scroll_bar->offset;

                WG_DrawRectangle(renderer,&rect,BORDER_COLOR);
            }

            breakpoint_view = breakpoint_view->next;
        }
        
        WG_RendererSetViewport(renderer,NULL,NULL);

        DrawScrollBar(debugger_breakpoints->list.vertical_scroll_bar,renderer);
    }
}

void DebuggerBreakpoints_Run(DebuggerBreakpoints *debugger_breakpoints){

    if(!debugger_breakpoints->debugger->metrics.left.open) return;

    DebuggerBreakpoints_Event(debugger_breakpoints);
    
    DebuggerBreakpoints_Draw(debugger_breakpoints);
}

void DebuggerBreakpoints_RunChildrenWindow(DebuggerBreakpoints *debugger_breakpoints){
    if(debugger_breakpoints->menu.menu->open){
        Menu_Run(debugger_breakpoints->menu.menu);
    }
    if(debugger_breakpoints->breakpoint_edit->open){
        BreakpointEdit_Run(debugger_breakpoints->breakpoint_edit);
    }
}

void DebuggerBreakpoints_Free(DebuggerBreakpoints *debugger_breakpoints){

    BreakpointEdit_Free(debugger_breakpoints->breakpoint_edit);

    Menu_Free(debugger_breakpoints->menu.menu);
    
    DestroyExpandButton(debugger_breakpoints->expand_button);

    free(debugger_breakpoints->list.vertical_scroll_bar);

    BreakpointsView_Free(debugger_breakpoints);
}