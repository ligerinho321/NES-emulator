#include <Debugger/Metrics.h>

void ConfigMetricTop(Debugger *debugger){
    DebuggerMenuBar *menu_bar = &debugger->menu_bar;
    menu_bar->config_rect(menu_bar);
}

void ConfigMetricLeft(Debugger *debugger){
    
    DebuggerBreakpoints *breakpoints = &debugger->breakpoints;
    DebuggerLabels *labels = &debugger->labels;
    DebuggerCallStack *callstack = &debugger->callstack;

    Rect rect = debugger->metrics.left.rect;

    int open_counter = 0;
    if(breakpoints->expand_button->open) ++open_counter;
    if(labels->expand_button->open)      ++open_counter;
    if(callstack->expand_button->open)   ++open_counter;

    int element_height = 0;

    if(open_counter > 0){
        element_height = (rect.h - (ELEMENT_HEIGHT * (3 - open_counter))) / open_counter;
    }

    breakpoints->rect = (Rect){
        .x = rect.x,
        .y = rect.y,
        .w = rect.w,
        .h = (breakpoints->expand_button->open) ? element_height : ELEMENT_HEIGHT
    };

    breakpoints->config_rect(breakpoints);


    labels->rect = (Rect){
        .x = rect.x,
        .y = breakpoints->rect.y + breakpoints->rect.h,
        .w = rect.w,
        .h = (labels->expand_button->open) ? element_height : ELEMENT_HEIGHT
    };

    labels->config_rect(labels);


    callstack->rect = (Rect){
        .x = rect.x,
        .y = labels->rect.y + labels->rect.h,
        .w = rect.w,
        .h = (callstack->expand_button->open) ? element_height : ELEMENT_HEIGHT
    };

    callstack->config_rect(callstack);
}

void ConfigMetricCenter(Debugger *debugger){
    DebuggerDisassembly *disassembly = &debugger->disassembly;
    disassembly->config_rect(disassembly);
}

void ConfigMetricRight(Debugger *debugger){
    
    DebuggerStatus *status = &debugger->status;

    status->config_rect(status);
}

