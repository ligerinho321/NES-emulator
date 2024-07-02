#pragma once

#define BUFFER_LENGTH 260

#define TILE_SIZE 8

#define FONT_HEIGHT 18

#define ICON_SIZE 30

#define ELEMENT_HEIGHT 30

#define ELEMENT_ROW_HEIGHT 40

#define BORDER_SPACING 5
#define BUTTON_SPACING 10

#define NES_SCREEN_COLUMNS 32
#define NES_SCREEN_ROWS 30
#define NES_SCREEN_WIDTH 256
#define NES_SCREEN_HEIGHT 240

#define PATTERN_TABLE_WIDTH 128
#define PATTERN_TABLE_HEIGHT 128

#define NAME_TABLE_COLUMNS 32
#define NAME_TABLE_ROWS 30
#define NAME_TABLE_WIDTH 256
#define NAME_TABLE_HEIGHT 240

#define OAM_TABLE_WIDTH 128
#define OAM_TABLE_HEIGHT 64

#define PALETTE_TABLE_WIDTH 128
#define PALETTE_TABLE_HEIGHT 16

#define DEBUG_COLOR               (Color){0,255,0,255}
#define DEBUG_VARIABLE_COLOR      (Color){120,220,232,255}
#define DEBUG_VALUE_COLOR         (Color){171,157,242,255}
#define DEBUG_INSTRUCTION_COLOR   (Color){169,220,118,255}
#define DEBUG_SYMBOL_COLOR        (Color){255,97,136,255}

#define ON_COLOR                  (Color){255,255,255,255}
#define OFF_COLOR                 (Color){150,150,150,255}
#define SELECTED_COLOR            (Color){255,190,0,255}
#define BORDER_COLOR              (Color){100,100,100,255}
#define ALERT_COLOR               (Color){255,0,0,255}

#define BACKGROUND_SELECTED_COLOR (Color){150,150,150,50}
#define BACKGROUND1_COLOR         (Color){15,15,15,255}
#define BACKGROUND2_COLOR         (Color){20,20,20,255}
#define BACKGROUND3_COLOR         (Color){25,25,25,255}

#define RESIZE_HANDLE_WIDTH 6

#define BLINK_TIME 600

#define CPU_6502_RATE 1789773

#define AUDIO_RATE 44100
#define AUDIO_BITS 16
#define AUDIO_TYPE int16_t
#define AUDIO_MAX  INT16_MAX

#define PULSE_TABLE_SIZE 31
#define TND_TABLE_SIZE 203
#define OUTPUT_SIZE 1024

#define BIT_7 0x80
#define BIT_6 0x40
#define BIT_5 0x20
#define BIT_4 0x10
#define BIT_3 0x08
#define BIT_2 0x04
#define BIT_1 0x02
#define BIT_0 0x01

//Mirroring
#define MIRRORING_HORIZONTAL       0x00
#define MIRRORING_VERTICAL         0x01
#define MIRRORING_FOUR_SCREEN      0x02
#define MIRRORING_ONE_SCREEN       0x03
#define MIRRORING_ONE_SCREEN_UPPER 0x04
#define MIRRORING_ONE_SCREEN_LOWER 0x05

//Reset
#define SOFTWARE_RESET 0x01
#define HARDWARE_RESET 0x02

#define LABEL  0x01
#define SCREEN 0x02
#define INPUT  0x04
#define BUTTON 0x08