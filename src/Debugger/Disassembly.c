#include <Debugger/Disassembly.h>
#include <NES/CPU.h>
#include <Utils.h>
#include <ScrollBar.h>
#include <DebugMemory.h>


void Disassembly_Init(DebuggerDisassembly *disassembly){    
    disassembly->nes = disassembly->debugger->nes;
    
    disassembly->vertical_scroll_bar = calloc(1,sizeof(ScrollBar));

    disassembly->update = false;

    disassembly->config_rect = Disassembly_ConfigRect;
}

void Disassembly_SetVisibleLines(DebuggerDisassembly *disassembly){

    disassembly->metrics.start_address = (uint16_t)-1;
    disassembly->metrics.end_address = (uint16_t)-1;
    disassembly->metrics.start_line = (uint16_t)-1;
    disassembly->metrics.end_line = (uint16_t)-1;

    uint32_t address = 0;
    uint32_t line = 0;
    uint8_t opcode = 0;

    CPU *cpu = &disassembly->nes->cpu;

    while(address <= 0xFFFF){
        
        if(line * disassembly->debugger->glyph_size.y + disassembly->vertical_scroll_bar->offset >= disassembly->vertical_scroll_bar->bar_rect.h){
            break;
        }
        else if((line + 1) * disassembly->debugger->glyph_size.y + disassembly->vertical_scroll_bar->offset >= 0.0f){
            
            if(disassembly->metrics.start_address == (uint16_t)-1 && disassembly->metrics.start_line == (uint16_t)-1){

                disassembly->metrics.start_line = line;
                disassembly->metrics.start_address = address;

                disassembly->metrics.end_line = line;
                disassembly->metrics.end_address = address;
            }
            else{

                disassembly->metrics.end_line = line;
                disassembly->metrics.end_address = address;
            }
        }

        opcode = (address >= 0x8000 && address <= 0xFFFF) ? Debug_CPU_Read8(disassembly->nes,address) : 0x00;

        address += cpu->instruction_table[opcode].bytes;

        ++line;
    }
}

void Disassembly_ConfigVerticalScrollBar(DebuggerDisassembly *disassembly){

    uint32_t address = 0;
    uint32_t line = 0;
    uint8_t opcode = 0;
    
    CPU *cpu = &disassembly->nes->cpu;

    line = address = 0x8000;

    while(address <= 0xFFFF){
        opcode = Debug_CPU_Read8(disassembly->nes,address);
        address += cpu->instruction_table[opcode].bytes;
        ++line;
    }

    disassembly->vertical_scroll_bar->length = line * disassembly->debugger->glyph_size.y;
    disassembly->vertical_scroll_bar->scroll_rect.h = GetScrollBarHeight(*disassembly->vertical_scroll_bar);

    if(disassembly->vertical_scroll_bar->scroll_rect.h > 0.0f && disassembly->vertical_scroll_bar->scroll_rect.h < disassembly->vertical_scroll_bar->bar_rect.h){
        disassembly->vertical_scroll_bar->valid = true;
        SetYScrollBar(disassembly->vertical_scroll_bar,disassembly->vertical_scroll_bar->offset);
    }
    else{
        disassembly->vertical_scroll_bar->valid = false;
        disassembly->vertical_scroll_bar->offset = 0.0f;
        disassembly->vertical_scroll_bar->scroll_rect.y = disassembly->vertical_scroll_bar->bar_rect.y;
    }

    Disassembly_SetVisibleLines(disassembly);
}

void Disassembly_ConfigRect(DebuggerDisassembly *disassembly){

    disassembly->rect = disassembly->debugger->metrics.center.rect;
    
    disassembly->breakpoint_rect = (Rect){
        .x = 0,
        .y = 0,
        .w = ICON_SIZE,
        .h = disassembly->rect.h
    };

    disassembly->vertical_scroll_bar->bar_rect = (Rect){
        .x = disassembly->rect.x + disassembly->rect.w - SCROLL_BAR_SIZE,
        .y = disassembly->rect.y,
        .w = SCROLL_BAR_SIZE,
        .h = disassembly->rect.h
    };

    disassembly->vertical_scroll_bar->scroll_rect = (RectF){
        .x = disassembly->vertical_scroll_bar->bar_rect.x,
        .y = disassembly->vertical_scroll_bar->bar_rect.y,
        .w = SCROLL_BAR_SIZE,
        .h = 0.0f
    };

    Disassembly_ConfigVerticalScrollBar(disassembly);
}

