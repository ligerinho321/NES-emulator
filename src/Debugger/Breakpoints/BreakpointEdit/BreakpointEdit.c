#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Debugger/Breakpoints/BreakpointEdit/Metrics.h>
#include <Debugger/Breakpoints/BreakpointEdit/MemoryType.h>
#include <Debugger/Breakpoints/BreakpointEdit/Address.h>
#include <Debugger/Breakpoints/BreakpointEdit/Condition.h>
#include <Debugger/Breakpoints/BreakpointView.h>
#include <ComboBox/ComboBox.h>
#include <Input.h>
#include <Utils.h>
#include <SyntacticAnalyzer.h>
#include <Resources.h>


static void BreakpointEdit_InsertDataIntoBreakpoint(BreakpointEdit *breakpoint_edit){

    BreakpointView *breakpoint_view = NULL;

    if(breakpoint_edit->operation == OPERATION_ADD){
        breakpoint_view = BreakpointsView_Insert(breakpoint_edit->debugger_breakpoints);
    }
    else if(breakpoint_edit->operation == OPERATION_EDIT){
        breakpoint_view = breakpoint_edit->breakpoint_view;
    }

    WaitForSingleObject(breakpoint_edit->debugger_breakpoints->debugger->breakpoint_mutex,INFINITE);
    
    //Enabled
    breakpoint_view->data->enabled = breakpoint_edit->enabled.check_button->is_enabled;

    //Memory Type
    breakpoint_view->data->memory_type = breakpoint_edit->memory_type.type;

    //Break Flags
    breakpoint_view->data->break_flags = 0;
    
    if(breakpoint_edit->break_flags.write.check_button->is_enabled){
        breakpoint_view->data->break_flags |= BREAK_WRITE;
    }

    if(breakpoint_edit->break_flags.read.check_button->is_enabled){
        breakpoint_view->data->break_flags |= BREAK_READ;
    }

    if(breakpoint_edit->break_flags.execute.check_button->is_enabled && breakpoint_edit->break_flags.execute.enabled){
        breakpoint_view->data->break_flags |= BREAK_EXECUTE;
    }

    //Address start/end
    breakpoint_view->data->address.start = breakpoint_edit->address.start.value;
    breakpoint_view->data->address.end = breakpoint_edit->address.end.value;

    //Infix
    free(breakpoint_view->data->condition.infix);
    breakpoint_view->data->condition.infix = NULL;
    GapBufferGetContent(breakpoint_edit->condition.input->gap_buffer,uint16_t,breakpoint_view->data->condition.infix);

    //Postfix
    utarray_clear(&breakpoint_view->data->condition.postfix);
    utarray_concat(&breakpoint_view->data->condition.postfix,&breakpoint_edit->condition.postfix);

    //SyntacticAnalyzer_PrintPostfix(&breakpoint_view->data->condition.postfix);

    ReleaseMutex(breakpoint_edit->debugger_breakpoints->debugger->breakpoint_mutex);

    BreakpointView_Update(breakpoint_edit->debugger_breakpoints,breakpoint_view);
}

