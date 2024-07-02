#include <MemoryViewer/MemoryViewer_Utils.h>
#include <Utils.h>
#include <ScrollBar.h>
#include <Input.h>

void Memory_AjustVerticalScrollBarOffsetByAddress(MemoryViewer *memory_viewer){
    
    if(!memory_viewer->vertical_scroll_bar->valid) return;

    uint32_t line = 0;
    
    if(memory_viewer->memory.input.address > 0 && memory_viewer->memory_columns.number > 0){
        line = memory_viewer->memory.input.address / memory_viewer->memory_columns.number;
    }

    float length_diff = memory_viewer->vertical_scroll_bar->length - memory_viewer->vertical_scroll_bar->bar_rect.h;
    float scroll_length = memory_viewer->vertical_scroll_bar->bar_rect.h - memory_viewer->vertical_scroll_bar->scroll_rect.h;


    if((line + 1) * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset > memory_viewer->vertical_scroll_bar->bar_rect.h){

        float diff = (line + 1) * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset - memory_viewer->vertical_scroll_bar->bar_rect.h;
        memory_viewer->vertical_scroll_bar->scroll_rect.y +=  diff / length_diff * scroll_length;
    }
    else if(line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset < 0.0f){

        float diff = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset;
        memory_viewer->vertical_scroll_bar->scroll_rect.y +=  diff / length_diff * scroll_length;
    }

    SetVerticalOffsetScrollBar(memory_viewer->vertical_scroll_bar);

    Memory_SetVisibleLines(memory_viewer);
}

void Memory_UpdateInputCursor(MemoryViewer *memory_viewer){

    if(memory_viewer->memory.input.cursor.update){

        memory_viewer->memory.input.cursor.update = false;

        memory_viewer->memory.input.cursor.visible = true;

        memory_viewer->memory.input.cursor.time = GetTickCount();
    }
    else if(GetTickCount() - memory_viewer->memory.input.cursor.time >= BLINK_TIME){

        memory_viewer->memory.input.cursor.visible = !memory_viewer->memory.input.cursor.visible;
        
        memory_viewer->memory.input.cursor.time = GetTickCount();
    }
}

void Memory_WriteInput(MemoryViewer *memory_viewer){
    
    if(memory_viewer->memory.selection.enabled){
            
        memory_viewer->memory.selection.enabled = false;

        for(uint32_t i=memory_viewer->memory.selection.start; i<=memory_viewer->memory.selection.end; ++i){
            
            memory_viewer->memory.write(memory_viewer,memory_viewer->memory.input.value,i);
        }
            
        Memory_SetInputAddress(memory_viewer,memory_viewer->memory.selection.end + 1);
    }
    else{
        memory_viewer->memory.write(memory_viewer,memory_viewer->memory.input.value,memory_viewer->memory.input.address);
        
        Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address + 1);
    }
}

void Memory_Input(MemoryViewer *memory_viewer,uint16_t character){
    
    if(memory_viewer->memory.input.editing){
        
        memory_viewer->memory.input.editing = false;
        
        memory_viewer->memory.input.buffer[1] = character;
        
        memory_viewer->memory.input.value = wcstol(memory_viewer->memory.input.buffer,NULL,16);

        Memory_WriteInput(memory_viewer);
    }
    else{
        memory_viewer->memory.input.editing = true;
                
        if(memory_viewer->memory.selection.enabled){

            memory_viewer->memory.input.buffer[0] = character;
            memory_viewer->memory.input.buffer[1] = L'0';
        }
        else{
            uint8_t value = memory_viewer->memory.read(memory_viewer,memory_viewer->memory.input.address);
                    
            swprintf(memory_viewer->memory.input.buffer,3,L"%.2X",value);
                    
            memory_viewer->memory.input.buffer[0] = character;
        }

        memory_viewer->memory.input.value = wcstol(memory_viewer->memory.input.buffer,NULL,16);
    }
}

