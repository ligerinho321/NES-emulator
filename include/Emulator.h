#pragma once

#include <WG/WG_Window.h>
#include <WG/WG_Renderer.h>
#include <WG/WG_Audio.h>

#include <Constants.h>

#include <stdint.h>
#include <assert.h>


#define PointInRect(point,rect) ((point).x >= (rect).x && (point).x < (rect).x + (rect).w && (point).y >= (rect).y && (point).y < (rect).y + (rect).h)

#define SpriteIsVisible(sprite) ((sprite).x >= 0 && (sprite).x < 0xF9 && (sprite).y >= 0 && (sprite).y < 0xEF)

#define PutPixelRGB(pixels,pitch,rows,x,y,color)                \
do{                                                             \
    uint32_t offset = ((rows) - (y) - 1) * (pitch) + (x) * 3;   \
    pixels[offset + 0] = color.r;                               \
    pixels[offset + 1] = color.g;                               \
    pixels[offset + 2] = color.b;                               \
}while(0)

typedef struct _CPUTree              CPUTree;
typedef struct _PPUTree              PPUTree;
typedef struct _APUTree              APUTree;

typedef struct _TreeResources        TreeResources;
typedef struct _TreeBranch           TreeBranch;
typedef struct _TreeInput            TreeInput;
typedef struct _TreeButton           TreeButton;
typedef struct _TreeLabel            TreeLabel;
typedef struct _TreeNode             TreeNode;
typedef struct _Tree                 Tree;

typedef struct _ComboBoxElement      ComboBoxElement;
typedef struct _ComboBoxMenu         ComboBoxMenu;
typedef struct _ComboBox             ComboBox;

typedef struct _Shortcut             Shortcut;
typedef struct _MenuOption           MenuOption;
typedef struct _Menu                 Menu;

typedef struct _Range                Range;
typedef struct _ResizeHandle         ResizeHandle;
typedef struct _ToggleButton         ToggleButton;
typedef struct _ExpandButton         ExpandButton;
typedef struct _IconButton           IconButton;
typedef struct _Button               Button;
typedef struct _Input                Input;
typedef struct _ScrollBar            ScrollBar;
typedef struct _Label                Label;
typedef struct _Frame                Frame;
typedef struct _Screen               Screen;
typedef struct _Sprite               Sprite;
typedef struct _Controller           Controller;
typedef struct _FrameCounter         FrameCounter;
typedef struct _LengthCounter        LengthCounter;
typedef struct _LinearCounter        LinearCounter;
typedef struct _Sweep                Sweep;
typedef struct _Envelope             Envelope;
typedef struct _Pulse                Pulse;
typedef struct _Triangle             Triangle;
typedef struct _Noise                Noise;
typedef struct _DMC                  DMC;
typedef struct _APU                  APU;
typedef struct _Mapper               Mapper;
typedef struct _CPU_Instruction      CPU_Instruction;
typedef struct _CPU                  CPU;
typedef struct _PPU                  PPU;
typedef struct _Cartridge            Cartridge;
typedef struct _PPUVIE               PPUVIE;
typedef struct _PPUVI                PPUVI;
typedef struct _PPUV                 PPUV;
typedef struct _MemoryViewer         MemoryViewer;

typedef struct _DebuggerBreakpoints  DebuggerBreakpoints;
typedef struct _DebuggerLabels       DebuggerLabels;
typedef struct _DebuggerCallStack    DebuggerCallStack;
typedef struct _DebuggerDisassembly  DebuggerDisassembly;
typedef struct _DebuggerStatus       DebuggerStatus;
typedef struct _DebuggerMenuBar      DebuggerMenuBar;
typedef struct _Debugger             Debugger;

typedef struct _NES                  NES;
typedef struct _MenuBar              MenuBar;
typedef struct _Emulator             Emulator;

//Function Type
enum{
    FUNCTION_NONE,
    FUNCTION_SUBRUNTINE,
    FUNCTION_NMI,
    FUNCTION_IRQ,
    FUNCTION_MAIN
};