static void BreakpointEdit_InsertData(BreakpointEdit *breakpoint_edit){
    
    GapBufferClear(breakpoint_edit->address.start.input->gap_buffer);
    GapBufferClear(breakpoint_edit->address.end.input->gap_buffer);
    GapBufferClear(breakpoint_edit->condition.input->gap_buffer);
    utarray_clear(&breakpoint_edit->condition.postfix);

    if(breakpoint_edit->operation == OPERATION_ADD){
        
        ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.cpu_memory);
        
        breakpoint_edit->break_flags.write.check_button->is_enabled = false;
        breakpoint_edit->break_flags.read.check_button->is_enabled = false;
        breakpoint_edit->break_flags.execute.check_button->is_enabled = false;
                
        breakpoint_edit->address.start.value = 0;

        breakpoint_edit->address.end.value = 0;

        breakpoint_edit->address.is_valid = true;

        breakpoint_edit->condition.is_valid = true;

        breakpoint_edit->enabled.check_button->is_enabled = true;
    }
    else if(breakpoint_edit->operation == OPERATION_EDIT){

        BreakpointView *breakpoint_view = breakpoint_edit->breakpoint_view;

        //Memory type
        switch(breakpoint_view->data->memory_type){
            case MEMORY_CPU:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.cpu_memory);
                break;
            case MEMORY_PPU:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.ppu_memory);
                break;
            case MEMORY_PRG_ROM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.prg_rom);
                break;
            case MEMORY_SYSTEM_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.system_ram);
                break;
            case MEMORY_WORK_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.work_ram);
                break;
            case MEMORY_NAMETABLE_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.nametable_ram);
                break;
            case MEMORY_SPRITE_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.sprite_ram);
                break;
            case MEMORY_PALETTE_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.palette_ram);
                break;
            case MEMORY_CHR_ROM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.chr_rom);
                break;
            case MEMORY_CHR_RAM:
                ComboBox_SetCurrentElement(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.chr_ram);
                break;
        }

        //Break Flags
        breakpoint_edit->break_flags.write.check_button->is_enabled = (breakpoint_view->data->break_flags & BREAK_WRITE) ? true : false;
        breakpoint_edit->break_flags.read.check_button->is_enabled = (breakpoint_view->data->break_flags & BREAK_READ) ? true : false;
        breakpoint_edit->break_flags.execute.check_button->is_enabled = (breakpoint_view->data->break_flags & BREAK_EXECUTE) ? true : false;

        uint32_t length = 0;

        //Address Start
        breakpoint_edit->address.start.value = breakpoint_view->data->address.start;

        if(breakpoint_view->data->address.start){
            length = swprintf(breakpoint_edit->buffer,BUFFER_LENGTH,L"%X",breakpoint_view->data->address.start);
            GapBufferInsertValues(breakpoint_edit->address.start.input->gap_buffer,uint16_t,breakpoint_edit->buffer,length);
        }

        //Address End
        breakpoint_edit->address.end.value = breakpoint_view->data->address.end;

        if(breakpoint_view->data->address.end){
            length = swprintf(breakpoint_edit->buffer,BUFFER_LENGTH,L"%X",breakpoint_view->data->address.end);
            GapBufferInsertValues(breakpoint_edit->address.end.input->gap_buffer,uint16_t,breakpoint_edit->buffer,length);
        }

        BreakpointEdit_Address_Validate(breakpoint_edit);

        //Infix
        if(breakpoint_view->data->condition.infix){
            length = wcslen(breakpoint_view->data->condition.infix);
            GapBufferInsertValues(breakpoint_edit->condition.input->gap_buffer,uint16_t,breakpoint_view->data->condition.infix,length);
        }

        //Postfix
        utarray_concat(&breakpoint_edit->condition.postfix,&breakpoint_view->data->condition.postfix);

        breakpoint_edit->condition.is_valid = SyntacticAnalyzer_InfixToPostfix(breakpoint_edit->condition.input->gap_buffer,&breakpoint_edit->condition.postfix);

        //Enabled        
        breakpoint_edit->enabled.check_button->is_enabled = breakpoint_view->data->enabled;
    }
}


void BreakpointEdit_Init(BreakpointEdit *breakpoint_edit,DebuggerBreakpoints *debugger_breakpoints){

    memset(breakpoint_edit,0,sizeof(BreakpointEdit));
    
    breakpoint_edit->debugger_breakpoints = debugger_breakpoints;

    breakpoint_edit->window_parent = debugger_breakpoints->debugger->window;

    breakpoint_edit->share_renderer = debugger_breakpoints->debugger->renderer;

    breakpoint_edit->buffer = debugger_breakpoints->debugger->buffer;

    breakpoint_edit->window = WG_CreateWindow(
        &debugger_breakpoints->debugger->window_class,
        breakpoint_edit->window_parent,
        L"Breakpoint Edit",
        0,
        0,
        0,
        0,
        WG_WINDOW_HIDDEN | WG_WINDOW_CAPTION,
        0
    );

    WG_EnforceChildWindowFocus(breakpoint_edit->window);

    breakpoint_edit->renderer = WG_CreateRenderer(breakpoint_edit->window,breakpoint_edit->share_renderer);

    breakpoint_edit->atlas = debugger_breakpoints->debugger->atlas;
    breakpoint_edit->glyph_size = debugger_breakpoints->debugger->glyph_size;

    //Resources
    breakpoint_edit->resources.arrow = debugger_breakpoints->debugger->resources.arrow;
    breakpoint_edit->resources.checked = debugger_breakpoints->debugger->resources.checked;
    breakpoint_edit->resources.unchecked = debugger_breakpoints->debugger->resources.unchecked;
    breakpoint_edit->resources.help = debugger_breakpoints->debugger->resources.help;

    //Memory Type
    BreakpointEdit_MemoryType_Init(breakpoint_edit);

    //Break Flags
    breakpoint_edit->break_flags.label = CreateLabel(L"Break Flags:",breakpoint_edit->atlas);

    breakpoint_edit->break_flags.write.check_button = CreateToggleButton(breakpoint_edit->resources.checked,breakpoint_edit->resources.unchecked);
    breakpoint_edit->break_flags.write.label = CreateLabel(L"Write",breakpoint_edit->atlas);

    breakpoint_edit->break_flags.read.check_button = CreateToggleButton(breakpoint_edit->resources.checked,breakpoint_edit->resources.unchecked);
    breakpoint_edit->break_flags.read.label = CreateLabel(L"Read",breakpoint_edit->atlas);

    breakpoint_edit->break_flags.execute.check_button = CreateToggleButton(breakpoint_edit->resources.checked,breakpoint_edit->resources.unchecked);
    breakpoint_edit->break_flags.execute.label = CreateLabel(L"Execute",breakpoint_edit->atlas);

    //Address
    BreakpointEdit_Address_Init(breakpoint_edit);

    //Condition
    BreakpointEdit_Condition_Init(breakpoint_edit);

    //Enabled
    breakpoint_edit->enabled.check_button = CreateToggleButton(breakpoint_edit->resources.checked,breakpoint_edit->resources.unchecked);
    breakpoint_edit->enabled.label = CreateLabel(L"Enabled",breakpoint_edit->atlas);

    //Ok
    breakpoint_edit->ok = CreateButton(L"Ok",breakpoint_edit->atlas);

    //Cancel
    breakpoint_edit->cancel = CreateButton(L"Cancel",breakpoint_edit->atlas);

    breakpoint_edit->open = false;
}