void Memory_SetInputAddress(MemoryViewer *memory_viewer,uint32_t address){
    
    if(address >= memory_viewer->memory_type.length){
        address = memory_viewer->memory_type.length - 1;
    }

    if(memory_viewer->memory.input.editing){
        memory_viewer->memory.input.editing = false;
        Memory_WriteInput(memory_viewer);
    }
    else if(!memory_viewer->memory.input.cursor.on && memory_viewer->memory.selection.enabled){
        memory_viewer->memory.selection.enabled = false;
    }

    memory_viewer->memory.input.address = address;

    if(memory_viewer->memory.input.address > 0){
        memory_viewer->memory.input.row = memory_viewer->memory.input.address / memory_viewer->memory_columns.number;
        memory_viewer->memory.input.column = memory_viewer->memory.input.address % memory_viewer->memory_columns.number;
    }
    else{
        memory_viewer->memory.input.row = 0;
        memory_viewer->memory.input.column = 0;
    }

    memory_viewer->memory.input.cursor.update = true;

    Memory_AjustVerticalScrollBarOffsetByAddress(memory_viewer);
}

uint32_t Memory_GetPos(MemoryViewer *memory_viewer){
    Vector2 position;

    position.x = (memory_viewer->mouse_pos.x - memory_viewer->memory.values_rect.x);
    position.y = (memory_viewer->mouse_pos.y - memory_viewer->memory.values_rect.y) - memory_viewer->vertical_scroll_bar->offset;
            
    position.x = position.x / (memory_viewer->glyph_size.x * 3.0f);
    position.y = position.y / memory_viewer->glyph_size.y;

    uint32_t columns = memory_viewer->memory_columns.number;
    uint32_t rows = ceilf((float)memory_viewer->memory_type.length / (float)memory_viewer->memory_columns.number);

    if(position.x < 0){
        position.x = 0;
    }
    else if(position.x >= columns){
        position.x = columns - 1;
    }

    if(position.y < 0){
        position.y = 0;
    }
    else if(position.y >= rows){
        position.y = rows - 1;
    }
    
    return position.y * memory_viewer->memory_columns.number + position.x;
}

void Memory_DraggingMouse(MemoryViewer *memory_viewer){

    uint32_t address = Memory_GetPos(memory_viewer);

    if(memory_viewer->memory.selection._start != address){

        memory_viewer->memory.selection.enabled = true;

        memory_viewer->memory.selection._end = address;

        if(memory_viewer->memory.selection._start > memory_viewer->memory.selection._end){

            memory_viewer->memory.selection.start = memory_viewer->memory.selection._end;
            memory_viewer->memory.selection.end = memory_viewer->memory.selection._start;
        }
        else{
            memory_viewer->memory.selection.start = memory_viewer->memory.selection._start;
            memory_viewer->memory.selection.end = memory_viewer->memory.selection._end;
        }

        Memory_SetInputAddress(memory_viewer,address);
    }
    else if(memory_viewer->memory.selection._start == address && memory_viewer->memory.selection.enabled){
        
        memory_viewer->memory.selection.enabled = false;

        Memory_SetInputAddress(memory_viewer,address);
    }
}

uint16_t* Memory_GetSelectionContent(MemoryViewer *memory_viewer){

    if(!memory_viewer->memory.selection.enabled) return NULL;

    GapBuffer *gap_buffer = NULL;
    CreateGapBuffer(gap_buffer,uint16_t,20);

    uint8_t value = 0;
    uint32_t size = 0;

    for(uint32_t i=memory_viewer->memory.selection.start; i<=memory_viewer->memory.selection.end; ++i){

        value = memory_viewer->memory.read(memory_viewer,i);

        if(i < memory_viewer->memory.selection.end){
            size = swprintf(memory_viewer->buffer,BUFFER_LENGTH,L"%.2X, ",value);
        }
        else{
            size = swprintf(memory_viewer->buffer,BUFFER_LENGTH,L"%.2X",value);
        }

        GapBufferInsertValues(gap_buffer,uint16_t,memory_viewer->buffer,size);
    }

    uint16_t *content = NULL;
    GapBufferGetContent(gap_buffer,uint16_t,content);
    
    GapBufferFree(gap_buffer);

    return content;
}

void Memory_SetContent(MemoryViewer *memory_viewer,uint16_t *content){

    if(!memory_viewer->memory.selection.enabled) return;

    uint16_t *ptr = content;
    uint16_t *entry = NULL;

    uint8_t value = 0;
    
    uint16_t buffer[3] = {0};
    uint8_t index = 0;
    
    uint32_t address = memory_viewer->memory.selection.start;

    while(ptr[0] != L'\0'){

        entry = wcscasechr(L"0123456789ABCDEF",ptr[0]);

        if(entry && index < 2){
            buffer[index++] = ptr[0];
        }

        if((!entry && index > 0) || index >= 2){

            value = wcstol(buffer,NULL,16);

            memory_viewer->memory.write(memory_viewer,value,address);

            if(++address > memory_viewer->memory.selection.end){
                break;
            }

            index = 0;

            buffer[0] = buffer[1] = buffer[2] = L'\0';
        }

        ++ptr;
    }

    memory_viewer->memory.selection.enabled = false;

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.selection.end + 1);
}

