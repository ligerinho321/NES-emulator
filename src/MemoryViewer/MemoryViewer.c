#include <MemoryViewer/MemoryViewer.h>
#include <MemoryViewer/MemoryViewer_Utils.h>
#include <MemoryViewer/MemoryColumns.h>
#include <MemoryViewer/MemoryType.h>
#include <ScrollBar.h>
#include <ComboBox/ComboBox.h>
#include <Input.h>
#include <Utils.h>
#include <Resources.h>

void MemoryViewer_UpdateMemory(MemoryViewer *memory_viewer){

    memory_viewer->memory.selection.enabled = false;
    
    MemoryViewer_MemoryType_SetShowStrings(memory_viewer);

    ComboBox_SetCurrentElement(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.cpu_memory);

    Vector2 position = {
        .x = memory_viewer->memory_type.label->rect.x + memory_viewer->memory_type.label->rect.w,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f,
    };

    ComboBox_ConfigRect(memory_viewer->memory_type.combo_box,position);
}

void MemoryViewer_Init(MemoryViewer *memory_viewer,Emulator *emulator){
    memset(memory_viewer,0,sizeof(MemoryViewer));

    memory_viewer->emulator = emulator;

    memory_viewer->nes = &emulator->nes;

    memory_viewer->window_class = emulator->window_class;

    memory_viewer->font = emulator->font;

    memory_viewer->window_size = (Vector2){
        .x = 800,
        .y = 600
    };

    memory_viewer->mutex = CreateMutex(NULL,FALSE,NULL);
}