void BreakpointEdit_OpenWindow(BreakpointEdit *breakpoint_edit,uint32_t operation,BreakpointView *breakpoint_view){

    if(breakpoint_edit->open && WG_WindowIsVisible(breakpoint_edit->window)) return;
    
    if(!operation || (operation == OPERATION_EDIT && !breakpoint_view)){
        printf("%s: invalid operation\n",__func__);
        return;
    }

    breakpoint_edit->operation = operation;
    breakpoint_edit->breakpoint_view = breakpoint_view;
    
    BreakpointEdit_InsertData(breakpoint_edit);

    BreakpointEdit_ConfigRect(breakpoint_edit);

    WG_SetWindowPos(
        breakpoint_edit->window,
        HWND_TOP,
        WG_WINDOWPOS_CENTERED,
        WG_WINDOWPOS_CENTERED,
        0,
        0,
        SWP_SHOWWINDOW | SWP_NOSIZE
    );
    
    if(WG_WindowIsVisible(breakpoint_edit->window)){
        breakpoint_edit->open = true;
    }
    else{
        printf("%s: activation failure in window\n",__func__);
        breakpoint_edit->open = false;
    }
}

void BreakpointEdit_CloseWindow(BreakpointEdit *breakpoint_edit){
    if(!breakpoint_edit->open && !WG_WindowIsVisible(breakpoint_edit->window)) return;

    WG_ShowWindow(breakpoint_edit->window,SW_HIDE);

    if(!WG_WindowIsVisible(breakpoint_edit->window)){
        breakpoint_edit->open = false;
        WG_BringWindowToTop(breakpoint_edit->window_parent);
    }
    else{
        printf("%s: window deactivation failed\n",__func__);
        breakpoint_edit->open = true;
    }
}

static void BreakpointEdit_MouseOver(BreakpointEdit *breakpoint_edit){
    
    Vector2 mouse = breakpoint_edit->mouse.window;

    if(PointInRect(mouse,breakpoint_edit->break_flags.write.check_button->rect)){
        breakpoint_edit->break_flags.write.check_button->mouse.over = true;
    }
    else if(PointInRect(mouse,breakpoint_edit->break_flags.read.check_button->rect)){
        breakpoint_edit->break_flags.read.check_button->mouse.over = true;
    }
    else if(PointInRect(mouse,breakpoint_edit->break_flags.execute.check_button->rect) && breakpoint_edit->break_flags.execute.enabled){
        breakpoint_edit->break_flags.execute.check_button->mouse.over = true;
    }
    else if(PointInRect(mouse,breakpoint_edit->enabled.check_button->rect)){
        breakpoint_edit->enabled.check_button->mouse.over = true;
    }
    else if(PointInRect(mouse,breakpoint_edit->ok->rect)){
        breakpoint_edit->ok->mouse.over = true;
    }
    else if(PointInRect(mouse,breakpoint_edit->cancel->rect)){
        breakpoint_edit->cancel->mouse.over = true;
    }
}

