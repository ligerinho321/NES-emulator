#include <MemoryViewer/MemoryType.h>
#include <MemoryViewer/MemoryViewer_Utils.h>
#include <ScrollBar.h>
#include <Input.h>
#include <ComboBox/ComboBox.h>
#include <Utils.h>

static void Configure(MemoryViewer *memory_viewer){

    memory_viewer->vertical_scroll_bar->offset = 0;

    memory_viewer->memory.address_row_length = swprintf(memory_viewer->buffer,BUFFER_LENGTH,L"%X",memory_viewer->memory_type.length - 1);
    memory_viewer->memory.address_row_width = (memory_viewer->memory.address_row_length + 2) * memory_viewer->glyph_size.x;

    memory_viewer->memory.input.address = 0;
    memory_viewer->memory.input.row = 0;
    memory_viewer->memory.input.column = 0;
    memory_viewer->memory.input.editing = false;

    memory_viewer->memory_address.input->max_length = memory_viewer->memory.address_row_length;
    GapBufferClear(memory_viewer->memory_address.input->gap_buffer);
    GapBufferInsertValue(memory_viewer->memory_address.input->gap_buffer,uint16_t,L'0');

    Memory_ConfigRect(memory_viewer);
}


static uint8_t CPU_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){

    uint8_t value = 0x00;

    //CPU RAM
    if(address >= 0x0000 && address <= 0x1FFF){
        value = memory_viewer->nes->cpu.ram[address & 0x07FF];
    }
    //PPU Registers
    else if(address >= 0x2000 && address <= 0x3FFF){
        
        switch(address & 0x07){
            //PPU Control
            case 0x00:
                value = memory_viewer->nes->ppu.control;
                break;
            //PPU Mask
            case 0x01:
                value = memory_viewer->nes->ppu.mask;
                break;
            //PPU Status
            case 0x02:
                value = (memory_viewer->nes->ppu.status & 0xE0) | (memory_viewer->nes->ppu.bus & 0x1F);
                break;
            //OAM Address
            case 0x03:
                value = memory_viewer->nes->ppu.oam_addr;
                break;
            //OAM Data
            case 0x04:
                value = memory_viewer->nes->ppu.oam[memory_viewer->nes->ppu.oam_addr];
                break;
            //PPU Scroll
            case 0x05:
                value = memory_viewer->nes->ppu.scroll;
                break;
            //PPU Address
            case 0x06:
                value = memory_viewer->nes->ppu.address;
                break;
            //PPU Data
            case 0x07:
                value = memory_viewer->nes->ppu.data;
                break;
        }
    }
    //APU, I/O Registers
    else if(address >= 0x4000 && address <= 0x4017){
        value = 0xFF;
    }
    //Unmapped
    else if(address >= 0x4018 && address <= 0x5FFF){
        value = memory_viewer->nes->cpu.bus;
    }
    //Cartridge RAM
    else if(address >= 0x6000 && address <= 0x7FFF){
        value = memory_viewer->nes->mapper.wram[address & 0x1FFF];
    }
    //Cartridge ROM
    else if(address >= 0x8000 && address <= 0xFFFF){
        value = memory_viewer->nes->mapper.rom[address & 0x7FFF];
    }

    return value;
}

static void CPU_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){

    //CPU RAM
    if(address >= 0x0000 && address <= 0x1FFF){
        memory_viewer->nes->cpu.ram[address & 0x07FF] = value;
    }
    //PPU Registers
    else if(address >= 0x2000 && address <= 0x3FFF){
        //Does not do anything
    }
    //APU, I/O Registers
    else if(address >= 0x4000 && address <= 0x4017){
        //Does not do anything
    }
    //Unmapped
    else if(address >= 0x4018 && address <= 0x5FFF){
        //Does not do anything
    }
    //Cartridge RAM
    else if(address >= 0x6000 && address <= 0x7FFF){
        memory_viewer->nes->mapper.wram[address & 0x1FFF] = value;
    }
    //Cartridge ROM
    else if(address >= 0x8000 && address <= 0xFFFF){
        memory_viewer->nes->mapper.rom[address & 0x7FFF] = value;
    }
}