void Disassembly_Event(DebuggerDisassembly *disassembly){
    
    //if(!disassembly->debugger->nes->paused) return;

    WG_Event event = disassembly->debugger->event;
    Vector2 mouse = disassembly->debugger->mouse;

    if(PointInRect(mouse,disassembly->vertical_scroll_bar->scroll_rect)){
        disassembly->vertical_scroll_bar->mouse.over = true;    
    }

    if(event.type == WG_MOUSEWHEEL && PointInRect(mouse,disassembly->rect)){
        
        if(event.mouse.wheel < 0.0f){
            SetYScrollBar(disassembly->vertical_scroll_bar,disassembly->vertical_scroll_bar->offset - (disassembly->debugger->glyph_size.y * 3));
        }
        else if(event.mouse.wheel > 0.0f){
            SetYScrollBar(disassembly->vertical_scroll_bar,disassembly->vertical_scroll_bar->offset + (disassembly->debugger->glyph_size.y * 3));
        }

        Disassembly_SetVisibleLines(disassembly);
    }
    else if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(disassembly->vertical_scroll_bar->mouse.over){

            disassembly->vertical_scroll_bar->mouse.pressed = true;

            disassembly->vertical_scroll_bar->mouse.diff = mouse.y - disassembly->vertical_scroll_bar->scroll_rect.y;
        }
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        disassembly->vertical_scroll_bar->mouse.pressed = false;
    }
    else if(WG_GetMouseState() & WG_MOUSEBUTTON_LEFT){

        if(disassembly->vertical_scroll_bar->mouse.pressed){

            HandleVerticalScrollBar(disassembly->vertical_scroll_bar,mouse);
            
            Disassembly_SetVisibleLines(disassembly);
        }
    }
}


static void Draw_ACU(DebuggerDisassembly *disassembly,Vector2 position){

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"A");

    // A
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,1,position,DEBUG_SYMBOL_COLOR);
}

static void Draw_IME(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"#$%.2X",Debug_CPU_Read8(disassembly->nes,address + 1));

    // #$00
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,4,position,DEBUG_VALUE_COLOR);
}

static void Draw_ABS(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){

    CPU *cpu = &disassembly->nes->cpu;

    uint16_t effective_address = Debug_CPU_Read16(disassembly->nes,address + 1);
    uint8_t value = Debug_CPU_Read8(disassembly->nes,address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.4X = $%.2X",effective_address,value);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,5,position,DEBUG_VALUE_COLOR);


    uint16_t *operation = cpu->instruction_table[Debug_CPU_Read8(disassembly->nes,address)].name;

    if(wcscmp(operation,L"JMP") && wcscmp(operation,L"JSR")){
        // =
        position.x += disassembly->debugger->glyph_size.x * 6;
        
        WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);
        
        // $00
        position.x += disassembly->debugger->glyph_size.x * 2;
        
        WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 8,3,position,DEBUG_VALUE_COLOR);
    }
}

static void Draw_ABX(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){

    CPU *cpu = &disassembly->nes->cpu;

    address = Debug_CPU_Read16(disassembly->nes,address + 1);
    uint16_t effective_address = address + cpu->x_register;
    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.4X,X [$%.4X] = $%.2X",address,effective_address,value);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,5,position,DEBUG_VALUE_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 5,1,position,OFF_COLOR);

    // X
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 8,1,position,DEBUG_SYMBOL_COLOR);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 9,5,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 16,1,position,DEBUG_SYMBOL_COLOR);

    //$00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 18,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_ABY(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){

    CPU *cpu = &disassembly->nes->cpu;

    address = Debug_CPU_Read16(disassembly->nes,address + 1);
    uint16_t effective_address = address + cpu->y_register;
    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.4X,Y [$%.4X] = $%.2X",address,effective_address,value);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,5,position,DEBUG_VALUE_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 5,1,position,OFF_COLOR);

    // Y
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 8,1,position,DEBUG_SYMBOL_COLOR);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 9,5,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 16,1,position,DEBUG_SYMBOL_COLOR);

    //$00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 18,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_IND(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
    
    uint16_t effective_address = Debug_CPU_Read16(disassembly->nes,address + 1);
    
    uint16_t lsb = Debug_CPU_Read8(disassembly->nes,effective_address);
    uint16_t msb = Debug_CPU_Read8(disassembly->nes,(effective_address & 0xFF00) | ((effective_address + 1) & 0xFF));
    
    effective_address = (msb << 8) | lsb;
                
    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"($%.4X)",effective_address);

    // (
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,1,position,DEBUG_SYMBOL_COLOR);

    //$0000
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 1,5,position,DEBUG_VALUE_COLOR);

    // )
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);
}

