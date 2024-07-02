#include <Debugger/Status.h>
#include <Debugger/Metrics.h>
#include <Tree/Tree.h>
#include <Tree/CPUTree.h>
#include <Tree/PPUTree.h>
#include <Tree/APUTree/APUTree.h>
#include <Utils.h>

void Status_Init(DebuggerStatus *status){
    status->nes = status->debugger->nes;

    status->tree_resources = malloc(sizeof(TreeResources));

    status->tree_resources->arrow = status->debugger->resources.arrow;
    status->tree_resources->branch1 = status->debugger->resources.branch1;
    status->tree_resources->branch2 = status->debugger->resources.branch2;
    status->tree_resources->branch3 = status->debugger->resources.branch3;
    status->tree_resources->circle = status->debugger->resources.circle;

    status->cpu.expand_button = CreateExpandButton(L"CPU",status->debugger->resources.arrow,status->debugger->atlas);
    status->cpu.tree = CPUTree_Create(&status->debugger->emulator->nes.cpu,status->tree_resources,status->debugger->atlas);

    status->ppu.expand_button = CreateExpandButton(L"PPU",status->debugger->resources.arrow,status->debugger->atlas);
    status->ppu.tree = PPUTree_Create(&status->debugger->emulator->nes.ppu,status->tree_resources,status->debugger->atlas);

    status->apu.expand_button = CreateExpandButton(L"APU",status->debugger->resources.arrow,status->debugger->atlas);
    status->apu.tree = APUTree_Create(&status->debugger->emulator->nes.apu,status->tree_resources,status->debugger->atlas);

    status->config_rect = Status_ConfigRect;
}

void Status_ConfigRect(DebuggerStatus *status){

    Rect rect = status->debugger->metrics.right.rect;

    int open_counter = 0;
    if(status->cpu.expand_button->open) ++open_counter;
    if(status->ppu.expand_button->open) ++open_counter;
    if(status->apu.expand_button->open) ++open_counter;

    int height = 0;

    if(open_counter > 0){
        height = (rect.h - (ELEMENT_HEIGHT * (3 - open_counter))) / open_counter;
    }

    status->cpu.expand_button->rect = (Rect){
        .x = rect.x,
        .y = rect.y,
        .w = rect.w,
        .h = ELEMENT_HEIGHT,
    };

    ExpandButtonConfigRect(status->cpu.expand_button);

    status->cpu.tree->tree->rect = (Rect){
        .x = status->cpu.expand_button->rect.x,
        .y = status->cpu.expand_button->rect.y + status->cpu.expand_button->rect.h,
        .w = status->cpu.expand_button->rect.w,
        .h = (status->cpu.expand_button->open) ? height - ELEMENT_HEIGHT : 0
    };

    /*if(status->cpu.expand_button->open)*/ Tree_ConfigRect(status->cpu.tree->tree);


    status->ppu.expand_button->rect = (Rect){
        .x = rect.x,
        .y = status->cpu.tree->tree->rect.y + status->cpu.tree->tree->rect.h,
        .w = rect.w,
        .h = ELEMENT_HEIGHT
    };

    ExpandButtonConfigRect(status->ppu.expand_button);

    status->ppu.tree->tree->rect = (Rect){
        .x = status->ppu.expand_button->rect.x,
        .y = status->ppu.expand_button->rect.y + status->ppu.expand_button->rect.h,
        .w = status->ppu.expand_button->rect.w,
        .h = (status->ppu.expand_button->open) ? height - ELEMENT_HEIGHT : 0
    };

    /*if(status->ppu.expand_button->open)*/ Tree_ConfigRect(status->ppu.tree->tree);


    status->apu.expand_button->rect = (Rect){
        .x = rect.x,
        .y = status->ppu.tree->tree->rect.y + status->ppu.tree->tree->rect.h,
        .w = rect.w,
        .h = ELEMENT_HEIGHT
    };

    ExpandButtonConfigRect(status->apu.expand_button);

    status->apu.tree->tree->rect = (Rect){
        .x = status->apu.expand_button->rect.x,
        .y = status->apu.expand_button->rect.y + status->apu.expand_button->rect.h,
        .w = status->apu.expand_button->rect.w,
        .h = (status->apu.expand_button->open) ? height - ELEMENT_HEIGHT : 0
    };

    /*if(status->apu.expand_button->open)*/ Tree_ConfigRect(status->apu.tree->tree);
}

