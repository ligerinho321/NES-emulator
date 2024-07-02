#include <Debugger/Labels.h>
#include <Debugger/Metrics.h>
#include <Utils.h>

void Labels_Init(DebuggerLabels *labels){
    labels->expand_button = CreateExpandButton(
        L"Labels",
        labels->debugger->resources.arrow,
        labels->debugger->atlas
    );
    
    labels->config_rect = Labels_ConfigRect;
}

void Labels_ConfigRect(DebuggerLabels *labels){

    labels->expand_button->rect = (Rect){
        .x = labels->rect.x,
        .y = labels->rect.y,
        .w = labels->rect.w,
        .h = ELEMENT_HEIGHT
    };

    ExpandButtonConfigRect(labels->expand_button);
}

static void Labels_MouseOver(DebuggerLabels *labels){
    Vector2 mouse = labels->debugger->mouse;

    if(PointInRect(mouse,labels->expand_button->rect)){
        labels->expand_button->mouse.over = true;
    }
}

static void Labels_MouseButtonDown(DebuggerLabels *labels){

    if(labels->expand_button->mouse.over){
        labels->expand_button->mouse.pressed = true;
    }
}

static void Labels_MouseButtonUp(DebuggerLabels *labels){

    if(labels->expand_button->mouse.pressed){

        if(labels->expand_button->mouse.over){
            labels->expand_button->open = !labels->expand_button->open;
            ConfigMetricLeft(labels->debugger);
        }

        labels->expand_button->mouse.pressed = false;
    }
}

void Labels_Event(DebuggerLabels *labels){
    if(!labels->debugger->metrics.left.open) return;

    WG_Event event = labels->debugger->event;

    Labels_MouseOver(labels);

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Labels_MouseButtonDown(labels);
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        Labels_MouseButtonUp(labels);
    }
}

void Labels_Draw(DebuggerLabels *labels){
    WG_Renderer *renderer = labels->debugger->renderer;
    DrawExpandButton(labels->expand_button,renderer);
}

void Labels_Run(DebuggerLabels *labels){
    if(!labels->debugger->metrics.left.open) return;

    Labels_Event(labels);

    Labels_Draw(labels);
}

void Labels_Free(DebuggerLabels *labels){
    DestroyExpandButton(labels->expand_button);
}