//Memory Type
enum{
    MEMORY_NONE,
    MEMORY_CPU,
    MEMORY_PPU,
    MEMORY_PRG_ROM,
    MEMORY_SYSTEM_RAM,
    MEMORY_WORK_RAM,
    MEMORY_NAMETABLE_RAM,
    MEMORY_SPRITE_RAM,
    MEMORY_SECONDARY_OAM,
    MEMORY_PALETTE_RAM,
    MEMORY_CHR_ROM,
    MEMORY_CHR_RAM
};

//PPUVI Type
enum{
    PPUVI_NONE,
    PPUVI_NAMETABLE_0,
    PPUVI_NAMETABLE_1,
    PPUVI_NAMETABLE_2,
    PPUVI_NAMETABLE_3,
    PPUVI_PATTERNTABLE_LEFT,
    PPUVI_PATTERNTABLE_RIGHT,
    PPUVI_OAM,
    PPUVI_PALETTE
};

//Input Flags
enum{
    INPUT_NONE             = 0x00,
    INPUT_ANY              = 0x01,
    INPUT_HEXADECIMAL      = 0x02,
    INPUT_DECIMAL          = 0x04,
    INPUT_SUBMIT           = 0x08,
    INPUT_VALIDATE         = 0x10,
    INPUT_RETURN_CONTENT   = 0x20,
};

//Break Flags
enum{
    BREAK_NONE    = 0x00,
    BREAK_WRITE   = 0x01,
    BREAK_READ    = 0x02,
    BREAK_EXECUTE = 0x04,
};

//Controller Flags
enum{
    CONTROLLER_BUTTON_NONE   = 0x00,
    CONTROLLER_BUTTON_A      = 0x01,
    CONTROLLER_BUTTON_B      = 0x02,
    CONTROLLER_BUTTON_SELECT = 0x04,
    CONTROLLER_BUTTON_START  = 0x08,
    CONTROLLER_BUTTON_UP     = 0x10,
    CONTROLLER_BUTTON_DOWN   = 0x20,
    CONTROLLER_BUTTON_LEFT   = 0x40,
    CONTROLLER_BUTTON_RIGHT  = 0x80
};


struct _Range{
    uint32_t start;
    uint32_t end;
};

struct _ResizeHandle{
    Rect rect;
    struct{
        bool over;
        bool pressed;
        int diff;
    }mouse;
};

struct _ToggleButton{
    struct{
        WG_Texture *off;
        WG_Texture *on;
    }icon;
    struct{
        bool over;
        bool pressed;
    }mouse;
    Rect rect;
    bool is_enabled;
};

struct _ExpandButton{
    struct{
        uint16_t *string;
        Vector2 size;
        Vector2 position;
        Rect rect;
    }name;
    struct{
        bool over;
        bool pressed;
    }mouse;
    struct{
        WG_Texture *arrow;
        Rect rect;
    }icon;
    WG_FontAtlas *atlas;
    Rect rect;
    bool open;
};

struct _IconButton{
    struct{
        WG_Texture *texture;
        Rect rect;
        struct{
            bool over;
            bool pressed;
        }mouse;
    }button;
    struct{
        uint16_t *text;
        Vector2 size;
        WG_FontAtlas *atlas;
    }tooltip;
};

struct _Button{
    struct{
        uint16_t *string;
        Vector2 size;
        Vector2 position;
    }s;
    struct{
        bool over;
        bool pressed;
    }mouse;
    WG_FontAtlas *atlas;
    Rect rect;
};

struct _Label{
    wchar_t *text;
    WG_FontAtlas *atlas;
    Vector2 size;
    Rect rect;
    Vector2 position;
};

struct _Frame{
    uint16_t *title;
    WG_FontAtlas *atlas;
    Vector2 title_size;
    Rect title_rect;
    Vector2 title_pos;
    RectF rect;
};