static void CPU_SetMemory(void *param){

    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_CPU) return;

    memory_viewer->memory_type.type = MEMORY_CPU;
    memory_viewer->memory_type.length = 0x10000;

    memory_viewer->memory.read = CPU_ReadMemory;
    memory_viewer->memory.write = CPU_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t PPU_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    
    PPU *ppu = &memory_viewer->nes->ppu;

    //PatternTables
    if(address >= 0x0000 && address <= 0x1FFF){
        return memory_viewer->nes->mapper.pattern_table[address & 0x1FFF];
    }
    //NameTables
    else if(address >= 0x2000 && address <= 0x3EFF){
        
        address &= 0x0FFF;

        uint8_t value = 0x00;

        switch(memory_viewer->nes->mapper.mirroring){

            case MIRRORING_VERTICAL:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    value = ppu->name_tables[0][address & 0x03FF];
                }
                //B
                else if(address >= 0x0400 && address <= 0x07FF){
                    value = ppu->name_tables[1][address & 0x03FF];
                }
                //A
                else if(address >= 0x0800 && address <= 0x0BFF){
                    value = ppu->name_tables[0][address & 0x03FF];
                }
                //B
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    value = ppu->name_tables[1][address & 0x03FF];
                }
                break;
            case MIRRORING_HORIZONTAL:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    value = ppu->name_tables[0][address & 0x03FF];
                }
                //A 
                else if(address >= 0x0400 && address <= 0x07FF){
                    value = ppu->name_tables[0][address & 0x03FF];
                }
                //B
                else if(address >= 0x0800 && address <= 0x0BFF){
                    value = ppu->name_tables[1][address & 0x03FF];
                }
                //B
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    value = ppu->name_tables[1][address & 0x03FF];
                }
                break;
            case MIRRORING_FOUR_SCREEN:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    value = ppu->name_tables[0][address & 0x03FF];
                }
                //B
                else if(address >= 0x0400 && address <= 0x07FF){
                    value = ppu->name_tables[1][address & 0x03FF];
                }
                //C
                else if(address >= 0x0800 && address <= 0x0BFF){
                    value = ppu->name_tables[2][address & 0x03FF];
                }
                //D
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    value = ppu->name_tables[3][address & 0x03FF];
                }
                break;
            case MIRRORING_ONE_SCREEN:
            case MIRRORING_ONE_SCREEN_LOWER:
                value = ppu->name_tables[0][address & 0x03FF];
                break;
            case MIRRORING_ONE_SCREEN_UPPER:
                value = ppu->name_tables[1][address & 0x03FF];
                break;
        }

        return value;
    }
    //Palette
    else if(address >= 0x3F00 && address <= 0x3FFF){
        return ppu->palette[address % 0x20];
    }
}