static void BreakpointEdit_MouseButtonDown(BreakpointEdit *breakpoint_edit){
    
    if(breakpoint_edit->break_flags.write.check_button->mouse.over){
        breakpoint_edit->break_flags.write.check_button->mouse.pressed = true;
    }
    else if(breakpoint_edit->break_flags.read.check_button->mouse.over){
        breakpoint_edit->break_flags.read.check_button->mouse.pressed = true;
    }
    else if(breakpoint_edit->break_flags.execute.check_button->mouse.over){
        breakpoint_edit->break_flags.execute.check_button->mouse.pressed = true;
    }
    else if(breakpoint_edit->enabled.check_button->mouse.over){
        breakpoint_edit->enabled.check_button->mouse.pressed = true;
    }
    else if(breakpoint_edit->ok->mouse.over){
        breakpoint_edit->ok->mouse.pressed = true;
    }
    else if(breakpoint_edit->cancel->mouse.over){
        breakpoint_edit->cancel->mouse.pressed = true;
    }
}

static void BreakpointEdit_MouseButtonUp(BreakpointEdit *breakpoint_edit){
    
    if(breakpoint_edit->break_flags.write.check_button->mouse.pressed){

        if(breakpoint_edit->break_flags.write.check_button->mouse.over){
            breakpoint_edit->break_flags.write.check_button->is_enabled = !breakpoint_edit->break_flags.write.check_button->is_enabled;
        }

        breakpoint_edit->break_flags.write.check_button->mouse.pressed = false;
    }
    else if(breakpoint_edit->break_flags.read.check_button->mouse.pressed){

        if(breakpoint_edit->break_flags.read.check_button->mouse.over){
            breakpoint_edit->break_flags.read.check_button->is_enabled = !breakpoint_edit->break_flags.read.check_button->is_enabled;
        }

        breakpoint_edit->break_flags.read.check_button->mouse.pressed = false;
    }
    else if(breakpoint_edit->break_flags.execute.check_button->mouse.pressed){

        if(breakpoint_edit->break_flags.execute.check_button->mouse.over){
            breakpoint_edit->break_flags.execute.check_button->is_enabled = !breakpoint_edit->break_flags.execute.check_button->is_enabled;
        }

        breakpoint_edit->break_flags.execute.check_button->mouse.pressed = false;
    }
    else if(breakpoint_edit->enabled.check_button->mouse.pressed){

        if(breakpoint_edit->enabled.check_button->mouse.over){
            breakpoint_edit->enabled.check_button->is_enabled = !breakpoint_edit->enabled.check_button->is_enabled;
        }

        breakpoint_edit->enabled.check_button->mouse.pressed = false;
    }
    else if(breakpoint_edit->ok->mouse.pressed){

        if(breakpoint_edit->ok->mouse.over && breakpoint_edit->address.is_valid && breakpoint_edit->condition.is_valid){
            BreakpointEdit_InsertDataIntoBreakpoint(breakpoint_edit);
            breakpoint_edit->open = false;
        }

        breakpoint_edit->ok->mouse.pressed = false;
    }
    else if(breakpoint_edit->cancel->mouse.pressed){

        if(breakpoint_edit->cancel->mouse.over){

            breakpoint_edit->open = false;
        }

        breakpoint_edit->cancel->mouse.pressed = false;
    }
}

static void BreakpointEdit_Event(BreakpointEdit *breakpoint_edit){
    
    WG_PollEvent(breakpoint_edit->window,&breakpoint_edit->event);

    WG_Event event = breakpoint_edit->event;

    WG_GetMousePosInWindow(breakpoint_edit->window,&breakpoint_edit->mouse.window.x,&breakpoint_edit->mouse.window.y);

    WG_GetMousePos(&breakpoint_edit->mouse.desktop.x,&breakpoint_edit->mouse.desktop.y);

    BreakpointEdit_MouseOver(breakpoint_edit);

    ComboBox_Event(breakpoint_edit->memory_type.combo_box,breakpoint_edit->event,breakpoint_edit->mouse.window);

    InputEvent(breakpoint_edit->address.start.input,event,breakpoint_edit->mouse.window);

    InputEvent(breakpoint_edit->address.end.input,event,breakpoint_edit->mouse.window);

    InputEvent(breakpoint_edit->condition.input,event,breakpoint_edit->mouse.window);
    
    if(event.type == WG_QUIT){
        breakpoint_edit->open = false;
    }
    else if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        BreakpointEdit_MouseButtonDown(breakpoint_edit);
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        BreakpointEdit_MouseButtonUp(breakpoint_edit);
    }
}