void ConfigMetrics(Debugger *debugger){

    if(debugger->metrics.left.resize.mouse.pressed){

        if(debugger->metrics.left.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f < LEFT_MIN_WIDTH * 0.5f){
            
            debugger->metrics.left.resize.rect.x = -(RESIZE_HANDLE_WIDTH * 0.5f);
            debugger->metrics.left.open = false;
        }
        else if(debugger->metrics.left.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f < LEFT_MIN_WIDTH){
            debugger->metrics.left.resize.rect.x = LEFT_MIN_WIDTH - RESIZE_HANDLE_WIDTH * 0.5f;
            debugger->metrics.left.open = true;
        }
        else{
            debugger->metrics.left.open = true;
        }
    }

    debugger->metrics.left.resize.rect.y = debugger->metrics.left.rect.y;
    debugger->metrics.left.resize.rect.w = RESIZE_HANDLE_WIDTH;
    debugger->metrics.left.resize.rect.h = debugger->metrics.left.rect.h;
    
    if(debugger->metrics.right.resize.mouse.pressed){

        if(debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f > debugger->window_size.x - RIGHT_MIN_WIDTH * 0.5f){
            debugger->metrics.right.resize.rect.x = debugger->window_size.x - RESIZE_HANDLE_WIDTH * 0.5f;
            debugger->metrics.right.open = false;
        }
        else if(debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f > debugger->window_size.x - RIGHT_MIN_WIDTH){
            debugger->metrics.right.resize.rect.x = debugger->window_size.x - RIGHT_MIN_WIDTH - RESIZE_HANDLE_WIDTH * 0.5f;
            debugger->metrics.right.open = true;
        }
        else{
            debugger->metrics.right.open = true;
        }
    }
    else{
        if(debugger->metrics.right.open){
            
            if(debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f > debugger->window_size.x - RIGHT_MIN_WIDTH){
                debugger->metrics.right.resize.rect.x = debugger->window_size.x - RIGHT_MIN_WIDTH - RESIZE_HANDLE_WIDTH * 0.5f;
            }
        }
        else{
            debugger->metrics.right.resize.rect.x = debugger->window_size.x - RESIZE_HANDLE_WIDTH * 0.5f;
        }
    }

    debugger->metrics.right.resize.rect.y = debugger->metrics.right.rect.y;
    debugger->metrics.right.resize.rect.w = RESIZE_HANDLE_WIDTH;
    debugger->metrics.right.resize.rect.h = debugger->metrics.right.rect.h;


    int left_width = 0;
    int right_width = 0;
    int center_width = debugger->window_size.x;

    if(debugger->metrics.left.open){
        left_width = debugger->metrics.left.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f;
        center_width -= left_width;
    }

    if(debugger->metrics.right.open){
        right_width = debugger->window_size.x - (debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f);
        center_width -= right_width;
    }

    if(center_width < CENTER_MIN_WIDTH){
        
        center_width = CENTER_MIN_WIDTH;

        if(debugger->metrics.right.open && debugger->metrics.right.resize.mouse.pressed){
            
            int x = left_width + center_width - RESIZE_HANDLE_WIDTH * 0.5f;
            debugger->metrics.right.resize.rect.x = x;
            
            right_width = debugger->window_size.x - (debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f);
        }
        else{
            int x = debugger->window_size.x - center_width - right_width - RESIZE_HANDLE_WIDTH * 0.5f;
            debugger->metrics.left.resize.rect.x = x;

            left_width = debugger->metrics.left.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f;
        }
    }


    debugger->metrics.top.rect = (Rect){
        .x = 0,
        .y = 0,
        .w = debugger->window_size.x,
        .h = debugger->glyph_size.y + 10
    };

    debugger->metrics.top.border[0] = (Vector2){
        .x = debugger->metrics.top.rect.x,
        .y = debugger->metrics.top.rect.y + debugger->metrics.top.rect.h
    };

    debugger->metrics.top.border[1] = (Vector2){
        .x = debugger->metrics.top.rect.x + debugger->metrics.top.rect.w,
        .y = debugger->metrics.top.rect.y + debugger->metrics.top.rect.h
    };


    debugger->metrics.left.rect = (Rect){
        .x = 0,
        .y = debugger->metrics.top.rect.y + debugger->metrics.top.rect.h,
        .w = left_width,
        .h = debugger->window_size.y - debugger->metrics.top.rect.h
    };

    debugger->metrics.left.border[0] = (Vector2){
        .x = debugger->metrics.left.rect.x + debugger->metrics.left.rect.w,
        .y = debugger->metrics.left.rect.y
    };

    debugger->metrics.left.border[1] = (Vector2){
        .x = debugger->metrics.left.rect.x + debugger->metrics.left.rect.w,
        .y = debugger->metrics.left.rect.y + debugger->metrics.left.rect.h
    };


    debugger->metrics.right.rect = (Rect){
        .x = debugger->metrics.right.resize.rect.x + RESIZE_HANDLE_WIDTH * 0.5f,
        .y = debugger->metrics.top.rect.y + debugger->metrics.top.rect.h,
        .w = right_width,
        .h = debugger->window_size.y - debugger->metrics.top.rect.h
    };

    debugger->metrics.right.border[0] = (Vector2){
        .x = debugger->metrics.right.rect.x,
        .y = debugger->metrics.right.rect.y,
    };

    debugger->metrics.right.border[1] = (Vector2){
        .x = debugger->metrics.right.rect.x,
        .y = debugger->metrics.right.rect.y + debugger->metrics.right.rect.h
    };


    debugger->metrics.center.rect = (Rect){
        .x = (debugger->metrics.left.open) ? debugger->metrics.left.rect.x + debugger->metrics.left.rect.w : 0,
        .y = debugger->metrics.top.rect.y + debugger->metrics.top.rect.h,
        .w = center_width,
        .h = debugger->window_size.y - debugger->metrics.top.rect.h
    };

    ConfigMetricTop(debugger);

    ConfigMetricLeft(debugger);

    ConfigMetricCenter(debugger);

    ConfigMetricRight(debugger);
}