void MemoryViewer_ConfigRect(MemoryViewer *memory_viewer){
    
    Vector2 position;

    memory_viewer->bar_rect = (Rect){
        .x = 0,
        .y = 0,
        .w = memory_viewer->window_size.x,
        .h = ELEMENT_ROW_HEIGHT
    };

    memory_viewer->memory_type.label->rect = (Rect){
        .x = memory_viewer->bar_rect.x,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f,
        .w = memory_viewer->memory_type.label->size.x + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(memory_viewer->memory_type.label);
    
    position.x = memory_viewer->memory_type.label->rect.x + memory_viewer->memory_type.label->rect.w;
    position.y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f;

    ComboBox_ConfigRect(memory_viewer->memory_type.combo_box,position);


    memory_viewer->memory_columns.label->rect = (Rect){
        .x = memory_viewer->memory_type.combo_box->expand_button.rect.x + memory_viewer->memory_type.combo_box->expand_button.rect.w + BORDER_SPACING,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f,
        .w = memory_viewer->memory_columns.label->size.x + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(memory_viewer->memory_columns.label);

    position.x = memory_viewer->memory_columns.label->rect.x + memory_viewer->memory_columns.label->rect.w;
    position.y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f;

    ComboBox_ConfigRect(memory_viewer->memory_columns.combo_box,position);


    memory_viewer->memory_address.label->rect = (Rect){
        .x = memory_viewer->memory_columns.combo_box->expand_button.rect.x + memory_viewer->memory_columns.combo_box->expand_button.rect.w + BORDER_SPACING,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f,
        .w = memory_viewer->memory_address.label->size.x + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(memory_viewer->memory_address.label);

    memory_viewer->memory_address.input->rect = (Rect){
        .x = memory_viewer->memory_address.label->rect.x + memory_viewer->memory_address.label->rect.w,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h * 0.5f - ELEMENT_HEIGHT * 0.5f,
        .w = memory_viewer->memory_address.input->glyph_size.x * 8 + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };


    memory_viewer->info_rect = (Rect){
        .x = 0,
        .y = memory_viewer->window_size.y - ELEMENT_HEIGHT,
        .w = memory_viewer->window_size.x,
        .h = ELEMENT_HEIGHT
    };

    Memory_ConfigRect(memory_viewer);
}

void MemoryViewer_OpenWindow(MemoryViewer *memory_viewer){

    memory_viewer->window = WG_CreateWindow(
        &memory_viewer->window_class,
        NULL,
        L"Memory Viewer",
        WG_WINDOWPOS_CENTERED,
        WG_WINDOWPOS_CENTERED,
        memory_viewer->window_size.x,
        memory_viewer->window_size.y,
        WG_WINDOW_SHOWN | WG_WINDOW_DEFAULT,
        0
    );

    memory_viewer->renderer = WG_CreateRenderer(memory_viewer->window,NULL);
    
    memory_viewer->atlas = WG_RenderFontAtlas(memory_viewer->renderer,memory_viewer->font,FONT_HEIGHT,0x20,0xFF);
    
    WG_SizeTextByFontAtlas(memory_viewer->atlas,L"A",&memory_viewer->glyph_size.x,&memory_viewer->glyph_size.y);

    memory_viewer->arrow = WG_LoadTextureFromData(memory_viewer->renderer,arrow,arrow_size,TEXTURE_NONE);
    WG_SetTextureColor(memory_viewer->arrow,OFF_COLOR);

    memory_viewer->vertical_scroll_bar = calloc(1,sizeof(ScrollBar));

    memory_viewer->memory_address.label = CreateLabel(L"Address:",memory_viewer->atlas);

    memory_viewer->memory_address.input = CreateInput(
        INPUT_HEXADECIMAL | INPUT_RETURN_CONTENT | INPUT_SUBMIT,
        memory_viewer->atlas,
        Memory_SetAddress,
        memory_viewer
    );
    
    MemoryViewer_MemoryType_Init(memory_viewer);
    
    MemoryViewer_MemoryColumns_Init(memory_viewer);
    
    MemoryViewer_ConfigRect(memory_viewer);

    memory_viewer->run = true;

    memory_viewer->update_memory = false;
}

void MemoryViewer_CloseWindow(MemoryViewer *memory_viewer){

    free(memory_viewer->vertical_scroll_bar);

    MemoryViewer_MemoryType_Free(memory_viewer);



    DestroyLabel(memory_viewer->memory_address.label);
    DestroyInput(memory_viewer->memory_address.input);

    WG_DestroyTexture(memory_viewer->arrow);

    WG_FontAtlasFree(memory_viewer->atlas);

    WG_DestroyRenderer(memory_viewer->renderer);

    WG_DestroyWindow(memory_viewer->window);

    memory_viewer->run = false;
}

void MemoryViewer_Event(MemoryViewer *memory_viewer){
    
    WG_PollEvent(memory_viewer->window,&memory_viewer->event);

    WG_GetMousePosInWindow(memory_viewer->window,&memory_viewer->mouse_pos.x,&memory_viewer->mouse_pos.y);

    ComboBox_Event(memory_viewer->memory_type.combo_box,memory_viewer->event,memory_viewer->mouse_pos);

    ComboBox_Event(memory_viewer->memory_columns.combo_box,memory_viewer->event,memory_viewer->mouse_pos);

    InputEvent(memory_viewer->memory_address.input,memory_viewer->event,memory_viewer->mouse_pos);

    if(PointInRect(memory_viewer->mouse_pos,memory_viewer->vertical_scroll_bar->scroll_rect)){
        memory_viewer->vertical_scroll_bar->mouse.over = true;
    }

    if(memory_viewer->event.type == WG_QUIT){
        
        memory_viewer->run = false;
    }
    else if(memory_viewer->event.type == WG_WINDOWRESIZE){

        WG_GetWindowClientSize(memory_viewer->window,&memory_viewer->window_size.x,&memory_viewer->window_size.y);
        
        MemoryViewer_ConfigRect(memory_viewer);
    }
    else if(memory_viewer->event.type == WG_MOUSEWHEEL){

        if(PointInRect(memory_viewer->mouse_pos,memory_viewer->memory.rect)){
            
            if(memory_viewer->event.mouse.wheel < 0.0f){
                SetYScrollBar(memory_viewer->vertical_scroll_bar,memory_viewer->vertical_scroll_bar->offset - (memory_viewer->glyph_size.y * 3));
            }
            else if(memory_viewer->event.mouse.wheel > 0.0f){
                SetYScrollBar(memory_viewer->vertical_scroll_bar,memory_viewer->vertical_scroll_bar->offset + (memory_viewer->glyph_size.y * 3));
            }

            Memory_SetVisibleLines(memory_viewer);
        }
    }
    else if(memory_viewer->event.type == WG_MOUSEBUTTONDOWN && memory_viewer->event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(memory_viewer->vertical_scroll_bar->mouse.over){
            memory_viewer->vertical_scroll_bar->mouse.pressed = true;
            memory_viewer->vertical_scroll_bar->mouse.diff = memory_viewer->mouse_pos.y - memory_viewer->vertical_scroll_bar->scroll_rect.y;
        }
        else if(PointInRect(memory_viewer->mouse_pos,memory_viewer->memory.values_rect)){

            uint32_t address = Memory_GetPos(memory_viewer);

            memory_viewer->memory.selection.enabled = false;

            memory_viewer->memory.input.cursor.on = true;

            memory_viewer->memory.selection._start = address;
            memory_viewer->memory.selection._end = address;
            memory_viewer->memory.selection.start = address;
            memory_viewer->memory.selection.end = address;

            Memory_SetInputAddress(memory_viewer,address);
        }
        else{
            memory_viewer->memory.selection.enabled = false;
        }
    }
    else if(memory_viewer->event.type == WG_MOUSEBUTTONUP && memory_viewer->event.mouse.button == WG_MOUSEBUTTON_LEFT){
       
        memory_viewer->vertical_scroll_bar->mouse.pressed = false;
        memory_viewer->memory.input.cursor.on = false;
    }
    else if(WG_GetMouseState() & WG_MOUSEBUTTON_LEFT){

        if(memory_viewer->vertical_scroll_bar->mouse.pressed){
            HandleVerticalScrollBar(memory_viewer->vertical_scroll_bar,memory_viewer->mouse_pos);
            Memory_SetVisibleLines(memory_viewer);
        }
        else if(memory_viewer->memory.input.cursor.on){
            Memory_DraggingMouse(memory_viewer);
        }
    }
    else if(memory_viewer->event.type == WG_TEXTINPUT && !memory_viewer->memory_address.input->on){
        
        uint16_t *entry = wcscasechr(L"0123456789ABCDEF",memory_viewer->event.text_input);
            
        if(entry != NULL){
            
            uint16_t character = towupper(memory_viewer->event.text_input);

            Memory_Input(memory_viewer,character);
        }
    }
    else if(memory_viewer->event.type == WG_KEYDOWN && !memory_viewer->memory_address.input->on){

        if(WG_GetKeyModState() & WG_KEYMOD_CTRL){

            if(memory_viewer->event.key.scancode == WG_SCANCODE_C && memory_viewer->memory.selection.enabled){
                uint16_t *content = Memory_GetSelectionContent(memory_viewer);
                WG_SetClipboardText(content);
                free(content);
            }
            else if(memory_viewer->event.key.scancode == WG_SCANCODE_V && memory_viewer->memory.selection.enabled){
                uint16_t *content = WG_GetClipboardText();
                Memory_SetContent(memory_viewer,content);
                free(content);
            }
        }
        else if(memory_viewer->event.key.scancode == WG_SCANCODE_UP && (int)memory_viewer->memory.input.address - (int)memory_viewer->memory_columns.number >= 0){
            Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address - memory_viewer->memory_columns.number);
        }
        else if(memory_viewer->event.key.scancode == WG_SCANCODE_DOWN){
            Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address + memory_viewer->memory_columns.number);
        }
        else if(memory_viewer->event.key.scancode == WG_SCANCODE_LEFT && memory_viewer->memory.input.address > 0){
            Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address - 1);
        }
        else if(memory_viewer->event.key.scancode == WG_SCANCODE_RIGHT){
            Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address + 1);
        }
    }
}