struct _Screen{
    WG_Texture *texture;
    uint8_t *pixels;
    int32_t pitch;
    Vector2 src_size;
    Vector2 dst_size;
    Rect rect;
    PixelFormat format;
};

struct _Sprite{
    uint8_t y;
    uint8_t index;
    uint8_t attributes;
    uint8_t x;
};

struct _Controller{
    uint8_t state;
    uint8_t index;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//APU

struct _FrameCounter{
    bool interrupt_inhibit;
    bool sequence_mode;
    uint8_t sequence_position;

    uint8_t last_value;
    uint32_t next_clock;
};

struct _LengthCounter{
    
    struct{
        uint32_t reload_cicle;
        bool reload;
        bool reload_value;
        bool value;
    }halt_flag;

    struct{
        uint32_t reload_cicle;
        bool reload;
        uint8_t reload_value;
        uint8_t value;
    }counter;
};

struct _LinearCounter{
    bool halt_flag;
    struct{
        bool reload_flag;
        uint8_t reload_value;
        uint8_t value;
    }counter;
};

struct _Sweep{
    Pulse *pulse;

    bool enabled;
    uint8_t period;
    bool negate_flag;
    uint8_t shift;

    bool reload_flag;
    uint8_t counter;

    uint16_t target_period;
};

struct _Envelope{
    bool start_flag;
    uint8_t devider;
    uint8_t counter;
    bool loop_flag;
    bool constant_volume;
    uint8_t volume;
};

struct _Pulse{
    bool channel_enabled;
    bool carry;
    
    uint8_t duty;
    uint8_t duty_position;
    
    uint16_t period;
    uint16_t timer;

    Envelope envelope;
    Sweep sweep;
    LengthCounter length_counter;
};

struct _Triangle{
    bool channel_enabled;

    uint16_t period;
    uint16_t timer;

    uint8_t sequence_position;

    LengthCounter length_counter;
    LinearCounter linear_counter;
};

struct _Noise{
    bool channel_enabled;
    bool mode_flag;

    uint16_t period;
    uint16_t timer;

    uint16_t lfsr;
    Envelope envelope;
    LengthCounter length_counter;
};

struct _DMC{
    bool channel_enabled;

    bool irq_enabled;
    bool loop_flag;
    
    uint16_t period;
    uint16_t timer;

    uint16_t sample_address;
    uint16_t sample_length;

    //Read memory
    bool sample_buffer_filled;
    uint8_t sample_buffer;
    uint16_t current_address;
    uint16_t bytes_remaining;

    //Output unit
    uint8_t output_level;
    uint8_t shift;
    uint8_t bits_remaining;
    bool silence;
};

struct _APU{
    NES *nes;

    Pulse pulse1;
    Pulse pulse2;
    Triangle triangle;
    Noise noise;
    DMC dmc;
    
    FrameCounter frame_counter;

    bool dmc_interrupt;
    bool frame_interrupt;

    float pulse_table[PULSE_TABLE_SIZE];
    float tnd_table[TND_TABLE_SIZE];

    AUDIO_TYPE output_samples[OUTPUT_SIZE];
    uint32_t output_count;

    uint32_t cycles;

    uint32_t cycles_per_samples_count;
    uint32_t cycles_per_samples;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CPU 6502

typedef void (*CPU_Operation)(CPU *cpu);

typedef uint16_t (*CPU_AddressMode)(CPU *cpu);

typedef bool (*CPU_CheckIO)(CPU *cpu);

struct _CPU_Instruction{
    uint16_t *name;
    CPU_Operation operation;
    CPU_AddressMode address;
    CPU_CheckIO check_io;
    uint8_t cycles;
    uint8_t bytes;
    uint8_t io;
};

struct _CPU{
    NES *nes;

    uint16_t program_counter;
    uint8_t accumulator;
    uint8_t x_register;
    uint8_t y_register;
    uint8_t status_register;
    uint8_t stack_pointer;
    