void Memory_ConfigRect(MemoryViewer *memory_viewer){

    if(!memory_viewer->memory_type.length || !memory_viewer->memory_columns.number) return;

    memory_viewer->memory.address_column_rect = (Rect){
        .x = 0,
        .y = memory_viewer->bar_rect.y + memory_viewer->bar_rect.h,
        .w = memory_viewer->window_size.x,
        .h = memory_viewer->glyph_size.y + BORDER_SPACING * 2
    };

    memory_viewer->memory.rect = (Rect){
        .x = 0,
        .y = memory_viewer->memory.address_column_rect.y + memory_viewer->memory.address_column_rect.h,
        .w = memory_viewer->window_size.x,
        .h = memory_viewer->window_size.y - memory_viewer->bar_rect.h - memory_viewer->memory.address_column_rect.h - memory_viewer->info_rect.h,
    };

    memory_viewer->memory.address_row_rect = (Rect){
        .x = 0,
        .y = 0,
        .w = memory_viewer->memory.address_row_width,
        .h = memory_viewer->window_size.y
    };

    memory_viewer->memory.values_rect = (Rect){
        .x = memory_viewer->memory.rect.x + memory_viewer->memory.address_row_rect.x + memory_viewer->memory.address_row_rect.w,
        .y = memory_viewer->memory.rect.y,
        .w = memory_viewer->memory_columns.number * memory_viewer->glyph_size.x * 3,
        .h = memory_viewer->memory.rect.h
    };

    memory_viewer->vertical_scroll_bar->bar_rect = (Rect){
        .x = memory_viewer->memory.rect.x + memory_viewer->memory.rect.w - SCROLL_BAR_SIZE,
        .y = memory_viewer->memory.rect.y,
        .w = SCROLL_BAR_SIZE,
        .h = memory_viewer->memory.rect.h
    };

    memory_viewer->vertical_scroll_bar->scroll_rect = (RectF){
        .x = memory_viewer->vertical_scroll_bar->bar_rect.x,
        .y = memory_viewer->vertical_scroll_bar->bar_rect.y,
        .w = memory_viewer->vertical_scroll_bar->bar_rect.w,
        .h = 0.0f
    };

    memory_viewer->vertical_scroll_bar->length = ceilf((float)memory_viewer->memory_type.length / (float)memory_viewer->memory_columns.number) * memory_viewer->glyph_size.y;
    memory_viewer->vertical_scroll_bar->length += memory_viewer->memory.rect.h - memory_viewer->glyph_size.y;

    memory_viewer->vertical_scroll_bar->scroll_rect.h = GetScrollBarHeight(*memory_viewer->vertical_scroll_bar);

    if(memory_viewer->vertical_scroll_bar->scroll_rect.h > 0.0f && memory_viewer->vertical_scroll_bar->scroll_rect.h < memory_viewer->vertical_scroll_bar->bar_rect.h){
        memory_viewer->vertical_scroll_bar->valid = true;
        SetYScrollBar(memory_viewer->vertical_scroll_bar,memory_viewer->vertical_scroll_bar->offset);
    }
    else{
        memory_viewer->vertical_scroll_bar->valid = false;
        memory_viewer->vertical_scroll_bar->offset = 0.0f;
    }

    Memory_SetVisibleLines(memory_viewer);
}

void Memory_SetVisibleLines(MemoryViewer *memory_viewer){

    if(!memory_viewer->memory_type.length) return;

    memory_viewer->viewer_metrics.start_line = (uint32_t)-1;
    memory_viewer->viewer_metrics.end_line = (uint32_t)-1;
    memory_viewer->viewer_metrics.start_address = (uint32_t)-1;
    memory_viewer->viewer_metrics.end_address = (uint32_t)-1;

    uint32_t address = 0;
    uint32_t line = 0;

    while(address < memory_viewer->memory_type.length){
        if(line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset >= memory_viewer->memory.rect.h){
            break;
        }
        else if((line+1) * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset >= 0.0f){
            
            if(memory_viewer->viewer_metrics.start_address == (uint32_t)-1 && memory_viewer->viewer_metrics.start_line == (uint32_t)-1){
                
                memory_viewer->viewer_metrics.start_address = address;
                memory_viewer->viewer_metrics.start_line = line;

                memory_viewer->viewer_metrics.end_address = address;
                memory_viewer->viewer_metrics.end_line = line;
            }
            else{
                memory_viewer->viewer_metrics.end_address = address;
                memory_viewer->viewer_metrics.end_line = line;
            }

        }
        address += memory_viewer->memory_columns.number;
        ++line;
    }
}