void MemoryViewer_Draw(MemoryViewer *memory_viewer){

    WG_Renderer *renderer = memory_viewer->renderer;

    WG_SetCurrentRenderer(memory_viewer->renderer);
    
    WG_ClearColor(renderer,BACKGROUND1_COLOR);

    //MemoryType
    DrawLabel(renderer,memory_viewer->memory_type.label,OFF_COLOR);
    ComboBox_Draw(memory_viewer->memory_type.combo_box,renderer);

    //MemoryColumns
    DrawLabel(renderer,memory_viewer->memory_columns.label,OFF_COLOR);
    ComboBox_Draw(memory_viewer->memory_columns.combo_box,renderer);

    //MemoryAddress
    DrawLabel(renderer,memory_viewer->memory_address.label,OFF_COLOR);
    DrawInput(memory_viewer->memory_address.input,renderer,BACKGROUND1_COLOR,BORDER_COLOR,ON_COLOR,OFF_COLOR);

    Memory_Draw(memory_viewer);

    DrawScrollBar(memory_viewer->vertical_scroll_bar,renderer);

    //Info
    swprintf(
        memory_viewer->buffer,
        BUFFER_LENGTH,
        L"Address: %.*X  Row: %d  Column: %d",
        memory_viewer->memory.address_row_length,
        memory_viewer->memory.input.address,
        memory_viewer->memory.input.row,
        memory_viewer->memory.input.column
    );

    Vector2 position = {
        .x = memory_viewer->info_rect.x + 5,
        .y = memory_viewer->info_rect.y + 5
    };

    WG_DrawText(renderer,memory_viewer->atlas,memory_viewer->buffer,position,OFF_COLOR);

    WG_Flip(renderer);
}

