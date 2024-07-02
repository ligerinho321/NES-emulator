#include <MenuBar.h>
#include <Menu.h>
#include <Utils.h>
#include <NES/NES.h>
#include <NES/Cartridge.h>
#include <MemoryViewer/MemoryViewer.h>
#include <PPUV/PPUV.h>
#include <Debugger/Debugger.h>

static void File_Open(void *data){

    MenuBar *menubar = data;

    OPENFILENAMEW ofn = {0};
    uint16_t buffer[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = L"All Suported Files (*.nes,*.unif,*.unf,*.fds,*.nsf)\0*.nes;*.unif;*.unf;*.fds;*.nsf\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.hwndOwner = WG_GetWindowHandle(menubar->emulator->window);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    if(GetOpenFileName(&ofn)){
        NES_InsertCartridge(&menubar->emulator->nes,buffer);
    }
    else{
        printf("Failed to open file: %d\n",CommDlgExtendedError());
    }
}

static void File_Close(void *data){

    MenuBar *menubar = data;

    if(!menubar->emulator->nes.cartridge) return;

    if(menubar->emulator->ppuv->run){
        PPUV_Exit(menubar->emulator->ppuv);
    }

    if(menubar->emulator->memory_viewer->run){
        MemoryViewer_Exit(menubar->emulator->memory_viewer);
    }

    if(menubar->emulator->debugger->run){
        Debugger_Exit(menubar->emulator->debugger);
    }

    Cartridge_Free(menubar->emulator->nes.cartridge);

    menubar->emulator->nes.cartridge = NULL;
}

static void File_Exit(void *data){
    MenuBar *menubar = data;
    menubar->emulator->run = false;
}

void File_Init(MenuBar *menubar){
    menubar->file.button = CreateButton(L"File",menubar->emulator->atlas);
    
    menubar->file.menu = malloc(sizeof(Menu));
    Menu_Init(
        menubar->file.menu,
        menubar->emulator->window_class,
        menubar->emulator->window,
        menubar->emulator->renderer,
        menubar->emulator->atlas
    );

    Shortcut *open_shortcut = Menu_CreateShortcut(L"Ctrl+O",WG_KEYMOD_CTRL,WG_SCANCODE_O);
    menubar->file.open = Menu_CreateOption(L"Open",open_shortcut,File_Open,menubar);
    Menu_InsertOption(menubar->file.menu,menubar->file.open);

    Shortcut *close_shortcut = Menu_CreateShortcut(L"Ctrl+F4",WG_KEYMOD_CTRL,WG_SCANCODE_F4);
    menubar->file.close = Menu_CreateOption(L"Close",close_shortcut,File_Close,menubar);
    Menu_InsertOption(menubar->file.menu,menubar->file.close);

    Shortcut *exit_shortcut = Menu_CreateShortcut(L"Alt+F4",WG_KEYMOD_ALT,WG_SCANCODE_F4);
    menubar->file.exit = Menu_CreateOption(L"Exit",exit_shortcut,File_Exit,menubar);
    Menu_InsertOption(menubar->file.menu,menubar->file.exit);
}


static void Debug_Debugger(void *data){
    
    MenuBar *menubar = data;

    if(!menubar->emulator->nes.cartridge || menubar->emulator->debugger->run) return;

    Debugger_Execute(menubar->emulator->debugger);
}

static void Debug_MemoryViewer(void *data){
    MenuBar *menubar = data;

    if(!menubar->emulator->nes.cartridge || menubar->emulator->memory_viewer->run) return;

    MemoryViewer_Execute(menubar->emulator->memory_viewer);
}

static void Debug_PPUViewer(void *data){
    MenuBar *menubar = data;

    if(!menubar->emulator->nes.cartridge || menubar->emulator->ppuv->run) return;

    PPUV_Execute(menubar->emulator->ppuv);
}