static void Memory_DrawColumns(MemoryViewer *memory_viewer){

    uint32_t size = 0;
    for(int i=0; i<memory_viewer->memory_columns.number; ++i){
        size += swprintf(memory_viewer->buffer + size,BUFFER_LENGTH - size,L"%.2X ",i);
    }

    Vector2 position = {
        .x = memory_viewer->memory.address_row_rect.w + memory_viewer->glyph_size.x * 0.5f,
        .y = memory_viewer->memory.address_column_rect.y + memory_viewer->memory.address_column_rect.h * 0.5f - memory_viewer->glyph_size.y * 0.5f
    };

    WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer,position,OFF_COLOR);
}

static void Memory_DrawAddress(MemoryViewer *memory_viewer,uint32_t address,uint32_t line){

    swprintf(memory_viewer->buffer,BUFFER_LENGTH,L"%.*X",memory_viewer->memory.address_row_length,address);

    Vector2 position = {
        .x = memory_viewer->memory.address_row_width * 0.5f - (memory_viewer->memory.address_row_length * memory_viewer->glyph_size.x) * 0.5f,
        .y = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset
    };
            
    WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer,position,OFF_COLOR);
}

static void Memory_DrawValuesSelected(MemoryViewer *memory_viewer,uint32_t columns,uint32_t address,uint32_t line){
    
    if(!memory_viewer->memory.selection.enabled) return;

    Vector2 position = {
        .x = memory_viewer->memory.address_row_width + memory_viewer->glyph_size.x * 0.5f,
        .y = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset
    };

    // .|....|.
    if(memory_viewer->memory.selection.start >= address && memory_viewer->memory.selection.end < address + columns){
            
        Rect rect = {
            .x = position.x + (memory_viewer->memory.selection.start % columns) * memory_viewer->glyph_size.x * 3,
            .y = position.y,
            .w = ((memory_viewer->memory.selection.end - memory_viewer->memory.selection.start) * memory_viewer->glyph_size.x * 3) + memory_viewer->glyph_size.x * 2,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // .|.....
    else if(memory_viewer->memory.selection.start >= address && memory_viewer->memory.selection.start < address + columns){

        uint32_t column = (memory_viewer->memory.selection.start % columns);

        Rect rect = {
            .x = position.x + column * memory_viewer->glyph_size.x * 3,
            .y = position.y,
            .w = ((columns - column - 1) * memory_viewer->glyph_size.x * 3) + memory_viewer->glyph_size.x * 2,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // <......> 
    else if(memory_viewer->memory.selection.start < address && memory_viewer->memory.selection.end >= address + columns){

        Rect rect = {
            .x = position.x,
            .y = position.y,
            .w = ((columns - 1) * memory_viewer->glyph_size.x * 3) + memory_viewer->glyph_size.x * 2,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // .....|.
    else if(memory_viewer->memory.selection.end >= address && memory_viewer->memory.selection.end < address + columns){

        Rect rect = {
            .x = position.x,
            .y = position.y,
            .w = ((memory_viewer->memory.selection.end % columns) * memory_viewer->glyph_size.x * 3) + memory_viewer->glyph_size.x * 2,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
}

static void Memory_DrawValues(MemoryViewer *memory_viewer,uint32_t columns,uint32_t address,uint32_t line){
    
    uint32_t size = 0;
    uint8_t value = 0;

    Vector2 position = {
        .x = memory_viewer->memory.address_row_width + memory_viewer->glyph_size.x * 0.5f,
        .y = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset
    };

    for(int i=0; i<columns; ++i){

        value = memory_viewer->memory.read(memory_viewer,address + i);

        if(memory_viewer->memory.input.editing){
            
            if(memory_viewer->memory.selection.enabled){
                
                if(address + i >= memory_viewer->memory.selection.start && address + i <= memory_viewer->memory.selection.end){
                    
                    value = memory_viewer->memory.input.value;
                }
            }
            else if(address + i == memory_viewer->memory.input.address){
                
                value = memory_viewer->memory.input.value;
            }
        }

        size += swprintf(memory_viewer->buffer + size,BUFFER_LENGTH - size,L"%.2X ",value);
    }

    WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer,position,ON_COLOR);
}

static void Memory_DrawCharactersSelected(MemoryViewer *memory_viewer,uint32_t columns,uint32_t address,uint32_t line){
    
    if(!memory_viewer->memory.selection.enabled) return;

    Vector2 position = {
        .x = memory_viewer->memory.values_rect.x + memory_viewer->memory.values_rect.w + memory_viewer->glyph_size.x * 4,
        .y = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset
    };

    // .|....|.
    if(memory_viewer->memory.selection.start >= address && memory_viewer->memory.selection.end < address + columns){
            
        Rect rect = {
            .x = position.x + (memory_viewer->memory.selection.start % columns) * memory_viewer->glyph_size.x,
            .y = position.y,
            .w = ((memory_viewer->memory.selection.end - memory_viewer->memory.selection.start) * memory_viewer->glyph_size.x) + memory_viewer->glyph_size.x,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // .|.....
    else if(memory_viewer->memory.selection.start >= address && memory_viewer->memory.selection.start < address + columns){

        uint32_t column = (memory_viewer->memory.selection.start % columns);

        Rect rect = {
            .x = position.x + column * memory_viewer->glyph_size.x,
            .y = position.y,
            .w = ((columns - column - 1) * memory_viewer->glyph_size.x) + memory_viewer->glyph_size.x,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // <......> 
    else if(memory_viewer->memory.selection.start < address && memory_viewer->memory.selection.end >= address + columns){

        Rect rect = {
            .x = position.x,
            .y = position.y,
            .w = ((columns - 1) * memory_viewer->glyph_size.x) + memory_viewer->glyph_size.x,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
    // .....|.
    else if(memory_viewer->memory.selection.end >= address && memory_viewer->memory.selection.end < address + columns){

        Rect rect = {
            .x = position.x,
            .y = position.y,
            .w = ((memory_viewer->memory.selection.end % columns) * memory_viewer->glyph_size.x) + memory_viewer->glyph_size.x,
            .h = memory_viewer->glyph_size.y
        };

        WG_DrawFilledRectangle(memory_viewer->renderer,&rect,(Color){80,80,80,255});
    }
}

static void Memory_DrawCharacters(MemoryViewer *memory_viewer,uint32_t columns,uint32_t address,uint32_t line){

    uint32_t size = 0;
    uint16_t value = 0;

    Vector2 position = {
        .x = memory_viewer->memory.values_rect.x + memory_viewer->memory.values_rect.w + memory_viewer->glyph_size.x * 4,
        .y = line * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset
    };

    for(int i=0; i<columns; ++i){

        value = memory_viewer->memory.read(memory_viewer,address + i);

        if(memory_viewer->memory.input.editing){

            if(memory_viewer->memory.selection.enabled){
                
                if(address + i >= memory_viewer->memory.selection.start && address + i <= memory_viewer->memory.selection.end){
                    value = memory_viewer->memory.input.value;
                }
            }
            else if(address + i == memory_viewer->memory.input.address){
                value = memory_viewer->memory.input.value;
            }
        }

        if(value <= 0x20 || value >= 0x7F) value = L'.';

        size += swprintf(memory_viewer->buffer + size,BUFFER_LENGTH - size,L"%lc",value);
    }

    WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer,position,ON_COLOR);
}

static void Memory_DrawInput(MemoryViewer *memory_viewer){

    Memory_UpdateInputCursor(memory_viewer);

    if(!memory_viewer->memory.input.cursor.visible) return;

    Vector2 position;
    Rect rect;
    uint16_t value;

    if(memory_viewer->memory.input.editing){
        value = memory_viewer->memory.input.value;
    }
    else{
        value = memory_viewer->memory.read(memory_viewer,memory_viewer->memory.input.address);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Value

    position.x = memory_viewer->memory.values_rect.x + memory_viewer->glyph_size.x * 0.5f;
    position.y = memory_viewer->memory.input.row * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset;

    rect = (Rect){
        .x = position.x + memory_viewer->memory.input.column * memory_viewer->glyph_size.x * 3 + memory_viewer->memory.input.editing * memory_viewer->glyph_size.x,
        .y = position.y,
        .w = memory_viewer->glyph_size.x,
        .h = memory_viewer->glyph_size.y
    };

    WG_DrawFilledRectangle(memory_viewer->renderer,&rect,ON_COLOR);

    position.x = rect.x;
    position.y = rect.y;

    if(memory_viewer->memory.input.editing){
        swprintf(memory_viewer->buffer,3,L"%.2X",value);
        WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer + 1,position,BACKGROUND2_COLOR);
    }
    else{
        swprintf(memory_viewer->buffer,2,L"%.2X",value);
        WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer + 0,position,BACKGROUND2_COLOR);
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Character

    position.x = memory_viewer->memory.values_rect.x + memory_viewer->memory.values_rect.w + memory_viewer->glyph_size.x * 4;
    position.y = memory_viewer->memory.input.row * memory_viewer->glyph_size.y + memory_viewer->vertical_scroll_bar->offset;

    rect = (Rect){
        .x = position.x + memory_viewer->memory.input.column * memory_viewer->glyph_size.x,
        .y = position.y,
        .w = memory_viewer->glyph_size.x,
        .h = memory_viewer->glyph_size.y
    };

    WG_DrawFilledRectangle(memory_viewer->renderer,&rect,ON_COLOR);

    if(value <= 0x20 || value >= 0x7F) value = L'.';

    swprintf(memory_viewer->buffer,BUFFER_LENGTH,L"%lc",value);

    position.x = rect.x;
    position.y = rect.y;

    WG_DrawText(memory_viewer->renderer,memory_viewer->atlas,memory_viewer->buffer,position,BACKGROUND2_COLOR);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void Memory_Draw(MemoryViewer *memory_viewer){
    
    if(!memory_viewer->memory_type.length || !memory_viewer->memory_columns.number) return;

    uint32_t address = memory_viewer->viewer_metrics.start_address;
    uint32_t line = memory_viewer->viewer_metrics.start_line;
    uint32_t columns = 0;
    Rect rect;

    WG_DrawFilledRectangle(memory_viewer->renderer,&memory_viewer->memory.address_column_rect,BACKGROUND1_COLOR);

    Memory_DrawColumns(memory_viewer);

    WG_RendererSetViewport(memory_viewer->renderer,NULL,&memory_viewer->memory.rect);

    WG_DrawFilledRectangle(memory_viewer->renderer,NULL,BACKGROUND2_COLOR);

    WG_DrawFilledRectangle(memory_viewer->renderer,&memory_viewer->memory.address_row_rect,BACKGROUND1_COLOR);

    while(address <= memory_viewer->viewer_metrics.end_address && line <= memory_viewer->viewer_metrics.end_line){

        if(address + memory_viewer->memory_columns.number > memory_viewer->memory_type.length){
            columns = memory_viewer->memory_type.length - address;
        }
        else{
            columns = memory_viewer->memory_columns.number;
        }

        Memory_DrawAddress(memory_viewer,address,line);

        Memory_DrawValuesSelected(memory_viewer,columns,address,line);
        
        Memory_DrawValues(memory_viewer,columns,address,line);

        Memory_DrawCharactersSelected(memory_viewer,columns,address,line);

        Memory_DrawCharacters(memory_viewer,columns,address,line);

        if(memory_viewer->memory.input.row == line){
            Memory_DrawInput(memory_viewer);
        }

        address += memory_viewer->memory_columns.number;
        ++line;
    }

    WG_RendererSetViewport(memory_viewer->renderer,NULL,NULL);

}


void Memory_SetAddress(void *userdata,const uint16_t *content,uint64_t length){

    MemoryViewer *memory_viewer = userdata;

    long address = wcstol(content,NULL,16);

    int32_t len = 0;

    if(address > 0 && memory_viewer->memory_columns.number > 0){
        len = (address / memory_viewer->memory_columns.number) * memory_viewer->glyph_size.y;
    }

    SetYScrollBar(memory_viewer->vertical_scroll_bar,-1.0f * len);

    Memory_SetVisibleLines(memory_viewer);

    if(!ContentSize(memory_viewer->memory_address.input->gap_buffer,uint16_t)){
        GapBufferInsertValue(memory_viewer->memory_address.input->gap_buffer,uint16_t,L'0');
    }
}