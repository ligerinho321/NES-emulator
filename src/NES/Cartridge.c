#include <NES/Cartridge.h>

//CPU/PPU timing
static const char *cpu_ppu_timing[0x04] = {
 /* 00 */ "NTSC",
 /* 01 */ "PAL",
 /* 02 */ "Dual",
 /* 03 */ "Dendy"
};

//Extended console type
static const char *extended_console_type[0x10] = {
 /* 00 */ "Regular NES/Famicom/Dendy",
 /* 01 */ "Nintendo Vs. System",
 /* 02 */ "Playchoice 10",
 /* 03 */ "Regular Famiclone, but with CPU that supports Decimal Mode",
 /* 04 */ "Regular NES/Famicom with EPSM module or plug-through cartridge",
 /* 05 */ "V.R. Technology VT01 with red/cyan STN palette",
 /* 06 */ "V.R. Technology VT02",
 /* 07 */ "V.R. Technology VT03",
 /* 08 */ "V.R. Technology VT09",
 /* 09 */ "V.R. Technology VT32",
 /* 0A */ "V.R. Technology VT369",
 /* 0B */ "UMC UM6578",
 /* 0C */ "Famicom Network System",
 /* 0D */ "Reserved",
 /* 0E */ "Reserved",
 /* 0F */ "Reserved"
};

//Vs. PPU types
static const char *vs_ppu_types[0x10] = {
 /* 00 */ "RP2C03B",
 /* 01 */ "RP2C03G",
 /* 02 */ "RP2C04-0001",
 /* 03 */ "RP2C04-0002",
 /* 04 */ "RP2C04-0003",
 /* 05 */ "RP2C04-0004",
 /* 06 */ "RC2C03B",
 /* 07 */ "RC2C03C",
 /* 08 */ "RC2C05-01 ($2002 AND $?? =$1B)",
 /* 09 */ "RC2C05-02 ($2002 AND $3F =$3D)",
 /* 0A */ "RC2C05-03 ($2002 AND $1F =$1C)",
 /* 0B */ "RC2C05-04 ($2002 AND $1F =$1B)",
 /* 0C */ "RC2C05-05 ($2002 AND $1F =unknown)",
 /* 0D */ "Reserved",
 /* 0E */ "Reserved",
 /* 0F */ "Reserved"
};

//Vs. Hardware type
static const char *vs_hardware_type[0x07] = {
 /* 00 */ "Vs. Unisystem (normal)",
 /* 01 */ "Vs. Unisystem (RBI Baseball protection)",
 /* 02 */ "Vs. Unisystem (TKO Boxing protection)",
 /* 03 */ "Vs. Unisystem (Super Xevious protection)",
 /* 04 */ "Vs. Unisystem (Vs. Ice Climber Japan protection)",
 /* 05 */ "Vs. Dual System (normal)",
 /* 06 */ "Vs. Dual System (Raid on Bungeling Bay protection)"
};

//Default expansion device
static const char *default_expansion_device[0x3B] = {
 /* 00 */ "Unspecified",
 /* 01 */ "Standard NES/Famicom controllers"
 /* 02 */ "NES Four Score/Satellite with two additional standard controllers",
 /* 03 */ "Famicom Four Players Adapter with two additional standard controllers using the \"simple\" protocol",
 /* 04 */ "Vs. System (1P via $4016)",
 /* 05 */ "Vs. System (1P via $4017)",
 /* 06 */ "Vs. Zapper",
 /* 07 */ "Reserved",
 /* 08 */ "Zapper ($4017)",
 /* 09 */ "Two Zappers",
 /* 0A */ "Bandai Hyper Shot Lightgun",
 /* 0B */ "Power Pad Side A",
 /* 0C */ "Power Pad Side B",
 /* 0D */ "Family Trainer Side A",
 /* 0E */ "Family Trainer Side B",
 /* 0F */ "Arkanoid Vaus Controller (NES)",
 /* 10 */ "Arkanoid Vaus Controller (Famicom)",
 /* 11 */ "Two Vaus Controllers plus Famicom Data Recorder",
 /* 12 */ "Konami Hyper Shot Controller",
 /* 13 */ "Coconuts Pachinko Controller",
 /* 14 */ "Exciting Boxing Punching Bag (Blowup Doll)",
 /* 15 */ "Jissen Mahjong Controller",
 /* 16 */ "Party Tap",
 /* 17 */ "Oeka Kids Tablet",
 /* 18 */ "Sunsoft Barcode Battler",
 /* 19 */ "Miracle Piano Keyboard",
 /* 1A */ "Pokkun Moguraa (Whack-a-Mole Mat and Mallet)",
 /* 1B */ "Top Rider (Inflatable Bicycle)",
 /* 1C */ "Double-Fisted (Requires or allows use of two controllers by one player)",
 /* 1D */ "Famicom 3D System",
 /* 1E */ "Doremikko Keyboard",
 /* 1F */ "R.O.B. Gyro Set",
 /* 20 */ "Famicom Data Recorder (\"silent\" keyboard)",
 /* 21 */ "ASCII Turbo File",
 /* 22 */ "IGS Storage Battle Box",
 /* 23 */ "Family BASIC Keyboard plus Famicom Data Recorder",
 /* 24 */ "Dongda PEC-586 Keyboard",
 /* 25 */ "Bit Corp. Bit-79 Keyboard",
 /* 26 */ "Subor Keyboard",
 /* 27 */ "Subor Keyboard plus mouse (3x8-bit protocol)",
 /* 28 */ "Subor Keyboard plus mouse (24-bit protocol via $4016)",
 /* 29 */ "SNES Mouse ($4017.d0)",
 /* 2A */ "Multicart",
 /* 2B */ "Two SNES controllers replacing the two standard NES controllers",
 /* 2C */ "RacerMate Bicycle",
 /* 2D */ "U-Force",
 /* 2E */ "R.O.B. Stack-Up",
 /* 2F */ "City Patrolman Lightgun",
 /* 30 */ "Sharp C1 Cassette Interface",
 /* 31 */ "Standard Controller with swapped Left-Right/Up-Down/B-A",
 /* 32 */ "Excalibor Sudoku Pad",
 /* 33 */ "ABL Pinball",
 /* 34 */ "Golden Nugget Casino extra buttons",
 /* 35 */ "Unknown famiclone keyboard used by the \"Golden Key\" educational cartridge",
 /* 36 */ "Subor Keyboard plus mouse (24-bit protocol via $4017)",
 /* 37 */ "Port test controller",
 /* 38 */ "Bandai Multi Game Player Gamepad buttons",
 /* 39 */ "Venom TV Dance Mat",
 /* 3A */ "LG TV Remote Control"
};


