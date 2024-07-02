#include <Debugger/Debugger.h>
#include <Debugger/MenuBar.h>
#include <Debugger/Breakpoints/DebuggerBreakpoints.h>
#include <Debugger/Breakpoints/Breakpointdata.h>
#include <Debugger/Labels.h>
#include <Debugger/CallStack.h>
#include <Debugger/Disassembly.h>
#include <Debugger/Status.h>
#include <Debugger/Metrics.h>
#include <Utils.h>
#include <Resources.h>

#define WINDOW_MIN_WIDTH (LEFT_MIN_WIDTH + CENTER_MIN_WIDTH + RIGHT_MIN_WIDTH)
#define WINDOW_MIN_HEIGHT 600

static void Debugger_LoadResources(Debugger *debugger){

    debugger->resources.arrow = WG_LoadTextureFromData(debugger->renderer,arrow,arrow_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.arrow,OFF_COLOR);

    debugger->resources.branch1 = WG_LoadTextureFromData(debugger->renderer,branch1,branch1_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.branch1,OFF_COLOR);

    debugger->resources.branch2 = WG_LoadTextureFromData(debugger->renderer,branch2,branch2_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.branch2,OFF_COLOR);

    debugger->resources.branch3 = WG_LoadTextureFromData(debugger->renderer,branch3,branch3_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.branch3,OFF_COLOR);

    debugger->resources.breakpoint_arrow = WG_LoadTextureFromData(debugger->renderer,breakpoint_arrow,breakpoint_arrow_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.breakpoint_arrow,OFF_COLOR);

    debugger->resources.circle = WG_LoadTextureFromData(debugger->renderer,circle,circle_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.circle,OFF_COLOR);

    debugger->resources.checked = WG_LoadTextureFromData(debugger->renderer,checked,checked_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.checked,OFF_COLOR);

    debugger->resources.unchecked = WG_LoadTextureFromData(debugger->renderer,unchecked,unchecked_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.unchecked,OFF_COLOR);

    debugger->resources.help = WG_LoadTextureFromData(debugger->renderer,help,help_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.help,OFF_COLOR);

    debugger->resources.play = WG_LoadTextureFromData(debugger->renderer,play,play_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.play,OFF_COLOR);

    debugger->resources.pause = WG_LoadTextureFromData(debugger->renderer,pause,pause_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.pause,OFF_COLOR);

    debugger->resources.step_into = WG_LoadTextureFromData(debugger->renderer,step_into,step_into_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.step_into,OFF_COLOR);

    debugger->resources.step_over = WG_LoadTextureFromData(debugger->renderer,step_over,step_over_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.step_over,OFF_COLOR);

    debugger->resources.step_out = WG_LoadTextureFromData(debugger->renderer,step_out,step_out_size,TEXTURE_NONE);
    WG_SetTextureColor(debugger->resources.step_out,OFF_COLOR);
}

static void Debugger_DestroyResources(Debugger *debugger){
    WG_DestroyTexture(debugger->resources.arrow);
    WG_DestroyTexture(debugger->resources.branch1);
    WG_DestroyTexture(debugger->resources.branch2);
    WG_DestroyTexture(debugger->resources.branch3);
    WG_DestroyTexture(debugger->resources.breakpoint_arrow);
    WG_DestroyTexture(debugger->resources.circle);
    WG_DestroyTexture(debugger->resources.checked);
    WG_DestroyTexture(debugger->resources.unchecked);
    WG_DestroyTexture(debugger->resources.help);
    WG_DestroyTexture(debugger->resources.play);
    WG_DestroyTexture(debugger->resources.pause);
    WG_DestroyTexture(debugger->resources.step_into);
    WG_DestroyTexture(debugger->resources.step_over);
    WG_DestroyTexture(debugger->resources.step_out);
}