static void Draw_ZP0(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
    
    uint16_t effective_address = Debug_CPU_Read8(disassembly->nes,address + 1);
    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.2X = $%.2X",effective_address,value);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,3,position,DEBUG_VALUE_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 4;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 4,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_ZPX(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){

    CPU *cpu = &disassembly->nes->cpu;
    
    address = Debug_CPU_Read8(disassembly->nes,address + 1);
    uint16_t effective_address = (address + cpu->x_register) & 0xFF;
    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);
    
    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.2X,X [$%.2X] = $%.2X",address,effective_address,value);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,3,position,DEBUG_VALUE_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 3,1,position,OFF_COLOR);

    // X
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 4,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 7,3,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 10,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 12,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_ZPY(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
    
    CPU *cpu = &disassembly->nes->cpu;

    address = Debug_CPU_Read8(disassembly->nes,address + 1);
    uint16_t effective_address = (address + cpu->y_register) & 0xFF;
    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);
    
    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.2X,Y [$%.2X] = $%.2X",address,effective_address,value);

    // $00
    // $00
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,3,position,DEBUG_VALUE_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 3,1,position,OFF_COLOR);

    // Y
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 4,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 7,3,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 10,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 12,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_IDX(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
    
    CPU *cpu = &disassembly->nes->cpu;

    address = (Debug_CPU_Read8(disassembly->nes,address + 1) + cpu->x_register) & 0xFF;
                
    uint16_t lsb = Debug_CPU_Read8(disassembly->nes,address);
    uint16_t msb = Debug_CPU_Read8(disassembly->nes,(address + 1) & 0xFF);
                
    uint16_t effective_address = (msb << 8) | lsb;

    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"($%.2X,X) [$%.4X] = $%.2X",address,effective_address,value);

    // (
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 1,3,position,DEBUG_VALUE_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 4,1,position,OFF_COLOR);

    // X
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 5,1,position,DEBUG_SYMBOL_COLOR);

    // )
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 8,1,position,DEBUG_SYMBOL_COLOR);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 9,5,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 16,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 18,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_IDY(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
    
    CPU *cpu = &disassembly->nes->cpu;

    address = Debug_CPU_Read8(disassembly->nes,address + 1);
                
    uint16_t lsb = Debug_CPU_Read8(disassembly->nes,address & 0xFF);
    uint16_t msb = Debug_CPU_Read8(disassembly->nes,(address + 1) & 0xFF);
                
    uint16_t effective_address = ((msb << 8) | lsb) + cpu->y_register;

    uint8_t value = Debug_CPU_Read8(disassembly->nes,effective_address);

    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"($%.2X),Y [$%.4X] = $%.2X",address,effective_address,value);

    // (
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 1,3,position,DEBUG_VALUE_COLOR);

    // )
    position.x += disassembly->debugger->glyph_size.x * 3;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 4,1,position,DEBUG_SYMBOL_COLOR);

    // ,
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 5,1,position,OFF_COLOR);

    // Y
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 6,1,position,DEBUG_SYMBOL_COLOR);

    // [
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 8,1,position,DEBUG_SYMBOL_COLOR);

    // $0000
    position.x += disassembly->debugger->glyph_size.x * 1;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 9,5,position,DEBUG_VALUE_COLOR);

    // ]
    position.x += disassembly->debugger->glyph_size.x * 5;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 14,1,position,DEBUG_SYMBOL_COLOR);

    // =
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 16,1,position,DEBUG_SYMBOL_COLOR);

    // $00
    position.x += disassembly->debugger->glyph_size.x * 2;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 18,3,position,DEBUG_VALUE_COLOR);
}

static void Draw_REL(DebuggerDisassembly *disassembly,Vector2 position,uint16_t address){
                
    uint16_t effective_address = address + 2 + Debug_CPU_Read8(disassembly->nes,address + 1);
                
    swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"$%.4X",effective_address);

    //$0000
    position.x += disassembly->debugger->glyph_size.x * 0;

    WG_DrawNText(disassembly->debugger->renderer,disassembly->debugger->atlas,disassembly->debugger->buffer + 0,5,position,DEBUG_VALUE_COLOR);
}

