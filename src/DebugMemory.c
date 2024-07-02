#include <DebugMemory.h>
#include <NES/PPU.h>

void Debug_CPU_WriteMemory(NES *nes,uint16_t address,uint8_t value){
    //CPU RAM
    if(address >= 0x0000 && address <= 0x1FFF){
        nes->cpu.ram[address & 0x07FF] = value;
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
        nes->mapper.wram[address & 0x1FFF] = value;
    }
    //Cartridge ROM
    else if(address >= 0x8000 && address <= 0xFFFF){
        nes->mapper.rom[address & 0x7FFF] = value;
    }
}

uint8_t Debug_CPU_ReadMemory(NES *nes,uint16_t address){

    //RAM
    if(address >= 0x0000 && address <= 0x1FFF){
        return nes->cpu.ram[address & 0x7FF];
    }
    //PPU Registers
    else if(address >= 0x2000 && address <= 0x3FFF){
        
        uint8_t value = 0x00;

        switch(address & 0x07){
            //PPU Control
            case 0x00:
                value = nes->ppu.bus;
                break;
            //PPU Mask
            case 0x01:
                value = nes->ppu.bus;
                break;
            //PPU Status
            case 0x02:
                value = (nes->ppu.status & 0xE0) | (nes->ppu.bus & 0x1F);
                break;
            //OAM Address
            case 0x03:
                value = nes->ppu.bus;
                break;
            //OAM Data
            case 0x04:
                value = nes->ppu.oam[nes->ppu.oam_addr];
                break;
            //PPU Scroll
            case 0x05:
                value = nes->ppu.bus;
                break;
            //PPU Address
            case 0x06:
                value = nes->ppu.bus;
                break;
            //PPU Data
            case 0x07:
                if(nes->ppu.scrolling.vram >= 0x3F00){
                    value = nes->ppu.palette[nes->ppu.scrolling.vram % 0x20];
                    if(nes->ppu.mask & GRAYSCALE){
                        value &= 0x30;
                    }
                }
                else{
                    value = nes->ppu.buffer;
                }
                break;
        }

        return value;
    }
    //APU and I/O Registers
    else if(address >= 0x4000 && address <= 0x4017){
        return 0x00;
    }
    //Unmapped
    else if(address >= 0x4018 && address <= 0x5FFF){
        return 0x00;
    }
    //PRG RAM
    else if(address >= 0x6000 && address <= 0x7FFF){
        return nes->mapper.wram[address & 0x1FFF];
    }
    //PRG ROM
    else if(address >= 0x8000 && address <= 0xFFFF){
        return nes->mapper.rom[address & 0x7FFF];
    }

    return 0;
}