void Debugger_Init(Debugger *debugger,Emulator *emulator){
    memset(debugger,0,sizeof(Debugger));
    
    debugger->window_class = emulator->window_class;
    
    debugger->emulator = emulator;
    
    debugger->nes = &emulator->nes;

    debugger->font = emulator->font;

    debugger->menu_bar.debugger = debugger;
    debugger->breakpoints.debugger = debugger;
    debugger->labels.debugger = debugger;
    debugger->callstack.debugger = debugger;
    debugger->disassembly.debugger = debugger;
    debugger->status.debugger = debugger;

    debugger->window_size = (Vector2){
        .x = WINDOW_MIN_WIDTH,
        .y = WINDOW_MIN_HEIGHT
    };

    GetIconDimensions(GetCursor(),&debugger->mouse_hotspot.x,&debugger->mouse_hotspot.y,NULL,NULL);
    
    debugger->command_mutex = CreateMutex(NULL,FALSE,NULL);
    debugger->breakpoint_mutex = CreateMutex(NULL,FALSE,NULL);
}

void Debugger_OpenWindow(Debugger *debugger){

    debugger->window = WG_CreateWindow(
        &debugger->emulator->window_class,
        NULL,
        L"Debugger",
        WG_WINDOWPOS_CENTERED,
        WG_WINDOWPOS_CENTERED,
        debugger->window_size.x,
        debugger->window_size.y,
        WG_WINDOW_SHOWN | WG_WINDOW_DEFAULT,
        0
    );
    
    WG_SetWindowMinSize(debugger->window,WINDOW_MIN_WIDTH,WINDOW_MIN_HEIGHT);

    debugger->renderer = WG_CreateRenderer(debugger->window,NULL);
    
    WG_RendererSetVsync(debugger->renderer,true);
    
    debugger->atlas = WG_RenderFontAtlas(debugger->renderer,debugger->font,FONT_HEIGHT,0x20,0xFF);

    WG_SizeTextByFontAtlas(debugger->atlas,L"A",&debugger->glyph_size.x,&debugger->glyph_size.y);

    Debugger_LoadResources(debugger);
    

    DebuggerMenuBar_Init(&debugger->menu_bar);

    DebuggerBreakpoints_Init(&debugger->breakpoints);

    Labels_Init(&debugger->labels);

    CallStack_Init(&debugger->callstack);

    Disassembly_Init(&debugger->disassembly);

    Status_Init(&debugger->status);


    debugger->metrics.left.resize.rect.x = LEFT_MIN_WIDTH - RESIZE_HANDLE_WIDTH * 0.5f;
    debugger->metrics.left.open = true;

    debugger->metrics.right.resize.rect.x = debugger->window_size.x - RIGHT_MIN_WIDTH - RESIZE_HANDLE_WIDTH * 0.5f;
    debugger->metrics.right.open = true;

    ConfigMetrics(debugger);

    debugger->run = true;
}

void Debugger_CloseWindow(Debugger *debugger){
    
    DebuggerMenuBar_Free(&debugger->menu_bar);

    DebuggerBreakpoints_Free(&debugger->breakpoints);
    
    Labels_Free(&debugger->labels);

    CallStack_Free(&debugger->callstack);

    Disassembly_Free(&debugger->disassembly);

    Status_Free(&debugger->status);

    Debugger_DestroyResources(debugger);

    WG_FontAtlasFree(debugger->atlas);

    WG_DestroyRenderer(debugger->renderer);

    WG_DestroyWindow(debugger->window);

    debugger->run = false;
}

static void Debugger_MouseOver(Debugger *debugger){
    return;
}

static void Debugger_MouseButtonDown(Debugger *debugger){
    
    if(PointInRect(debugger->mouse,debugger->metrics.left.resize.rect)){
        debugger->metrics.left.resize.mouse.pressed = true;
        debugger->metrics.left.resize.mouse.diff = debugger->mouse.x - debugger->metrics.left.resize.rect.x;
    }
    else if(PointInRect(debugger->mouse,debugger->metrics.right.resize.rect)){
        debugger->metrics.right.resize.mouse.pressed = true;
        debugger->metrics.right.resize.mouse.diff = debugger->mouse.x - debugger->metrics.right.resize.rect.x;
    }

}

static void Debugger_MouseButtonUp(Debugger *debugger){

    if(debugger->metrics.left.resize.mouse.pressed){
        debugger->metrics.left.resize.mouse.pressed = false;
    }
    else if(debugger->metrics.right.resize.mouse.pressed){
        debugger->metrics.right.resize.mouse.pressed = false;
    }
}