    uint8_t opcode;
    uint16_t address;
    int32_t cycles;
    CPU_Operation operation;

    uint32_t total_cycles;
    uint8_t bus;
    
    bool rst_asserted;
    bool rst_executing;

    uint32_t request_nmi_cycle;
    bool nmi_asserted;
    uint32_t nmi_delay;
    bool nmi_executing;

    uint8_t irq_source;
    bool irq_need;
    bool irq_delay;
    bool irq_executing;

    uint8_t oam_dma_byte;
    uint16_t oam_dma_start_address;
    uint32_t oam_dma_count;
    uint32_t oam_dma_delay;

    uint8_t ram[0x2000];
    const CPU_Instruction *instruction_table;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _PPU{
    NES *nes;

    uint8_t control;
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;
    uint8_t scroll;
    uint8_t address;
    uint8_t data;
    uint8_t buffer;

    uint8_t vram_increment;

    struct{
        uint16_t vram;
        uint16_t temp;
        uint16_t x_scroll;
        bool first_write;
    }scrolling;

    uint8_t name_tables[0x04][0x400];

    uint8_t palette[0x20];

    uint8_t oam[0x100];
    Sprite sprites[0x08];
    uint8_t sprites_pattern[0x08][0x02];
    uint8_t sprites_count;

    uint16_t bus;
    
    bool prevent_vblank;

    Vector2 scanline;

    bool odd_frame;
    bool skip_cycle;
    bool frame_complete;
    bool rendering_enabled;
    
    uint16_t next_tile_id_address;
    uint8_t next_tile_id;

    uint16_t next_attribute_address;
    uint8_t next_attribute;

    uint16_t next_tile_lsb_address;
    uint8_t next_tile_lsb;

    uint16_t next_tile_msb_address;
    uint8_t next_tile_msb;
    
    uint16_t pattern_shift_lsb;
    uint16_t pattern_shift_msb;
    
    uint16_t attribute_shift_lsb;
    uint16_t attribute_shift_msb;

    uint16_t sprite_pattern_address;
    
    void (*write)(PPU *ppu,uint8_t value,uint16_t address);
    uint8_t (*read)(PPU *ppu,uint16_t address);

    uint32_t total_cycles;
    uint32_t total_frames;
};

struct _Mapper{
    NES *nes;
    Cartridge *cartridge;
    void *extension;
    uint32_t mirroring;
    uint32_t type;

    uint8_t pattern_table[0x2000];
    uint8_t wram[0x2000];
    uint8_t rom[0x8000];

    void (*prg_write)(Mapper *mapper,uint8_t value,uint16_t address);
    uint8_t (*prg_read)(Mapper *mapper,uint16_t address);
    void (*chr_write)(Mapper *mapper,uint8_t value,uint16_t address);
    uint8_t (*chr_read)(Mapper *mapper,uint16_t adderess);
    void (*reset)(Mapper *mapper);
    void (*vram_address_change)(Mapper *mapper,uint16_t address);
};

struct _Cartridge{

    uint8_t *data;
    uint32_t size;

    uint32_t header_format;

    uint32_t mapper_type;
    uint32_t submapper_type;

    uint8_t *prg_rom_pointer;
    uint32_t prg_rom_banks;
    uint32_t prg_ram_size;

    uint8_t *chr_rom_pointer;
    uint32_t chr_rom_banks;
    uint32_t chr_ram_size;

    uint32_t mirroring;

    uint32_t region;

    uint32_t system;

    bool trainer;

    uint32_t misc_roms;

    uint32_t input_device;

    bool battery;
};

struct _PPUVIE{
    Label *variable;
    uint8_t type;
    union{
        Screen *screen;
        Label *string;
    }u;
    bool show;
    struct _PPUVIE *next;
};

struct _PPUVI{
    Emulator *emulator;
    NES *nes;
    PPUV *ppuv;
    WNDCLASSW window_class;
    WG_Window *window;
    WG_Renderer *renderer;
    WG_Font *font;
    WG_FontAtlas *atlas;
    HANDLE thread;