static void PPU_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    PPU *ppu = &memory_viewer->nes->ppu;

    //PatternTables
    if(address >= 0x0000 && address <= 0x1FFF){
        memory_viewer->nes->mapper.pattern_table[address & 0x1FFF] = value;
    }
    //NameTables
    else if(address >= 0x2000 && address <= 0x3EFF){
        
        address &= 0x0FFF;

        switch(memory_viewer->nes->mapper.mirroring){

            case MIRRORING_VERTICAL:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    ppu->name_tables[0][address & 0x03FF] = value;
                }
                //B
                else if(address >= 0x0400 && address <= 0x07FF){
                    ppu->name_tables[1][address & 0x03FF] = value;
                }
                //A
                else if(address >= 0x0800 && address <= 0x0BFF){
                    ppu->name_tables[0][address & 0x03FF] = value;
                }
                //B
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    ppu->name_tables[1][address & 0x03FF] = value;
                }
                break;
            case MIRRORING_HORIZONTAL:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    ppu->name_tables[0][address & 0x03FF] = value;
                }
                //A 
                else if(address >= 0x0400 && address <= 0x07FF){
                    ppu->name_tables[0][address & 0x03FF] = value;
                }
                //B
                else if(address >= 0x0800 && address <= 0x0BFF){
                    ppu->name_tables[1][address & 0x03FF] = value;
                }
                //B
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    ppu->name_tables[1][address & 0x03FF] = value;
                }
                break;
            case MIRRORING_FOUR_SCREEN:
                //A
                if(address >= 0x0000 && address <= 0x03FF){
                    ppu->name_tables[0][address & 0x03FF] = value;
                }
                //B
                else if(address >= 0x0400 && address <= 0x07FF){
                    ppu->name_tables[1][address & 0x03FF] = value;
                }
                //C
                else if(address >= 0x0800 && address <= 0x0BFF){
                    ppu->name_tables[2][address & 0x03FF] = value;
                }
                //D
                else if(address >= 0x0C00 && address <= 0x0FFF){
                    ppu->name_tables[3][address & 0x03FF] = value;
                }
                break;
            case MIRRORING_ONE_SCREEN:
            case MIRRORING_ONE_SCREEN_LOWER:
                ppu->name_tables[0][address & 0x03FF] = value;
                break;
            case MIRRORING_ONE_SCREEN_UPPER:
                ppu->name_tables[1][address & 0x03FF] = value;
                break;
        }
    }
    //Palette
    else if(address >= 0x3F00 && address <= 0x3FFF){
        ppu->palette[address % 0x20] = value;
    }
}

static void PPU_SetMemory(void *param){

    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_PPU) return;

    memory_viewer->memory_type.type = MEMORY_PPU;
    memory_viewer->memory_type.length = 0x4000;

    memory_viewer->memory.read = PPU_ReadMemory;
    memory_viewer->memory.write = PPU_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t PRGROM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    uint8_t value = 0;
    WaitForSingleObject(memory_viewer->mutex,INFINITE);
    if(memory_viewer->memory_type.length && memory_viewer->nes->cartridge){
        value = memory_viewer->nes->cartridge->prg_rom_pointer[address];
    }
    ReleaseMutex(memory_viewer->mutex);
    return value;
}

static void PRGROM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    WaitForSingleObject(memory_viewer->mutex,INFINITE);
    if(memory_viewer->memory_type.length && memory_viewer->nes->cartridge){
        memory_viewer->nes->cartridge->prg_rom_pointer[address] = value;
    }
    ReleaseMutex(memory_viewer->mutex);
}

static void PRGROM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    uint32_t last_length = memory_viewer->memory_type.length;
    uint32_t current_length = memory_viewer->nes->cartridge->prg_rom_banks * (16 * 1024);

    if(last_length == current_length && memory_viewer->memory_type.type == MEMORY_PRG_ROM) return;

    memory_viewer->memory_type.type = MEMORY_PRG_ROM;
    memory_viewer->memory_type.length = current_length;

    memory_viewer->memory.read = PRGROM_ReadMemory;
    memory_viewer->memory.write = PRGROM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t SystemRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    return memory_viewer->nes->cpu.ram[address & 0x7FF];
}

static void SystemRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    memory_viewer->nes->cpu.ram[address & 0x7FF] = value;
}

static void SystemRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_SYSTEM_RAM) return;

    memory_viewer->memory_type.type = MEMORY_SYSTEM_RAM;
    memory_viewer->memory_type.length = 0x800;

    memory_viewer->memory.read = SystemRAM_ReadMemory;
    memory_viewer->memory.write = SystemRAM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t WorkRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    return memory_viewer->nes->mapper.wram[address & 0x1FFF];
}

static void WorkRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    memory_viewer->nes->mapper.wram[address & 0x1FFF] = value;
}

