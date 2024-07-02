#include <NES/Mapper.h>

static void PRG_Write(Mapper *mapper,uint8_t value,uint16_t address){
    if(address >= 0x6000 && address <= 0x7FFF){
        //Does not do anything
    }
    else if(address >= 0x8000 && address <= 0xFFFF){
        //switch pattern table bank
        memcpy(mapper->pattern_table,mapper->cartridge->chr_rom_pointer + (value & 0x03) * 0x2000,sizeof(uint8_t) * 0x2000);
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
    mapper->pattern_table[address & 0x1FFF] = value;
}

static uint8_t CHR_Read(Mapper *mapper,uint16_t address){
    return mapper->pattern_table[address & 0x1FFF];
}


static void Reset(Mapper *mapper){

    if(mapper->cartridge->prg_rom_banks){
        //fisrt bank
        memcpy(mapper->pattern_table,mapper->cartridge->chr_rom_pointer,sizeof(uint8_t) * 0x2000);
    }

    if(mapper->cartridge->prg_rom_banks){
        //fist bank
        memcpy(mapper->rom,mapper->cartridge->prg_rom_pointer,sizeof(uint8_t) * 0x4000);
        //second bank
        memcpy(mapper->rom + 0x4000,mapper->cartridge->prg_rom_pointer + (mapper->cartridge->prg_rom_banks - 1) * 0x4000,sizeof(uint8_t) * 0x4000);
    }
}


void CNROM_Init(Mapper *mapper){
    mapper->prg_write = PRG_Write;
    mapper->prg_read = PRG_Read;
    mapper->chr_write = CHR_Write;
    mapper->chr_read = CHR_Read;
    mapper->reset = Reset;
}