void MemoryViewer_RunMainWindow(MemoryViewer *memory_viewer){
    MemoryViewer_Event(memory_viewer);
    MemoryViewer_Draw(memory_viewer);
}

void MemoryViewer_RunChildrenWindow(MemoryViewer *memory_viewer){
    if(memory_viewer->memory_type.combo_box->menu->open){
        ComboBoxMenu_Run(memory_viewer->memory_type.combo_box->menu);
    }

    if(memory_viewer->memory_columns.combo_box->menu->open){
        ComboBoxMenu_Run(memory_viewer->memory_columns.combo_box->menu);
    }
}

DWORD WINAPI MemoryViewer_Main(LPVOID lParam){

    MemoryViewer *memory_viewer = lParam;

    MemoryViewer_OpenWindow(memory_viewer);

    while(memory_viewer->run){
        
        if(memory_viewer->update_memory){
            
            WaitForSingleObject(memory_viewer->mutex,INFINITE);
            
            memory_viewer->update_memory = false;

            ReleaseMutex(memory_viewer->mutex);
            
            MemoryViewer_UpdateMemory(memory_viewer);
        }

        MemoryViewer_RunMainWindow(memory_viewer);
        MemoryViewer_RunChildrenWindow(memory_viewer);
    }

    MemoryViewer_CloseWindow(memory_viewer);

    return 0;
}

void MemoryViewer_Execute(MemoryViewer *memory_viewer){

    MemoryViewer_Exit(memory_viewer);

    memory_viewer->thread = CreateThread(NULL,0,MemoryViewer_Main,memory_viewer,0,&memory_viewer->threadID);
    
    if(memory_viewer->thread){
        memory_viewer->run = true;
    }
    else{
        printf("Failed to initialize MemoryViewer: %d.\n",GetLastError());
    }
}

void MemoryViewer_Exit(MemoryViewer *memory_viewer){
    memory_viewer->run = false;
    WaitForSingleObject(memory_viewer->thread,INFINITE);
    CloseHandle(memory_viewer->thread);
    memory_viewer->thread = NULL;
}

void MemoryViewer_Free(MemoryViewer *memory_viewer){
    MemoryViewer_Exit(memory_viewer);
    CloseHandle(memory_viewer->mutex);
    free(memory_viewer);
}