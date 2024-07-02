#include <NES/Mapper.h>

static void PRG_Write(Mapper *mapper,uint8_t value,uint16_t address){
    if(address >= 0x6000 && address <= 0x7FFF){
        //Does not do anything
    }
    else if(address >= 0x8000 && address <= 0xFFFF){

        memcpy(mapper->rom,mapper->cartridge->prg_rom_pointer + (value & 0x07) * 0x8000,sizeof(uint8_t) * 0x8000);

        if(value & BIT_4){
            mapper->mirroring = MIRRORING_ONE_SCREEN_LOWER;
        }
        else{
            mapper->mirroring = MIRRORING_ONE_SCREEN_UPPER;
        }
    }
}

static uint8_t PRG_Read(Mapper *mapper,uint16_t address){
    if(address >= 0x6000 && address <= 0x7FFF){
        
        return mapper->nes->cpu.bus;
    }
    else if(address >= 0x8000 && address <= 0xFFFF){

        uint8_t value = mapper->rom[address & 0x7FFF];

        mapper->nes->cpu.bus = value;

        return value;
    }
    
    return mapper->nes->cpu.bus;
}

static void CHR_Write(Mapper *mapper,uint8_t value,uint16_t address){
    if(address >= 0x0000 && address <= 0x2000){
        mapper->pattern_table[address & 0x1FFF] = value;
    }
}

static uint8_t CHR_Read(Mapper *mapper,uint16_t address){
    if(address >= 0x0000 && address <= 0x2000){
        return mapper->pattern_table[address & 0x1FFF];
    }
    return 0;
}


static void Reset(Mapper *mapper){

    if(mapper->cartridge->chr_rom_banks){
        memcpy(mapper->pattern_table,mapper->cartridge->chr_rom_pointer,sizeof(uint8_t) * 0x2000);
    }

    if(mapper->cartridge->prg_rom_banks){
        memcpy(mapper->rom,mapper->cartridge->prg_rom_pointer,sizeof(uint8_t) * 0x8000);
    }
}


void AXROM_Init(Mapper *mapper){
    mapper->prg_write = PRG_Write;
    mapper->prg_read = PRG_Read;
    mapper->chr_write = CHR_Write;
    mapper->chr_read = CHR_Read;
    mapper->reset = Reset;
}