static void BreakpointEdit_Draw(BreakpointEdit *breakpoint_edit){
    WG_Renderer *renderer = breakpoint_edit->renderer;

    WG_SetCurrentRenderer(renderer);

    WG_ClearColor(breakpoint_edit->renderer,BACKGROUND1_COLOR);

    //Memory Type
    DrawLabel(renderer,breakpoint_edit->memory_type.label,OFF_COLOR);
    ComboBox_Draw(breakpoint_edit->memory_type.combo_box,renderer);
    DrawLabel(renderer,breakpoint_edit->memory_type.max,OFF_COLOR);

    //Break Flags
    DrawLabel(renderer,breakpoint_edit->break_flags.label,OFF_COLOR);

    //Write
    DrawToggleButton(breakpoint_edit->break_flags.write.check_button,renderer);
    DrawLabel(renderer,breakpoint_edit->break_flags.write.label,OFF_COLOR);

    //Read
    DrawToggleButton(breakpoint_edit->break_flags.read.check_button,renderer);
    DrawLabel(renderer,breakpoint_edit->break_flags.read.label,OFF_COLOR);

    //Execute
    if(breakpoint_edit->break_flags.execute.enabled){
        DrawToggleButton(breakpoint_edit->break_flags.execute.check_button,renderer);
        DrawLabel(renderer,breakpoint_edit->break_flags.execute.label,OFF_COLOR);
    }

    //Address
    BreakpointEdit_Address_Draw(breakpoint_edit);

    //Condition
    BreakpointEdit_Condition_Draw(breakpoint_edit);

    //Enabled
    DrawToggleButton(breakpoint_edit->enabled.check_button,renderer);
    DrawLabel(renderer,breakpoint_edit->enabled.label,OFF_COLOR);

    //Ok
    DrawButton(breakpoint_edit->ok,renderer);

    //Cancel
    DrawButton(breakpoint_edit->cancel,renderer);

    WG_Flip(breakpoint_edit->renderer);
}

static void BreakpointEdit_RunChildrenWindow(BreakpointEdit *breakpoint_edit){
    if(breakpoint_edit->memory_type.combo_box->menu->open){
        ComboBoxMenu_Run(breakpoint_edit->memory_type.combo_box->menu);
    }
}

void BreakpointEdit_Run(BreakpointEdit *breakpoint_edit){
    if(!breakpoint_edit->open) return;

    BreakpointEdit_Event(breakpoint_edit);
    BreakpointEdit_Draw(breakpoint_edit);

    if(breakpoint_edit->open){
        BreakpointEdit_RunChildrenWindow(breakpoint_edit);
    }
    else{
        ComboBoxMenu_CloseWindow(breakpoint_edit->memory_type.combo_box->menu);

        BreakpointEdit_CloseWindow(breakpoint_edit);
    }
}

void BreakpointEdit_Free(BreakpointEdit *breakpoint_edit){
    
    //Memory Type
    BreakpointEdit_MemoryType_Free(breakpoint_edit);

    //Break Flags
    DestroyLabel(breakpoint_edit->break_flags.label);
    DestroyToggleButton(breakpoint_edit->break_flags.write.check_button);
    DestroyLabel(breakpoint_edit->break_flags.write.label);
    DestroyToggleButton(breakpoint_edit->break_flags.read.check_button);
    DestroyLabel(breakpoint_edit->break_flags.read.label);
    DestroyToggleButton(breakpoint_edit->break_flags.execute.check_button);
    DestroyLabel(breakpoint_edit->break_flags.execute.label);

    //Address
    BreakpointEdit_Address_Free(breakpoint_edit);

    //Condition
    BreakpointEdit_Condition_Free(breakpoint_edit);

    //Enabled
    DestroyToggleButton(breakpoint_edit->enabled.check_button);
    DestroyLabel(breakpoint_edit->enabled.label);

    //Ok
    DestroyButton(breakpoint_edit->ok);

    //Cancel
    DestroyButton(breakpoint_edit->cancel);


    WG_DestroyRenderer(breakpoint_edit->renderer);

    WG_DestroyWindow(breakpoint_edit->window);

    free(breakpoint_edit);
}