    struct{
        PPUVIE *tile;
        PPUVIE *palette;
        PPUVIE *col_row;
        PPUVIE *xy;
        PPUVIE *size;
        PPUVIE *tile_map_address;
        PPUVIE *tile_index;
        PPUVIE *tile_address;
        PPUVIE *palette_index;
        PPUVIE *palette_address;
        PPUVIE *attribute_address;
        PPUVIE *attribute_data;
        PPUVIE *list;
    }name_table_tile;

    struct{
        PPUVIE *tile;
        PPUVIE *tile_address;
        PPUVIE *tile_index;
        PPUVIE *list;
    }pattern_table_tile;

    struct{
        PPUVIE *sprite8x8;
        PPUVIE *sprite8x16;
        PPUVIE *palette;
        PPUVIE *sprite_index;
        PPUVIE *xy;
        PPUVIE *size;
        PPUVIE *tile_index;
        PPUVIE *tile_address;
        PPUVIE *palette_index;
        PPUVIE *palette_address;
        PPUVIE *horizontal_flip;
        PPUVIE *vertical_flip;
        PPUVIE *priority;
        PPUVIE *visible;
        uint8_t sprite_size;
        PPUVIE *list;
    }oam_sprite;

    struct{
        PPUVIE *color;
        PPUVIE *index;
        PPUVIE *value;
        PPUVIE *rgb;
        PPUVIE *hex;
        PPUVIE *list;
    }palette;

    uint16_t buffer[BUFFER_LENGTH];

    bool run;
    Vector2 window_pos;
    Vector2 window_size;
    Vector2 workarea_size;
    Vector2 glyph_size;
    Vector2 mouse_pos;
    Vector2 mouse_hotspot;
    DWORD threadID;
    WG_Event event;
};

struct _PPUV{
    Emulator *emulator;
    NES *nes;
    WNDCLASSW window_class;
    WG_Window *window;
    WG_Renderer *renderer;
    PPUVI *ppuvi;
    HANDLE thread;
    WG_Font *font;
    WG_FontAtlas *atlas;

    struct{
        uint32_t type;
        int column;
        int row;
        Vector2 position;
        Rect rect;
    }info;

    Frame *pattern_table_frame;
    Frame *name_table_frame;
    Frame *oam_table_frame;
    Frame *palette_table_frame;

    Screen *pattern_table_left;
    Screen *pattern_table_right;

    Screen *name_table_0;
    Screen *name_table_1;
    Screen *name_table_2;
    Screen *name_table_3;

    Screen *oam_table;

    Screen *palette_table;
    
    uint16_t buffer[BUFFER_LENGTH];

    bool run;
    bool update;
    Vector2 window_size;
    DWORD threadID;
    Vector2 glyph_size;
    uint32_t frame_spacing;
    WG_Event event;
    Vector2 mouse_pos;
    Vector2 mouse_hotspot;
};

struct _MemoryViewer{
    Emulator *emulator;
    NES *nes;
    WNDCLASSW window_class;
    WG_Window *window;
    WG_Renderer *renderer;
    WG_Font *font;
    WG_FontAtlas *atlas;
    WG_Texture *arrow;
    HANDLE thread;
    HANDLE mutex;
    uint16_t buffer[BUFFER_LENGTH];
    ScrollBar *vertical_scroll_bar;
    DWORD threadID;
    bool run;
    bool update_memory;
    Vector2 window_size;
    Vector2 glyph_size;
    Vector2 mouse_pos;
    WG_Event event;
    Rect bar_rect;
    Rect info_rect;
    