static void WorkRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_WORK_RAM) return;

    memory_viewer->memory_type.type = MEMORY_WORK_RAM;
    memory_viewer->memory_type.length = 0x2000;

    memory_viewer->memory.read = WorkRAM_ReadMemory;
    memory_viewer->memory.write = WorkRAM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t NameTableRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    
    PPU *ppu = &memory_viewer->nes->ppu;

    address &= 0x0FFF;

    uint8_t value = 0x00;

    switch(memory_viewer->nes->mapper.mirroring){

        case MIRRORING_VERTICAL:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                value = ppu->name_tables[0][address & 0x03FF];
            }
            //B
            else if(address >= 0x0400 && address <= 0x07FF){
                value = ppu->name_tables[1][address & 0x03FF];
            }
            //A
            else if(address >= 0x0800 && address <= 0x0BFF){
                value = ppu->name_tables[0][address & 0x03FF];
            }
            //B
            else if(address >= 0x0C00 && address <= 0x0FFF){
                value = ppu->name_tables[1][address & 0x03FF];
            }
            break;
        case MIRRORING_HORIZONTAL:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                value = ppu->name_tables[0][address & 0x03FF];
            }
            //A 
            else if(address >= 0x0400 && address <= 0x07FF){
                value = ppu->name_tables[0][address & 0x03FF];
            }
            //B
            else if(address >= 0x0800 && address <= 0x0BFF){
                value = ppu->name_tables[1][address & 0x03FF];
            }
            //B
            else if(address >= 0x0C00 && address <= 0x0FFF){
                value = ppu->name_tables[1][address & 0x03FF];
            }
            break;
        case MIRRORING_FOUR_SCREEN:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                value = ppu->name_tables[0][address & 0x03FF];
            }
            //B
            else if(address >= 0x0400 && address <= 0x07FF){
                value = ppu->name_tables[1][address & 0x03FF];
            }
            //C
            else if(address >= 0x0800 && address <= 0x0BFF){
                value = ppu->name_tables[2][address & 0x03FF];
            }
            //D
            else if(address >= 0x0C00 && address <= 0x0FFF){
                value = ppu->name_tables[3][address & 0x03FF];
            }
            break;
        case MIRRORING_ONE_SCREEN:
        case MIRRORING_ONE_SCREEN_LOWER:
            value = ppu->name_tables[0][address & 0x03FF];
            break;
        case MIRRORING_ONE_SCREEN_UPPER:
            value = ppu->name_tables[1][address & 0x03FF];
            break;
    }

    return value;
}

static void NameTableRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    PPU *ppu = &memory_viewer->nes->ppu;

    address &= 0x0FFF;

    switch(memory_viewer->nes->mapper.mirroring){

        case MIRRORING_VERTICAL:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                ppu->name_tables[0][address & 0x03FF] = value;
            }
            //B
            else if(address >= 0x0400 && address <= 0x07FF){
                ppu->name_tables[1][address & 0x03FF] = value;
            }
            //A
            else if(address >= 0x0800 && address <= 0x0BFF){
                ppu->name_tables[0][address & 0x03FF] = value;
            }
            //B
            else if(address >= 0x0C00 && address <= 0x0FFF){
                ppu->name_tables[1][address & 0x03FF] = value;
            }
            break;
        case MIRRORING_HORIZONTAL:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                ppu->name_tables[0][address & 0x03FF] = value;
            }
            //A 
            else if(address >= 0x0400 && address <= 0x07FF){
                ppu->name_tables[0][address & 0x03FF] = value;
            }
            //B
            else if(address >= 0x0800 && address <= 0x0BFF){
                ppu->name_tables[1][address & 0x03FF] = value;
            }
            //B
            else if(address >= 0x0C00 && address <= 0x0FFF){
                ppu->name_tables[1][address & 0x03FF] = value;
            }
            break;
        case MIRRORING_FOUR_SCREEN:
            //A
            if(address >= 0x0000 && address <= 0x03FF){
                ppu->name_tables[0][address & 0x03FF] = value;
            }
            //B
            else if(address >= 0x0400 && address <= 0x07FF){
                ppu->name_tables[1][address & 0x03FF] = value;
            }
            //C
            else if(address >= 0x0800 && address <= 0x0BFF){
                ppu->name_tables[2][address & 0x03FF] = value;
            }
            //D
            else if(address >= 0x0C00 && address <= 0x0FFF){
                ppu->name_tables[3][address & 0x03FF] = value;
            }
            break;
        case MIRRORING_ONE_SCREEN:
        case MIRRORING_ONE_SCREEN_LOWER:
            ppu->name_tables[0][address & 0x03FF] = value;
            break;
        case MIRRORING_ONE_SCREEN_UPPER:
            ppu->name_tables[1][address & 0x03FF] = value;
            break;
    }
}

