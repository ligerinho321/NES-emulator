#include <Debugger/Breakpoints/BreakpointView.h>
#include <ScrollBar.h>
#include <Utils.h>

void BreakpointsView_Init(DebuggerBreakpoints *debugger_breakpoints){
    if(!debugger_breakpoints->list.breakpoints.data) return;

    BreakpointData *breakpoint_data = debugger_breakpoints->list.breakpoints.data;
    BreakpointView *buffer = NULL;

    while(breakpoint_data){

        BreakpointView *breakpoint_view = calloc(1,sizeof(BreakpointView));
        
        breakpoint_view->data = breakpoint_data;
        
        breakpoint_view->enabled = CreateToggleButton(debugger_breakpoints->debugger->resources.checked,debugger_breakpoints->debugger->resources.unchecked);

        breakpoint_view->description = CreateLabel(L"\0",debugger_breakpoints->debugger->atlas);
        
        BreakpointView_Update(debugger_breakpoints,breakpoint_view);

        if(buffer){
            buffer->next = breakpoint_view;
        }
        else{
            debugger_breakpoints->list.breakpoints.view = breakpoint_view;
        }

        buffer = breakpoint_view;

        breakpoint_data = breakpoint_data->next;
    }
}

BreakpointView* BreakpointsView_Insert(DebuggerBreakpoints *debugger_breakpoints){

    BreakpointView *breakpoint_view = calloc(1,sizeof(BreakpointView));

    breakpoint_view->data = BreakpointsData_Insert(debugger_breakpoints);

    breakpoint_view->enabled = CreateToggleButton(debugger_breakpoints->debugger->resources.checked,debugger_breakpoints->debugger->resources.unchecked);
    
    breakpoint_view->description = CreateLabel(L"\0",debugger_breakpoints->debugger->atlas);

    if(debugger_breakpoints->list.breakpoints.view){
        BreakpointView *buffer = debugger_breakpoints->list.breakpoints.view;
        while(buffer->next){buffer = buffer->next;}
        buffer->next = breakpoint_view;
    }
    else{
        debugger_breakpoints->list.breakpoints.view = breakpoint_view;
    }

    debugger_breakpoints->list.breakpoints.current = breakpoint_view;
    
    BreakpointsView_ConfigRect(debugger_breakpoints);
    BreakpointsView_ConfigVerticalScrollBar(debugger_breakpoints);

    return breakpoint_view;
}

void BreakpointsView_Remove(DebuggerBreakpoints *debugger_breakpoints){
    if(!debugger_breakpoints->list.breakpoints.view || !debugger_breakpoints->list.breakpoints.current) return;

    if(debugger_breakpoints->list.breakpoints.current == debugger_breakpoints->list.breakpoints.view){
        debugger_breakpoints->list.breakpoints.view = debugger_breakpoints->list.breakpoints.current->next;
    }
    else{
        BreakpointView *buffer = debugger_breakpoints->list.breakpoints.view;
        while(buffer && buffer->next != debugger_breakpoints->list.breakpoints.current){buffer = buffer->next;}
        if(buffer){
            buffer->next = debugger_breakpoints->list.breakpoints.current->next;
        }
        else{
            printf("%s: breakpoint not found\n");
        }
    }

    BreakpointView_Destroy(debugger_breakpoints,debugger_breakpoints->list.breakpoints.current,true);
    debugger_breakpoints->list.breakpoints.current = NULL;

    BreakpointsView_ConfigRect(debugger_breakpoints);
    BreakpointsView_ConfigVerticalScrollBar(debugger_breakpoints);
}

void BreakpointsView_ConfigVerticalScrollBar(DebuggerBreakpoints *debugger_breakpoints){
    
    int length = 0;
    BreakpointView *breakpoint = debugger_breakpoints->list.breakpoints.view;
    
    while(breakpoint){
        length += ELEMENT_HEIGHT;
        breakpoint = breakpoint->next;
    }

    debugger_breakpoints->list.vertical_scroll_bar->length = length;
    debugger_breakpoints->list.vertical_scroll_bar->scroll_rect.h = GetScrollBarHeight(*debugger_breakpoints->list.vertical_scroll_bar);

    if(debugger_breakpoints->list.vertical_scroll_bar->scroll_rect.h > 0.0f && debugger_breakpoints->list.vertical_scroll_bar->scroll_rect.h < debugger_breakpoints->list.vertical_scroll_bar->bar_rect.h){
        debugger_breakpoints->list.vertical_scroll_bar->valid = true;
        SetYScrollBar(debugger_breakpoints->list.vertical_scroll_bar,debugger_breakpoints->list.vertical_scroll_bar->offset);
    }
    else{
        debugger_breakpoints->list.vertical_scroll_bar->valid = false;
        debugger_breakpoints->list.vertical_scroll_bar->offset = 0.0f;
        debugger_breakpoints->list.vertical_scroll_bar->scroll_rect.y = debugger_breakpoints->list.vertical_scroll_bar->bar_rect.y;
    }
}

