#include <Debugger/Breakpoints/BreakpointEdit/MemoryType.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Debugger/Breakpoints/BreakpointEdit/Address.h>
#include <Debugger/Breakpoints/BreakpointEdit/Metrics.h>
#include <ComboBox/ComboBox.h>
#include <Utils.h>

static void Configure(BreakpointEdit *breakpoint_edit){
    
    if(breakpoint_edit->memory_type.length > 0){
        swprintf(breakpoint_edit->buffer,BUFFER_LENGTH,L"(Max: $%X)",breakpoint_edit->memory_type.length - 1);
        UpdateLabel(breakpoint_edit->memory_type.max,breakpoint_edit->buffer);
    }
    else{
        swprintf(breakpoint_edit->buffer,BUFFER_LENGTH,L"Invalid Memory");
        UpdateLabel(breakpoint_edit->memory_type.max,breakpoint_edit->buffer);
    }

    if(breakpoint_edit->open && WG_WindowIsVisible(breakpoint_edit->window)){
        BreakpointEdit_ConfigRect(breakpoint_edit);
    }

    BreakpointEdit_Address_Validate(breakpoint_edit);
}

static void BreakpointEdit_CPU_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_CPU) return;

    breakpoint_edit->memory_type.type = MEMORY_CPU;
    breakpoint_edit->memory_type.length = 0x10000;

    breakpoint_edit->break_flags.execute.enabled = true;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_PPU_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_PPU) return;

    breakpoint_edit->memory_type.type = MEMORY_PPU;
    breakpoint_edit->memory_type.length = 0x4000;

    breakpoint_edit->break_flags.execute.enabled = false;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_PRGROM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;
    NES *nes = &breakpoint_edit->debugger_breakpoints->debugger->emulator->nes;

    uint32_t last_length = breakpoint_edit->memory_type.length;
    uint32_t current_length = nes->cartridge->prg_rom_banks * (16 * 1024);

    if(last_length == current_length && breakpoint_edit->memory_type.type == MEMORY_PRG_ROM) return;

    breakpoint_edit->memory_type.type = MEMORY_PRG_ROM;
    breakpoint_edit->memory_type.length = current_length;

    breakpoint_edit->break_flags.execute.enabled = true;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_SystemRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_SYSTEM_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_SYSTEM_RAM;
    breakpoint_edit->memory_type.length = 0x800;

    breakpoint_edit->break_flags.execute.enabled = true;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_WorkRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_WORK_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_WORK_RAM;
    breakpoint_edit->memory_type.length = 0x2000;

    breakpoint_edit->break_flags.execute.enabled = true;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_NameTableRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_NAMETABLE_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_NAMETABLE_RAM;
    breakpoint_edit->memory_type.length = 0x1000;

    breakpoint_edit->break_flags.execute.enabled = false;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_SpriteRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_SPRITE_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_SPRITE_RAM;
    breakpoint_edit->memory_type.length = 0x100;

    breakpoint_edit->break_flags.execute.enabled = false;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_PaletteRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_PALETTE_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_PALETTE_RAM;
    breakpoint_edit->memory_type.length = 0x20;

    breakpoint_edit->break_flags.execute.enabled = false;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_CHRROM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;
    NES *nes = &breakpoint_edit->debugger_breakpoints->debugger->emulator->nes;

    uint32_t last_length = breakpoint_edit->memory_type.length;
    uint32_t current_length = nes->cartridge->chr_rom_banks * (8 * 1024);

    if(last_length == current_length && breakpoint_edit->memory_type.type == MEMORY_CHR_ROM) return;

    breakpoint_edit->memory_type.type = MEMORY_CHR_ROM;
    breakpoint_edit->memory_type.length = current_length;

    breakpoint_edit->break_flags.execute.enabled = false;

    Configure(breakpoint_edit);
}