static void NameTableRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_NAMETABLE_RAM) return;

    memory_viewer->memory_type.type = MEMORY_NAMETABLE_RAM;
    memory_viewer->memory_type.length = 0x1000;

    memory_viewer->memory.read = NameTableRAM_ReadMemory;
    memory_viewer->memory.write = NameTableRAM_WriteMemory;
    
    Configure(memory_viewer);
}


static uint8_t SpriteRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    return memory_viewer->nes->ppu.oam[address & 0xFF];
}

static void SpriteRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    memory_viewer->nes->ppu.oam[address & 0xFF] = value;
}

static void SpriteRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_SPRITE_RAM) return;

    memory_viewer->memory_type.type = MEMORY_SPRITE_RAM;
    memory_viewer->memory_type.length = 0x100;

    memory_viewer->memory.read = SpriteRAM_ReadMemory;
    memory_viewer->memory.write = SpriteRAM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t SecondaryOAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    address &= 0x1F;
    uint8_t value = 0x00;

    switch(address % 4){
        case 0:
            value = memory_viewer->nes->ppu.sprites[address/4].y;        
            break;
        case 1:
            value = memory_viewer->nes->ppu.sprites[address/4].index;
            break;
        case 2:
            value = memory_viewer->nes->ppu.sprites[address/4].attributes;
            break;
        case 3:
            value = memory_viewer->nes->ppu.sprites[address/4].x;
            break;
    }

    return value;
}

static void SecondaryOAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    address &= 0x1F;

    switch(address % 4){
        case 0:
            memory_viewer->nes->ppu.sprites[address/4].y = value;        
            break;
        case 1:
            memory_viewer->nes->ppu.sprites[address/4].index = value;
            break;
        case 2:
            memory_viewer->nes->ppu.sprites[address/4].attributes = value;
            break;
        case 3:
            memory_viewer->nes->ppu.sprites[address/4].x = value;
            break;
    }
}

static void SecondaryOAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_SECONDARY_OAM) return;
    
    memory_viewer->memory_type.type = MEMORY_SECONDARY_OAM;
    memory_viewer->memory_type.length = 0x20;

    memory_viewer->memory.read = SecondaryOAM_ReadMemory;
    memory_viewer->memory.write = SecondaryOAM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t PaletteRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    return memory_viewer->nes->ppu.palette[address & 0x1F];
}

static void PaletteRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    memory_viewer->nes->ppu.palette[address & 0x1F] = value;
}

static void PaletteRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_PALETTE_RAM) return;

    memory_viewer->memory_type.type = MEMORY_PALETTE_RAM;
    memory_viewer->memory_type.length = 0x20;

    memory_viewer->memory.read = PaletteRAM_ReadMemory;
    memory_viewer->memory.write = PaletteRAM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t CHRROM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    uint8_t value = 0;
    WaitForSingleObject(memory_viewer->mutex,INFINITE);
    if(memory_viewer->memory_type.length && memory_viewer->nes->cartridge){
        value = memory_viewer->nes->cartridge->chr_rom_pointer[address];
    }
    ReleaseMutex(memory_viewer->mutex);
    return value;
}

static void CHRROM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    WaitForSingleObject(memory_viewer->mutex,INFINITE);
    if(memory_viewer->memory_type.length && memory_viewer->nes->cartridge){
        memory_viewer->nes->cartridge->chr_rom_pointer[address] = value;
    }
    ReleaseMutex(memory_viewer->mutex);
}

static void CHRROM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    uint32_t last_length = memory_viewer->memory_type.length;
    uint32_t current_length = memory_viewer->nes->cartridge->chr_rom_banks * (8 * 1024);

    if(last_length == current_length && memory_viewer->memory_type.type == MEMORY_CHR_ROM) return;

    memory_viewer->memory_type.type = MEMORY_CHR_ROM;
    memory_viewer->memory_type.length = current_length;

    memory_viewer->memory.read = CHRROM_ReadMemory;
    memory_viewer->memory.write = CHRROM_WriteMemory;

    Configure(memory_viewer);
}