static void Status_MouseOver(DebuggerStatus *status){

    Vector2 mouse = status->debugger->mouse;

    if(PointInRect(mouse,status->cpu.expand_button->rect)){
        status->cpu.expand_button->mouse.over = true;
    }
    else if(PointInRect(mouse,status->ppu.expand_button->rect)){
        status->ppu.expand_button->mouse.over = true;
    }
    else if(PointInRect(mouse,status->apu.expand_button->rect)){
        status->apu.expand_button->mouse.over = true;   
    }
}

static void Status_MouseButtonDown(DebuggerStatus *status){

    if(status->cpu.expand_button->mouse.over){
        status->cpu.expand_button->mouse.pressed = true;
    }
    else if(status->ppu.expand_button->mouse.over){
        status->ppu.expand_button->mouse.pressed = true;
    }
    else if(status->apu.expand_button->mouse.over){
        status->apu.expand_button->mouse.pressed = true;
    }
}

static void Status_MouseButtonUp(DebuggerStatus *status){
    
    if(status->cpu.expand_button->mouse.pressed){

        if(status->cpu.expand_button->mouse.over){
            status->cpu.expand_button->open = !status->cpu.expand_button->open;
            ConfigMetricRight(status->debugger);
        }

        status->cpu.expand_button->mouse.pressed = false;
    }
    else if(status->ppu.expand_button->mouse.pressed){

        if(status->ppu.expand_button->mouse.over){
            status->ppu.expand_button->open = !status->ppu.expand_button->open;
            ConfigMetricRight(status->debugger);
        }

        status->ppu.expand_button->mouse.pressed = false;
    }
    else if(status->apu.expand_button->mouse.pressed){

        if(status->apu.expand_button->mouse.over){
            status->apu.expand_button->open = !status->apu.expand_button->open;
            ConfigMetricRight(status->debugger);
        }

        status->apu.expand_button->mouse.pressed = false;
    }
}

void Status_Event(DebuggerStatus *status){
    
    if(!status->debugger->metrics.right.open) return;
    
    WG_Event event = status->debugger->event;
    Vector2 mouse = status->debugger->mouse;

    Status_MouseOver(status);

    //if(status->debugger->nes->paused){

        /*if(status->cpu.expand_button->open)*/ Tree_Event(status->cpu.tree->tree,event,mouse);
        
        /*if(status->ppu.expand_button->open)*/ Tree_Event(status->ppu.tree->tree,event,mouse);
        
        /*if(status->apu.expand_button->open)*/ Tree_Event(status->apu.tree->tree,event,mouse);
    //}

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Status_MouseButtonDown(status);
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Status_MouseButtonUp(status);
    }
}

void Status_Draw(DebuggerStatus *status){

    WG_Renderer *renderer = status->debugger->renderer;

    DrawExpandButton(status->cpu.expand_button,renderer);

    //if(status->cpu.expand_button->open && status->debugger->nes->paused){
        Tree_Draw(status->cpu.tree->tree,renderer);
    //}

    DrawExpandButton(status->ppu.expand_button,renderer);

    //if(status->ppu.expand_button->open && status->debugger->nes->paused){
        Tree_Draw(status->ppu.tree->tree,renderer);
    //}
    
    DrawExpandButton(status->apu.expand_button,renderer);

    //if(status->apu.expand_button->open && status->debugger->nes->paused){
        Tree_Draw(status->apu.tree->tree,renderer);
    //}
}

void Status_Run(DebuggerStatus *status){
    
    Status_Event(status);

    Status_Draw(status);
}

void Status_Free(DebuggerStatus *status){
    
    DestroyExpandButton(status->cpu.expand_button);
    CPUTree_Free(status->cpu.tree);

    DestroyExpandButton(status->ppu.expand_button);
    PPUTree_Free(status->ppu.tree);

    DestroyExpandButton(status->apu.expand_button);
    APUTree_Free(status->apu.tree);

    free(status->tree_resources);
}