    struct{
        Label *label;
        ComboBoxElement *cpu_memory;
        ComboBoxElement *ppu_memory;
        ComboBoxElement *prg_rom;
        ComboBoxElement *system_ram;
        ComboBoxElement *work_ram;
        ComboBoxElement *nametable_ram;
        ComboBoxElement *sprite_ram;
        ComboBoxElement *secondary_oam;
        ComboBoxElement *palette_ram;
        ComboBoxElement *chr_rom;
        ComboBoxElement *chr_ram;
        ComboBox *combo_box;
        uint32_t type;
        uint32_t length;
    }memory_type;

    struct{
        Label *label;
        ComboBoxElement *column4;
        ComboBoxElement *column8;
        ComboBoxElement *column16;
        ComboBoxElement *column32;
        ComboBoxElement *column48;
        ComboBoxElement *column64;
        ComboBox *combo_box;
        uint32_t number;
    }memory_columns;

    struct{
        Label *label;
        Input *input;
    }memory_address;

    struct{
        uint32_t address_row_width;
        uint32_t address_row_length;
        
        Rect address_row_rect;
        Rect address_column_rect;
        Rect values_rect;
        Rect rect;

        uint8_t (*read)(MemoryViewer *memory_viewer,uint32_t address);
        void (*write)(MemoryViewer *memory_viewer,uint8_t value,uint32_t address);

        struct{
            uint32_t address;
            uint32_t row;
            uint32_t column;

            bool editing;
            
            uint16_t buffer[3];
            uint8_t value;

            struct{
                bool on;
                bool visible;
                bool update;
                uint32_t time;
            }cursor;
        }input;

        struct{
            uint32_t _start;
            uint32_t _end;
            uint32_t start;
            uint32_t end;
            bool enabled;
        }selection;
    }memory;

    struct{
        uint32_t start_line;
        uint32_t end_line;
        uint32_t start_address;
        uint32_t end_address;
    }viewer_metrics;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Debugger

#define OPERATION_ADD  0x01
#define OPERATION_EDIT 0x02

typedef struct _BreakpointData BreakpointData;
typedef struct _BreakpointView BreakpointView;
typedef struct _BreakpointEdit BreakpointEdit;

struct _DebuggerBreakpoints{
    Debugger *debugger;
    ExpandButton *expand_button;
    struct{
        struct{
            BreakpointData *data;
            BreakpointView *view;
            BreakpointView *current;
        }breakpoints;
        Rect rect;
        struct{
            bool over;
            bool pressed;
        }mouse;
        ScrollBar *vertical_scroll_bar;
    }list;
    struct{
        MenuOption *add;
        MenuOption *edit;
        MenuOption *delete;
        Menu *menu;
    }menu;
    BreakpointEdit *breakpoint_edit;
    Rect rect;
    void (*config_rect)(DebuggerBreakpoints *breakpoints);
};

struct _DebuggerLabels{
    Debugger *debugger;
    ExpandButton *expand_button;
    Rect rect;
    void (*config_rect)(DebuggerLabels *labels);
};

struct _DebuggerCallStack{
    Debugger *debugger;
    ExpandButton *expand_button;
    Rect rect;
    void (*config_rect)(DebuggerCallStack *callstack);
};

struct _DebuggerStatus{
    Debugger *debugger;
    NES *nes;
    TreeResources *tree_resources;
    struct{
        ExpandButton *expand_button;
        CPUTree *tree;
    }cpu;
    struct{
        ExpandButton *expand_button;
        PPUTree *tree;
    }ppu;
    struct{
        ExpandButton *expand_button;
        APUTree *tree;
    }apu;
    void (*config_rect)(DebuggerStatus *status);
};

struct _DebuggerDisassembly{
    Debugger *debugger;
    NES *nes;
    ScrollBar *vertical_scroll_bar;
    Rect rect;
    Rect breakpoint_rect;
    bool update;
    struct{
        uint16_t start_address;
        uint16_t end_address;
        uint16_t start_line;
        uint16_t end_line;
    }metrics;
    void (*config_rect)(DebuggerDisassembly *disassembly);
};

struct _DebuggerMenuBar{
    Debugger *debugger;
    IconButton *_continue;
    IconButton *_break;
    IconButton *step_into;
    IconButton *step_over;
    IconButton *step_out;
    Rect rect;
    void (*config_rect)(struct _DebuggerMenuBar *menu_bar);
};

struct _Debugger{
    WNDCLASS window_class;
    Emulator *emulator;
    NES *nes;
    WG_Window *window;
    WG_Renderer *renderer;
    WG_Font *font;
    WG_FontAtlas *atlas;
    HANDLE thread;
    HANDLE command_mutex;
    HANDLE breakpoint_mutex;
    uint16_t buffer[BUFFER_LENGTH];
    