void Disassembly_Draw(DebuggerDisassembly *disassembly){
    
    //if(!disassembly->debugger->nes->paused) return;

    WG_Renderer *renderer = disassembly->debugger->renderer;
    CPU *cpu = &disassembly->nes->cpu;
    uint32_t address = disassembly->metrics.start_address;
    uint32_t line = disassembly->metrics.start_line;
    uint32_t size = 0;
    uint8_t opcode = 0;
    Vector2 position;
    int y = 0;

    WG_RendererSetViewport(renderer,NULL,&disassembly->rect);

    while(address <= disassembly->metrics.end_address && line <= disassembly->metrics.end_line){

        y = line * disassembly->debugger->glyph_size.y + disassembly->vertical_scroll_bar->offset;

        if(address == cpu->program_counter){
            
            Rect rect = {
                .x = disassembly->breakpoint_rect.x + disassembly->breakpoint_rect.w * 0.5f - ICON_SIZE * 0.5f,
                .y = y + disassembly->debugger->glyph_size.y * 0.5f - ICON_SIZE * 0.5f,
                .w = ICON_SIZE,
                .h = ICON_SIZE
            };

            WG_BlitColor(renderer,disassembly->debugger->resources.breakpoint_arrow,NULL,&rect,(Color){255,255,0,255});
        }

        opcode = (address >= 0x8000 && address <= 0xFFFF) ? Debug_CPU_Read8(disassembly->nes,address) : 0x00;

        swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"%.4X",address);

        position.x = disassembly->breakpoint_rect.w;
        position.y = y;

        WG_DrawText(renderer,disassembly->debugger->atlas,disassembly->debugger->buffer,position,OFF_COLOR);

        //Bytes
        size = swprintf(disassembly->debugger->buffer,BUFFER_LENGTH,L"%.2X",opcode);

        if(cpu->instruction_table[opcode].bytes == 2){
            size += swprintf(
                disassembly->debugger->buffer + size,
                BUFFER_LENGTH - size,
                L" %.2X",
                Debug_CPU_Read8(disassembly->nes,address + 1)
            );
        }
        else if(cpu->instruction_table[opcode].bytes == 3){
            size += swprintf(
                disassembly->debugger->buffer + size,
                BUFFER_LENGTH - size,
                L" %.2X %.2X",
                Debug_CPU_Read8(disassembly->nes,address + 1),
                Debug_CPU_Read8(disassembly->nes,address + 2)
            );
        }

        position.x = disassembly->breakpoint_rect.w + disassembly->debugger->glyph_size.x * 5,
        position.y = y;

        WG_DrawText(renderer,disassembly->debugger->atlas,disassembly->debugger->buffer,position,ON_COLOR);


        //Instruction
        position.x = disassembly->breakpoint_rect.w + disassembly->debugger->glyph_size.x * 20;
        position.y = y;

        WG_DrawText(renderer,disassembly->debugger->atlas,cpu->instruction_table[opcode].name,position,DEBUG_INSTRUCTION_COLOR);


        if(cpu->instruction_table[opcode].address != IMP && wcscmp(cpu->instruction_table[opcode].name,L"NOP")){
            
            position.x = disassembly->breakpoint_rect.w + disassembly->debugger->glyph_size.x * 24;
            position.y = y;

            if(cpu->instruction_table[opcode].address == ACU){
                Draw_ACU(disassembly,position);
            }
            else if(cpu->instruction_table[opcode].address == IME){
                Draw_IME(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ABS){
                Draw_ABS(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ABX){
                Draw_ABX(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ABY){
                Draw_ABY(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == IND){
                Draw_IND(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ZP0){
                Draw_ZP0(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ZPX){
                Draw_ZPX(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == ZPY){
                Draw_ZPY(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == IDX){
                Draw_IDX(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == IDY){
                Draw_IDY(disassembly,position,address);
            }
            else if(cpu->instruction_table[opcode].address == REL){
                Draw_REL(disassembly,position,address);
            }

        }
        
        address += cpu->instruction_table[opcode].bytes;
        ++line;
    }

    WG_RendererSetViewport(renderer,NULL,NULL); 

    DrawScrollBar(disassembly->vertical_scroll_bar,renderer);
}

void Disassembly_Run(DebuggerDisassembly *disassembly){
    
    if(disassembly->update){

        disassembly->update = false;

        int line = 0;
        CPU *cpu = &disassembly->debugger->nes->cpu;

        if(cpu->program_counter <= 0x7FFF){
            line = cpu->program_counter;
        }
        else{
            int address = line = 0x8000;

            while(address < cpu->program_counter){
                address += cpu->instruction_table[Debug_CPU_Read8(disassembly->nes,address)].bytes;
                ++line;
            }
        }

        int y = line * disassembly->debugger->glyph_size.y + disassembly->vertical_scroll_bar->offset;

        if(y + disassembly->debugger->glyph_size.y < 0 || y > disassembly->rect.h){
            int offset = (-1 * line * disassembly->debugger->glyph_size.y) + (disassembly->rect.h * 0.5f - disassembly->debugger->glyph_size.y * 0.5f);
            SetYScrollBar(disassembly->vertical_scroll_bar,offset);
        }
        
        Disassembly_SetVisibleLines(disassembly);
    }

    Disassembly_Event(disassembly);

    Disassembly_Draw(disassembly);
}

void Disassembly_Free(DebuggerDisassembly *disassembly){
    free(disassembly->vertical_scroll_bar);
}