void Debug_Init(MenuBar *menubar){
    menubar->debug.button = CreateButton(L"Debug",menubar->emulator->atlas);

    menubar->debug.menu = malloc(sizeof(Menu));
    Menu_Init(
        menubar->debug.menu,
        menubar->emulator->window_class,
        menubar->emulator->window,
        menubar->emulator->renderer,
        menubar->emulator->atlas
    );

    Shortcut *debugger_shortcut = Menu_CreateShortcut(L"Ctrl+D",WG_KEYMOD_CTRL,WG_SCANCODE_D);
    menubar->debug.debugger = Menu_CreateOption(L"Debugger",debugger_shortcut,Debug_Debugger,menubar);
    Menu_InsertOption(menubar->debug.menu,menubar->debug.debugger);

    Shortcut *memory_viewer_shortcut = Menu_CreateShortcut(L"Ctrl+M",WG_KEYMOD_CTRL,WG_SCANCODE_M);
    menubar->debug.memory_viewer = Menu_CreateOption(L"Memory Viewer",memory_viewer_shortcut,Debug_MemoryViewer,menubar);
    Menu_InsertOption(menubar->debug.menu,menubar->debug.memory_viewer);

    Shortcut *ppu_viewer_shortcut = Menu_CreateShortcut(L"Ctrl+P",WG_KEYMOD_CTRL,WG_SCANCODE_P);
    menubar->debug.ppu_viewer = Menu_CreateOption(L"PPU Viewer",ppu_viewer_shortcut,Debug_PPUViewer,menubar);
    Menu_InsertOption(menubar->debug.menu,menubar->debug.ppu_viewer);
}


void MenuBar_Init(MenuBar *menubar,Emulator *emulator){
    memset(menubar,0,sizeof(MenuBar));
    menubar->emulator = emulator;
    File_Init(menubar);
    Debug_Init(menubar);
}

void MenuBar_ConfigRect(MenuBar *menubar){

    menubar->file.button->rect = (Rect){
        .x = menubar->rect.x,
        .y = menubar->rect.y,
        .w = menubar->file.button->s.size.x + BUTTON_SPACING * 2,
        .h = menubar->rect.h
    };

    ButtonConfigPosition(menubar->file.button);

    menubar->debug.button->rect = (Rect){
        .x = menubar->file.button->rect.x + menubar->file.button->rect.w,
        .y = menubar->rect.y,
        .w = menubar->debug.button->s.size.x + BUTTON_SPACING * 2,
        .h = menubar->rect.h
    };

    ButtonConfigPosition(menubar->debug.button);
}

static void MenuBar_Event(MenuBar *menubar){

    WG_Event event = menubar->emulator->event;
    Vector2 mouse_pos = menubar->emulator->mouse_pos;

    Menu_ShortcutEvent(menubar->file.menu,menubar->emulator->event);
    Menu_ShortcutEvent(menubar->debug.menu,menubar->emulator->event);

    if(PointInRect(mouse_pos,menubar->file.button->rect)){
        menubar->file.button->mouse.over = true;
    }
    else if(PointInRect(mouse_pos,menubar->debug.button->rect)){
        menubar->debug.button->mouse.over = true;
    }

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        if(menubar->file.button->mouse.over){
            menubar->file.button->mouse.pressed = true;
        }
        else if(menubar->debug.button->mouse.over){
            menubar->debug.button->mouse.pressed = true;
        }
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(menubar->file.button->mouse.pressed){

            if(menubar->file.button->mouse.over && !menubar->file.menu->open){
                
                Vector2 position = {
                    .x = menubar->file.button->rect.x,
                    .y = menubar->file.button->rect.y + menubar->file.button->rect.h
                };

                WG_ClientToScreen(menubar->emulator->window,&position.x,&position.y);

                Menu_OpenWindow(menubar->file.menu,position);
            }

            menubar->file.button->mouse.pressed = false;
        }
        else if(menubar->debug.button->mouse.pressed){

            if(menubar->debug.button->mouse.over && !menubar->debug.menu->open){

                Vector2 position = {
                    .x = menubar->debug.button->rect.x,
                    .y = menubar->debug.button->rect.y + menubar->debug.button->rect.h
                };

                WG_ClientToScreen(menubar->emulator->window,&position.x,&position.y);

                Menu_OpenWindow(menubar->debug.menu,position);
            }

            menubar->debug.button->mouse.pressed = false;
        }
    }
}

static void MenuBar_Draw(MenuBar *menubar){
    DrawButton(menubar->file.button,menubar->emulator->renderer);

    DrawButton(menubar->debug.button,menubar->emulator->renderer);
}

void MenuBar_Run(MenuBar *menubar){
    MenuBar_Event(menubar);
    MenuBar_Draw(menubar);
}

void MenuBar_RunChildrenWindow(MenuBar *menubar){
    if(menubar->file.menu->open){
        Menu_Run(menubar->file.menu);
    }

    if(menubar->debug.menu->open){
        Menu_Run(menubar->debug.menu);
    }
}

void MenuBar_Free(MenuBar *menubar){

    DestroyButton(menubar->file.button);
    Menu_Free(menubar->file.menu);

    DestroyButton(menubar->debug.button);
    Menu_Free(menubar->debug.menu);

    free(menubar);
}