static uint8_t CHRRAM_ReadMemory(MemoryViewer *memory_viewer,uint32_t address){
    return memory_viewer->nes->mapper.pattern_table[address & 0x1FFF];
}

static void CHRRAM_WriteMemory(MemoryViewer *memory_viewer,uint8_t value,uint32_t address){
    memory_viewer->nes->mapper.pattern_table[address & 0x1FFF] = value;
}

static void CHRRAM_SetMemory(void *param){
    MemoryViewer *memory_viewer = param;

    if(memory_viewer->memory_type.type == MEMORY_CHR_RAM) return;

    memory_viewer->memory_type.type = MEMORY_CHR_RAM;
    memory_viewer->memory_type.length = 0x2000;

    memory_viewer->memory.read = CHRRAM_ReadMemory;
    memory_viewer->memory.write = CHRRAM_WriteMemory;

    Configure(memory_viewer);
}


void MemoryViewer_MemoryType_Init(MemoryViewer *memory_viewer){

    memory_viewer->memory_type.label = CreateLabel(L"Memory Type:",memory_viewer->atlas);

    memory_viewer->memory_type.combo_box = ComboBox_Create(
        memory_viewer->emulator->window_class,
        memory_viewer->window,
        memory_viewer->renderer,
        memory_viewer->arrow,
        memory_viewer->atlas
    );

    memory_viewer->memory_type.cpu_memory = ComboBox_CreateString(L"CPU Memory",CPU_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.cpu_memory);

    memory_viewer->memory_type.ppu_memory = ComboBox_CreateString(L"PPU Memory",PPU_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.ppu_memory);

    memory_viewer->memory_type.prg_rom = ComboBox_CreateString(L"PRG ROM",PRGROM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.prg_rom);

    memory_viewer->memory_type.system_ram = ComboBox_CreateString(L"System RAM",SystemRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.system_ram);

    memory_viewer->memory_type.work_ram = ComboBox_CreateString(L"Work RAM",WorkRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.work_ram);

    memory_viewer->memory_type.nametable_ram = ComboBox_CreateString(L"NameTable RAM",NameTableRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.nametable_ram);

    memory_viewer->memory_type.sprite_ram = ComboBox_CreateString(L"Sprite RAM",SpriteRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.sprite_ram);

    memory_viewer->memory_type.secondary_oam = ComboBox_CreateString(L"Secondary OAM",SecondaryOAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.secondary_oam);

    memory_viewer->memory_type.palette_ram = ComboBox_CreateString(L"Palette RAM",PaletteRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.palette_ram);

    memory_viewer->memory_type.chr_rom = ComboBox_CreateString(L"CHR ROM",CHRROM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.chr_rom);

    memory_viewer->memory_type.chr_ram = ComboBox_CreateString(L"CHR RAM",CHRRAM_SetMemory,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.chr_ram);

    MemoryViewer_MemoryType_SetShowStrings(memory_viewer);

    ComboBox_SetCurrentElement(memory_viewer->memory_type.combo_box,memory_viewer->memory_type.cpu_memory);
}

void MemoryViewer_MemoryType_SetShowStrings(MemoryViewer *memory_viewer){

    if(memory_viewer->nes->cartridge->prg_rom_banks){
        memory_viewer->memory_type.prg_rom->show = true;
    }
    else{
        memory_viewer->memory_type.prg_rom->show = false;
    }

    if(memory_viewer->nes->cartridge->chr_rom_banks){
        memory_viewer->memory_type.chr_rom->show = true;
    }
    else{
        memory_viewer->memory_type.chr_rom->show = false;
    }
}

void MemoryViewer_MemoryType_Free(MemoryViewer *memory_viewer){
    DestroyLabel(memory_viewer->memory_type.label);
    ComboBox_Free(memory_viewer->memory_type.combo_box);
}