Cartridge* Cartridge_Load(const uint16_t *fileName){

    uint8_t *data = NULL;
    uint32_t size = 0;
    LoadFile(fileName,&data,&size);
    
    if(!data || memcmp(data,"NES\x1A",4)){
        fprintf(stderr,"Error: Invalid data\n");
        free(data);
        return NULL;
    }

    Cartridge *cartridge = malloc(sizeof(Cartridge));

    cartridge->data = data;
    cartridge->size = size;
    
    if(data[7] & 0x0C == 0x08){
        cartridge->header_format = NES20;
    }
    else{
        cartridge->header_format = iNES;
    }

    if(cartridge->header_format == NES20){

        cartridge->mapper_type = ((data[8] & 0x0F) << 8) | (data[7] & 0xF0) | ((data[6] & 0xF0) >> 4);

        cartridge->submapper_type = (data[8] & 0xF0) >> 4;

        uint32_t prg_rom_lsb = data[4];
        uint32_t prg_rom_msb = (data[9] & 0x0F) >> 0;
        
        if(prg_rom_msb != 0x0F){
            cartridge->prg_rom_banks = ((prg_rom_msb << 8) | prg_rom_lsb);
        }
        else{
            cartridge->prg_rom_banks = (1 << (prg_rom_lsb >> 2)) * ((prg_rom_lsb & 0x03) * 2 + 1);
        }

        uint32_t prg_ram_size = 64 << ((data[10] & 0x0F) >> 0); //volatile
        uint32_t prg_nvram_size = 64 << ((data[10] & 0xF0) >> 4); //no-valatile

        cartridge->prg_ram_size = (prg_ram_size > 0) ? prg_ram_size : prg_nvram_size;
        
        
        uint32_t chr_rom_lsb = data[5];
        uint32_t chr_rom_msb = (data[9] & 0xF0) >> 4;
        
        if(chr_rom_msb != 0x0F){
            cartridge->chr_rom_banks = ((chr_rom_msb << 8) | chr_rom_lsb) * (KB * 8);
        }
        else{
            cartridge->chr_rom_banks = (1 << (chr_rom_lsb >> 2)) * ((chr_rom_msb & 0x03) * 2 + 1);
        }
        
        uint32_t chr_ram_size = 64 << ((data[11] & 0x0F) >> 0); //volatile
        uint32_t chr_nvram_size = 64 << ((data[11] & 0xF0) >> 4); //non-volatile

        cartridge->chr_ram_size = (chr_ram_size) ? chr_ram_size : chr_nvram_size;
        

        if(data[6] & BIT_3){
            cartridge->mirroring = MIRRORING_FOUR_SCREEN;
        }
        else if(data[6] & BIT_0){
            cartridge->mirroring = MIRRORING_VERTICAL;
        }
        else{
            cartridge->mirroring = MIRRORING_HORIZONTAL;
        }

        cartridge->region = data[12] & 0x03;
        
        cartridge->system = data[7] & 0x03;
        
        cartridge->trainer = (data[6] & BIT_2) ? true : false;
        
        cartridge->misc_roms = data[14] & 0x03;
        
        cartridge->input_device = data[15] & 0x3F;
        if(cartridge->input_device > 0x3A) cartridge->input_device = 0x00;
        
        cartridge->battery = (data[6] & BIT_1) ? true : false;
    }
    else{

        cartridge->mapper_type = ((data[7] & 0xF0) >> 0) | ((data[6] & 0xF0) >> 4);

        cartridge->submapper_type = 0;

        cartridge->prg_rom_banks = data[4];

        cartridge->prg_ram_size = data[8] * (8 * KB);

        cartridge->chr_rom_banks = data[5];

        cartridge->chr_ram_size = 0;

        if(data[6] & BIT_3){
            cartridge->mirroring = MIRRORING_FOUR_SCREEN;
        }
        else if(data[6] & BIT_0){
            cartridge->mirroring = MIRRORING_VERTICAL;
        }
        else{
            cartridge->mirroring = MIRRORING_HORIZONTAL;
        }

        cartridge->region = data[9] & BIT_0;
        
        cartridge->system = data[7] & 0x03;
        
        cartridge->trainer = (data[6] & BIT_2) ? true : false;
        
        cartridge->misc_roms = 0;

        cartridge->input_device = 0;

        cartridge->battery = (data[6] & BIT_1) ? true : false;
    }

    cartridge->prg_rom_pointer = cartridge->data + 16 + ((cartridge->trainer) ? 512 : 0);
    cartridge->chr_rom_pointer = cartridge->prg_rom_pointer + (cartridge->prg_rom_banks * (16 * KB));

    return cartridge;
}