static void BreakpointEdit_CHRRAM_SetMemory(void *data){
    BreakpointEdit *breakpoint_edit = data;

    if(breakpoint_edit->memory_type.type == MEMORY_CHR_RAM) return;

    breakpoint_edit->memory_type.type = MEMORY_CHR_RAM;
    breakpoint_edit->memory_type.length = 0x2000;

    breakpoint_edit->break_flags.execute.enabled = false;
    
    Configure(breakpoint_edit);
}


void BreakpointEdit_MemoryType_Init(BreakpointEdit *breakpoint_edit){

    breakpoint_edit->memory_type.label = CreateLabel(L"Memory Type:",breakpoint_edit->atlas);

    breakpoint_edit->memory_type.combo_box = ComboBox_Create(
        breakpoint_edit->debugger_breakpoints->debugger->window_class,
        breakpoint_edit->window,
        breakpoint_edit->renderer,
        breakpoint_edit->resources.arrow,
        breakpoint_edit->atlas
    );

    breakpoint_edit->memory_type.max = CreateLabel(L"Invalid Memory",breakpoint_edit->atlas);

    breakpoint_edit->memory_type.cpu_memory = ComboBox_CreateString(L"CPU Memory",BreakpointEdit_CPU_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.cpu_memory);

    breakpoint_edit->memory_type.ppu_memory = ComboBox_CreateString(L"PPU Memory",BreakpointEdit_PPU_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.ppu_memory);

    breakpoint_edit->memory_type.prg_rom = ComboBox_CreateString(L"PRG ROM",BreakpointEdit_PRGROM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.prg_rom);

    breakpoint_edit->memory_type.system_ram = ComboBox_CreateString(L"System RAM",BreakpointEdit_SystemRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.system_ram);
    
    breakpoint_edit->memory_type.work_ram = ComboBox_CreateString(L"Work RAM",BreakpointEdit_WorkRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.work_ram);

    breakpoint_edit->memory_type.nametable_ram = ComboBox_CreateString(L"NameTable RAM",BreakpointEdit_NameTableRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.nametable_ram);

    breakpoint_edit->memory_type.sprite_ram = ComboBox_CreateString(L"Sprite RAM",BreakpointEdit_SpriteRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.sprite_ram);

    breakpoint_edit->memory_type.palette_ram = ComboBox_CreateString(L"Palette RAM",BreakpointEdit_PaletteRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.palette_ram);

    breakpoint_edit->memory_type.chr_rom = ComboBox_CreateString(L"CHR ROM",BreakpointEdit_CHRROM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.chr_rom);

    breakpoint_edit->memory_type.chr_ram = ComboBox_CreateString(L"CHR RAM",BreakpointEdit_CHRRAM_SetMemory,breakpoint_edit);
    ComboBox_InsertString(breakpoint_edit->memory_type.combo_box,breakpoint_edit->memory_type.chr_ram);

    BreakpointEdit_MemoryType_SetShowStrings(breakpoint_edit);

    ComboBox_SetDefaultElement(breakpoint_edit->memory_type.combo_box);
}

void BreakpointEdit_MemoryType_SetShowStrings(BreakpointEdit *breakpoint_edit){
    NES *nes = &breakpoint_edit->debugger_breakpoints->debugger->emulator->nes;

    if(nes->cartridge->prg_rom_banks){
        breakpoint_edit->memory_type.prg_rom->show = true;
    }
    else{
        breakpoint_edit->memory_type.prg_rom->show = false;
    }

    if(nes->cartridge->chr_rom_banks){
        breakpoint_edit->memory_type.chr_rom->show = true;
    }
    else{
        breakpoint_edit->memory_type.chr_rom->show = false;
    }
}

void BreakpointEdit_MemoryType_Free(BreakpointEdit *breakpoint_edit){
    DestroyLabel(breakpoint_edit->memory_type.label);
    ComboBox_Free(breakpoint_edit->memory_type.combo_box);
    DestroyLabel(breakpoint_edit->memory_type.max);
}