    struct{
        WG_Texture *arrow;
        WG_Texture *branch1;
        WG_Texture *branch2;
        WG_Texture *branch3;
        WG_Texture *breakpoint_arrow;
        WG_Texture *circle;
        WG_Texture *checked;
        WG_Texture *unchecked;
        WG_Texture *help;
        WG_Texture *play;
        WG_Texture *pause;
        WG_Texture *step_into;
        WG_Texture *step_over;
        WG_Texture *step_out;
    }resources;

    struct{
        struct{
            Rect rect;
            Vector2 border[2];
        }top;

        struct{
            Rect rect;
            Vector2 border[2];
            ResizeHandle resize;
            bool open;
        }left;

        struct{
            Rect rect;
        }center;

        struct{
            Rect rect;
            Vector2 border[2];
            ResizeHandle resize;
            bool open;
        }right;
    }metrics;

    DebuggerMenuBar menu_bar;

    DebuggerBreakpoints breakpoints;

    DebuggerLabels labels;

    DebuggerCallStack callstack;
    
    DebuggerDisassembly disassembly;

    DebuggerStatus status;

    DWORD threadID;
    WG_Event event;
    Vector2 glyph_size;
    Vector2 window_size;
    Vector2 mouse;
    Vector2 mouse_hotspot;
    bool run;
    bool request_activate;
    bool activated;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _NES{
    Emulator *emulator;
    Mapper mapper;
    APU apu;
    CPU cpu;
    PPU ppu;
    Cartridge *cartridge;
    Color system_palette[64];
    Screen *screen;
    const uint8_t *keyboardstate;
    Controller controller[2];
    bool strobe;
    Rect rect;
    bool paused;
    bool breakpoint_found;

    void (*write)(struct _NES *nes,uint8_t value,uint16_t address);
    uint8_t (*read)(struct _NES *nes,uint16_t address);

    void (*toggle_pause)(struct _NES *nes);
    void (*step_into)(struct _NES *nes);
    void (*run_frame)(struct _NES *nes);
};

struct _MenuBar{
    Emulator *emulator;
    Rect rect;
    struct{
        Button *button;
        MenuOption *open;
        MenuOption *close;
        MenuOption *exit;
        Menu *menu;
    }file;
    struct{
        Button *button;
        MenuOption *debugger;
        MenuOption *memory_viewer;
        MenuOption *ppu_viewer;
        Menu *menu;
    }debug;
};

struct _Emulator{

    WNDCLASSW window_class;

    WG_Window *window;
    
    WG_Renderer *renderer;

    WG_AudioDevice *audio_device;

    WG_Font *font;
    WG_FontAtlas *atlas;

    MenuBar *menubar;
    NES nes;
    PPUV *ppuv;
    MemoryViewer *memory_viewer;
    Debugger *debugger;

    uint16_t buffer[BUFFER_LENGTH];

    bool run;
    uint64_t last_timer;
    double frame_duration;
    Vector2 window_size;
    Vector2 glyph_size;
    Vector2 mouse_pos;
    WG_Event event;
};

void Emulator_Init(Emulator *emulator,HINSTANCE hInstance);

void Emulator_ConfigRect(Emulator *emulator);

void Emulator_Run(Emulator *emulator);

void Emulator_Free(Emulator *emulator);