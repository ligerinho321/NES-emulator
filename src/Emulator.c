#include <Emulator.h>
#include <MenuBar.h>
#include <NES/NES.h>
#include <PPUV/PPUV.h>
#include <MemoryViewer/MemoryViewer.h>
#include <Debugger/Debugger.h>
#include <Utils.h>
#include <Resources.h>

void Emulator_Init(Emulator *emulator,HINSTANCE hInstance){

    memset(emulator,0,sizeof(Emulator));
    
    emulator->window_class = (WNDCLASSW){0};
    emulator->window_class.lpfnWndProc = WindowProc;
    emulator->window_class.hInstance = hInstance;
    emulator->window_class.lpszClassName = L"Main";
    
    RegisterClassW(&emulator->window_class);

    emulator->window_size = (Vector2){NES_SCREEN_WIDTH * 2.0f,NES_SCREEN_HEIGHT * 2.0f + FONT_HEIGHT + 10};

    emulator->window = WG_CreateWindow(
        &emulator->window_class,
        NULL,
        L"NES Emulator",
        WG_WINDOWPOS_CENTERED,
        WG_WINDOWPOS_CENTERED,
        emulator->window_size.x,
        emulator->window_size.y,
        WG_WINDOW_SHOWN | WG_WINDOW_DEFAULT,
        0
    );

    WG_SetWindowMinSize(emulator->window,NES_SCREEN_WIDTH,NES_SCREEN_HEIGHT);

    emulator->renderer = WG_CreateRenderer(emulator->window,NULL);
    
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = AUDIO_RATE + 500;
    waveFormat.wBitsPerSample = AUDIO_BITS;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    emulator->audio_device = WG_OpenAudioDevice(waveFormat);

    uint8_t *data = malloc(font_size);
    memcpy(data,font,font_size);
    emulator->font = WG_OpenFontFromData(data,FONT_HEIGHT);

    emulator->atlas = WG_RenderFontAtlas(emulator->renderer,emulator->font,FONT_HEIGHT,0x20,0xFF);
    
    WG_SizeTextByFontAtlas(emulator->atlas,L"A",&emulator->glyph_size.x,&emulator->glyph_size.y);

    emulator->menubar = malloc(sizeof(MenuBar));
    MenuBar_Init(emulator->menubar,emulator);

    NES_Init(&emulator->nes,emulator);

    emulator->ppuv = malloc(sizeof(PPUV));
    PPUV_Init(emulator->ppuv,emulator);

    emulator->memory_viewer = malloc(sizeof(MemoryViewer));
    MemoryViewer_Init(emulator->memory_viewer,emulator);

    emulator->debugger = malloc(sizeof(Debugger));
    Debugger_Init(emulator->debugger,emulator);
    
    emulator->run = true;

    emulator->last_timer = WG_GetPerformanceCounter();
    emulator->frame_duration = 1.0 / 60.098814;

    Emulator_ConfigRect(emulator);
}

void Emulator_ConfigRect(Emulator *emulator){
    
    emulator->menubar->rect = (Rect){
        .x = 0,
        .y = 0,
        .w = emulator->window_size.x,
        .h = ELEMENT_HEIGHT
    };

    MenuBar_ConfigRect(emulator->menubar);

    emulator->nes.rect = (Rect){
        .x = 0,
        .y = emulator->menubar->rect.y + emulator->menubar->rect.h,
        .w = emulator->window_size.x,
        .h = emulator->window_size.y - emulator->menubar->rect.h
    };

    NES_ResizeScreen(&emulator->nes);
}

void Emulator_Event(Emulator *emulator){

    WG_PollEvent(emulator->window,&emulator->event);

    WG_GetMousePosInWindow(emulator->window,&emulator->mouse_pos.x,&emulator->mouse_pos.y);

    if(emulator->event.type == WG_QUIT){
        
        emulator->run = false;
    }
    else if(emulator->event.type == WG_WINDOWRESIZE){
        
        WG_GetWindowClientSize(emulator->window,&emulator->window_size.x,&emulator->window_size.y);

        Emulator_ConfigRect(emulator);
    }
    else if(emulator->event.type == WG_KEYDOWN){
                
        if(emulator->nes.cartridge != NULL){
            
            //Pause
            if(emulator->event.key.scancode == WG_SCANCODE_ESCAPE){
                emulator->nes.toggle_pause(&emulator->nes);
            }
            //Reset
            else if(emulator->event.key.scancode == WG_SCANCODE_F4){
                NES_Reset(&emulator->nes,HARDWARE_RESET);
            }
        }
    }
}

static void Emulator_RunMainWindow(Emulator *emulator){
    
    Emulator_Event(emulator);

    WG_SetCurrentRenderer(emulator->renderer);
    
    WG_ClearColor(emulator->renderer,BACKGROUND1_COLOR);

    MenuBar_Run(emulator->menubar);
        
    NES_Run(&emulator->nes);

    WG_Flip(emulator->renderer);
}

static void Emulator_RunChildrenWindow(Emulator *emulator){
    MenuBar_RunChildrenWindow(emulator->menubar);
}

void Emulator_Run(Emulator *emulator){
    while(emulator->run){
        
        Emulator_RunMainWindow(emulator);
        Emulator_RunChildrenWindow(emulator);

        float performace_frequency = WG_GetPerformanceFrequency();

        while(true){

            float elapsed_seconds = (WG_GetPerformanceCounter() - emulator->last_timer) / performace_frequency;
                
            if(elapsed_seconds >= emulator->frame_duration){
                break;
            }
        }

        uint64_t current_timer = WG_GetPerformanceCounter();

        emulator->nes.apu.cycles_per_samples = (29780.5f / ((current_timer - emulator->last_timer) / performace_frequency)) / AUDIO_RATE;
                    
        emulator->last_timer = current_timer;
    }
}

void Emulator_Free(Emulator *emulator){

    PPUV_Free(emulator->ppuv);

    MemoryViewer_Free(emulator->memory_viewer);

    Debugger_Free(emulator->debugger);
    
    MenuBar_Free(emulator->menubar);

    NES_Free(&emulator->nes);

    WG_FontAtlasFree(emulator->atlas);

    WG_CloseFont(emulator->font);

    WG_CloseAudioDevice(emulator->audio_device);

    WG_DestroyRenderer(emulator->renderer);

    WG_DestroyWindow(emulator->window);
}