void Debugger_Event(Debugger *debugger){
    
    WG_PollEvent(debugger->window,&debugger->event);

    WG_GetMousePosInWindow(debugger->window,&debugger->mouse.x,&debugger->mouse.y);

    Debugger_MouseOver(debugger);

    if(debugger->event.type == WG_QUIT){
        debugger->run = false;
    }
    else if(debugger->event.type == WG_WINDOWRESIZE){
        WG_GetWindowClientSize(debugger->window,&debugger->window_size.x,&debugger->window_size.y);
        ConfigMetrics(debugger);
    }
    else if(debugger->event.type == WG_MOUSEBUTTONDOWN && debugger->event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Debugger_MouseButtonDown(debugger);
    }
    else if(debugger->event.type == WG_MOUSEBUTTONUP && debugger->event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Debugger_MouseButtonUp(debugger);
    }
    else if(WG_GetMouseState() & WG_MOUSEBUTTON_LEFT){

        if(debugger->metrics.left.resize.mouse.pressed){
            debugger->metrics.left.resize.rect.x = debugger->mouse.x - debugger->metrics.left.resize.mouse.diff;
            ConfigMetrics(debugger);
        }
        else if(debugger->metrics.right.resize.mouse.pressed){
            debugger->metrics.right.resize.rect.x = debugger->mouse.x - debugger->metrics.right.resize.mouse.diff;
            ConfigMetrics(debugger);
        }
    }
}

static void Debugger_RunMainWindow(Debugger *debugger){
    
    Debugger_Event(debugger);

    WG_SetCurrentRenderer(debugger->renderer);
    
    WG_ClearColor(debugger->renderer,BACKGROUND2_COLOR);

    WG_DrawFilledRectangle(debugger->renderer,&debugger->metrics.top.rect,BACKGROUND1_COLOR);


    DebuggerMenuBar_Run(&debugger->menu_bar);
    
    DebuggerBreakpoints_Run(&debugger->breakpoints);

    Labels_Run(&debugger->labels);

    CallStack_Run(&debugger->callstack);

    Disassembly_Run(&debugger->disassembly);

    Status_Run(&debugger->status);

    //WG_DrawRectangle(debugger->renderer,&debugger->metrics.left.resize.rect,DEBUG_COLOR);
    //WG_DrawRectangle(debugger->renderer,&debugger->metrics.right.resize.rect,DEBUG_COLOR);
        
    WG_DrawLine(debugger->renderer,debugger->metrics.top.border[0],debugger->metrics.top.border[1],BORDER_COLOR);
        
    if(debugger->metrics.left.open){
        WG_DrawLine(debugger->renderer,debugger->metrics.left.border[0],debugger->metrics.left.border[1],BORDER_COLOR);
    }
        
    if(debugger->metrics.right.open){
        WG_DrawLine(debugger->renderer,debugger->metrics.right.border[0],debugger->metrics.right.border[1],BORDER_COLOR);
    }

    DebuggerMenuBar_DrawPopUps(&debugger->menu_bar);

    WG_Flip(debugger->renderer);
}

static void Debugger_RunChildrenWindow(Debugger *debugger){
    DebuggerBreakpoints_RunChildrenWindow(&debugger->breakpoints);
}

DWORD WINAPI Debugger_Main(LPVOID lParam){
    
    Debugger *debugger = lParam;

    Debugger_OpenWindow(debugger);

    while(debugger->run){
        Debugger_RunMainWindow(debugger);
        Debugger_RunChildrenWindow(debugger);
    }

    Debugger_CloseWindow(debugger);

    return 0;
}

void Debugger_Execute(Debugger *debugger){
    Debugger_Exit(debugger);

    debugger->thread = CreateThread(NULL,0,Debugger_Main,debugger,0,&debugger->threadID);

    if(debugger->thread){
        debugger->run = true;
    }
    else{
        printf("Failed to initialize Debugger: %d.\n",GetLastError());
    }
}

void Debugger_Exit(Debugger *debugger){
    debugger->run = false;
    WaitForSingleObject(debugger->thread,INFINITE);
    CloseHandle(debugger->thread);
    debugger->thread = NULL;
}

void Debugger_Free(Debugger *debugger){
    Debugger_Exit(debugger);
    
    BreakpointsData_Free(&debugger->breakpoints);
    
    CloseHandle(debugger->command_mutex);

    CloseHandle(debugger->breakpoint_mutex);
    
    free(debugger);
}