void Cartridge_Free(Cartridge *cartridge){
    if(!cartridge) return;
    free(cartridge->data);
    free(cartridge);
}

void Cartridge_PrintInformations(Cartridge *cartridge){

    printf("\n\n");

    //Hexadecimal
    printf("Hex: ");
    for(int i = 0; i<16; ++i){printf("%.2X ",cartridge->data[i]);}
    printf("\n");

    //Format
    if(cartridge->header_format == NES20) printf("Format: NES 2.0\n");
    else                                  printf("Format: iNES\n");

    //Mapper
    printf("Mapper: %d\n",cartridge->mapper_type);

    //Submapper
    printf("Submapper: %d\n",cartridge->submapper_type);

    //PRG-ROM Size
    if(cartridge->prg_rom_banks * (16 * KB) / KB > 0)  printf("PRG_ROM: %dkb\n",cartridge->prg_rom_banks * (16 * KB) / KB);
    else                                               printf("PRG_ROM: %db\n",cartridge->prg_rom_banks * (16 * KB));

    //PRG-RAM Size
    if(cartridge->prg_ram_size / KB > 0) printf("PRG_RAM: %dkb\n",cartridge->prg_ram_size / KB);
    else                                 printf("PRG_RAM: %db\n",cartridge->prg_ram_size);

    //CHR-ROM Size
    if(cartridge->chr_rom_banks * (8 * KB) / KB > 0)  printf("CHR_ROM: %dkb\n",cartridge->chr_rom_banks * (8 * KB) / KB);
    else                                              printf("CHR_ROM: %db\n",cartridge->chr_rom_banks * (8 * KB));

    //CHR-RAM Size
    if(cartridge->chr_ram_size / KB > 0) printf("CHR_RAM: %dkb\n",cartridge->chr_ram_size / KB);
    else                                 printf("CHR_RAM: %db\n",cartridge->chr_ram_size);

    //Mirroring
    printf("Mirroring: ");
    switch(cartridge->mirroring){
        case MIRRORING_HORIZONTAL:
            printf("Horizontal\n");
            break;
        case MIRRORING_VERTICAL:
            printf("Vertical\n");
            break;
        case MIRRORING_FOUR_SCREEN:
            printf("Four-Screen\n");
            break;
    }

    //Region
    printf("Region: %s\n",cpu_ppu_timing[cartridge->region]);

    //System
    printf("System: ");
    switch(cartridge->system){
        case NINTENDO_DEFAULT:
            printf("Nintendo Entertainment System/Family Computer\n");
            break;
        case NINTENDO_VS_SYSTEM:
            printf("Nintendo Vs. System\n");
            printf("PPU: %s\n",vs_ppu_types[cartridge->data[13] & 0x0F]);
            printf("Hardware: %s\n",vs_hardware_type[cartridge->data[13] & 0xF0]);
            break;
        case NINTENDO_PLAYCHOICE_10:
            printf("Nintendo Playchoice 10\n");
            break;
        case EXTENDED_CONSOLE_TYPE:
            printf("Extended console type\n");
            printf("%s\n",extended_console_type[cartridge->data[13] & 0x0F]);
            break;
    }

    //Trainer
    printf("Trainer: %s\n",(cartridge->trainer) ? "true" : "false");

    //Miscellaneous ROMs
    printf("Misc. ROM(s): %d\n",cartridge->misc_roms);

    //Input Device
    printf("Input device: %s\n",default_expansion_device[cartridge->input_device]);

    //Battery-backed
    printf("Battery: %s\n",(cartridge->battery) ? "true" : "false");

    printf("\n\n");
}