void BreakpointsView_ConfigRect(DebuggerBreakpoints *debugger_breakpoints){
    BreakpointView *breakpoint = debugger_breakpoints->list.breakpoints.view;
    int y = 0;
    while(breakpoint){
        
        breakpoint->rect = (Rect){
            .x = 0,
            .y = y,
            .w = debugger_breakpoints->list.rect.w,
            .h = ELEMENT_HEIGHT
        };

        breakpoint->enabled->rect = (Rect){
            .x = breakpoint->rect.x,
            .y = breakpoint->rect.y,
            .w = ICON_SIZE,
            .h = ICON_SIZE
        };

        breakpoint->description->rect = (Rect){
            .x = breakpoint->enabled->rect.x + breakpoint->enabled->rect.w,
            .y = breakpoint->rect.y,
            .w = breakpoint->rect.w - breakpoint->enabled->rect.w,
            .h = ELEMENT_HEIGHT
        };

        breakpoint->description->position = (Vector2){
            .x = breakpoint->description->rect.x,
            .y = breakpoint->description->rect.y + breakpoint->description->rect.h * 0.5f - breakpoint->description->size.y * 0.5f
        };

        y += ELEMENT_HEIGHT;

        breakpoint = breakpoint->next;
    }
}

void BreakpointView_Update(DebuggerBreakpoints *debugger_breakpoints,BreakpointView *breakpoint_view){
    
    BreakpointData *breakpoint_data = breakpoint_view->data;
    uint16_t *buffer = debugger_breakpoints->debugger->buffer;
    uint32_t length = 0;

    breakpoint_view->enabled->is_enabled = breakpoint_data->enabled;

    switch(breakpoint_data->memory_type){
        case MEMORY_CPU:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"CPU:");
            break;
        case MEMORY_PPU:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"PPU:");
            break;
        case MEMORY_PRG_ROM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"PRG:");
            break;
        case MEMORY_SYSTEM_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"RAM:");
            break;
        case MEMORY_WORK_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"WRAM:");
            break;
        case MEMORY_NAMETABLE_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"NRAM:");
            break;
        case MEMORY_SPRITE_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"OAM:");
            break;
        case MEMORY_PALETTE_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"PRAM:");
            break;
        case MEMORY_CHR_ROM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"CROM:");
            break;
        case MEMORY_CHR_RAM:
            length += swprintf(buffer + length,BUFFER_LENGTH,L"CRAM:");
            break;
    }

    if(breakpoint_data->break_flags & BREAK_WRITE){
        length += swprintf(buffer + length,BUFFER_LENGTH,L"W");
    }
    else{
        length += swprintf(buffer + length,BUFFER_LENGTH,L"-");
    }

    if(breakpoint_data->break_flags & BREAK_READ){
        length += swprintf(buffer + length,BUFFER_LENGTH,L"R");
    }
    else{
        length += swprintf(buffer + length,BUFFER_LENGTH,L"-");
    }

    if(breakpoint_data->break_flags & BREAK_EXECUTE){
        length += swprintf(buffer + length,BUFFER_LENGTH,L"E");
    }
    else{
        length += swprintf(buffer + length,BUFFER_LENGTH,L"-");
    }

    length += swprintf(buffer + length,BUFFER_LENGTH,L" $%.4X-$%.4X ",breakpoint_data->address.start,breakpoint_data->address.end);

    if(breakpoint_data->condition.infix){
        swprintf(buffer + length,BUFFER_LENGTH,L"%ls",breakpoint_data->condition.infix);
    }

    UpdateLabel(breakpoint_view->description,buffer);
}

void BreakpointView_Destroy(DebuggerBreakpoints *debugger_breakpoints,BreakpointView *breakpoint_view,bool destroy_data){
    if(destroy_data) BreakpointsData_Remove(debugger_breakpoints,breakpoint_view->data);
    DestroyToggleButton(breakpoint_view->enabled);
    DestroyLabel(breakpoint_view->description);
    free(breakpoint_view);
}

void BreakpointsView_Free(DebuggerBreakpoints *debugger_breakpoints){
    BreakpointView *breakpoint_view = debugger_breakpoints->list.breakpoints.view;
    BreakpointView *buffer = NULL;
    while(breakpoint_view){
        buffer = breakpoint_view->next;
        BreakpointView_Destroy(debugger_breakpoints,breakpoint_view,false);
        breakpoint_view = buffer;
    }
    debugger_breakpoints->list.breakpoints.view = NULL;
    debugger_breakpoints->list.breakpoints.current = NULL;
}