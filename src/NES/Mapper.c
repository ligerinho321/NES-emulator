#include <NES/Mapper.h>

static void PRG_Write(Mapper *mapper,uint8_t value,uint16_t address){return;}

static uint8_t PRG_Read(Mapper *mapper,uint16_t address){return 0;}

static void CHR_Write(Mapper *mapper,uint8_t value,uint16_t address){return;}

static uint8_t CHR_Read(Mapper *mapper,uint16_t adderess){return 0;}

static void Reset(Mapper *mapper){return;}

static void IRQ(Mapper *mapper){return;}

static void VramAddressChange(Mapper *mapper,uint16_t address){return;}


static void Clear(Mapper *mapper){
   
    memset(mapper->pattern_table,0,sizeof(uint8_t) * 0x2000);
    memset(mapper->wram,0,sizeof(uint8_t) * 0x2000);
    memset(mapper->rom,0,sizeof(uint8_t) * 0x8000);

    free(mapper->extension);

    mapper->cartridge = NULL;
    mapper->extension = NULL;
    mapper->mirroring = 0;

    mapper->prg_write = PRG_Write;
    mapper->prg_read = PRG_Read;
    mapper->chr_write = CHR_Write;
    mapper->chr_read = CHR_Read;
    mapper->reset = Reset;
    mapper->vram_address_change = VramAddressChange;
}


void Mapper_Init(Mapper *mapper,NES *nes){
    mapper->nes = nes;
}

bool Mapper_InsertCartridge(Mapper *mapper,Cartridge *cartridge){

    Clear(mapper);

    if(cartridge == NULL) return false;

    switch(cartridge->mapper_type){
        case MAPPER_NROM:
            NROM_Init(mapper);
            break;
        case MAPPER_MMC1:
            MMC1_Init(mapper);
            break;
        case MAPPER_UXROM:
            UXROM_Init(mapper);
            break;
        case MAPPER_CNROM:
            CNROM_Init(mapper);
            break;
        case MAPPER_MMC3:
            MMC3_Init(mapper);
            break;
        case MAPPER_AXROM:
            AXROM_Init(mapper);
            break;
        default:
            fprintf(stderr,"Error: Mapper %d not implemented\n",cartridge->mapper_type);    
            return false;
    }

    mapper->cartridge = cartridge;
    mapper->mirroring = mapper->cartridge->mirroring;
    mapper->type = mapper->cartridge->mapper_type;

    return true;
}

void Mapper_Free(Mapper *mapper){
    free(mapper->extension);
}