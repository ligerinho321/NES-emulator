#include <Debugger/CallStack.h>
#include <Debugger/Metrics.h>
#include <Utils.h>

void CallStack_Init(DebuggerCallStack *callstack){
    callstack->expand_button = CreateExpandButton(
        L"Call Stack",
        callstack->debugger->resources.arrow,
        callstack->debugger->atlas
    );
    callstack->config_rect = CallStack_ConfigRect;
}

void CallStack_ConfigRect(DebuggerCallStack *callstack){

    callstack->expand_button->rect = (Rect){
        .x = callstack->rect.x,
        .y = callstack->rect.y,
        .w = callstack->rect.w,
        .h = ELEMENT_HEIGHT
    };

    ExpandButtonConfigRect(callstack->expand_button);
}

static void CallStack_MouseOver(DebuggerCallStack *callstack){

    Vector2 mouse = callstack->debugger->mouse;

    if(PointInRect(mouse,callstack->expand_button->rect)){
        callstack->expand_button->mouse.over = true;
    }
}

static void CallStack_MouseButtonDown(DebuggerCallStack *callstack){

    if(callstack->expand_button->mouse.over){
        callstack->expand_button->mouse.pressed = true;
    }
}

static void CallStack_MouseButtonUp(DebuggerCallStack *callstack){

    if(callstack->expand_button->mouse.pressed){

        if(callstack->expand_button->mouse.over){
            callstack->expand_button->open = !callstack->expand_button->open;
            ConfigMetricLeft(callstack->debugger);
        }

        callstack->expand_button->mouse.pressed = false;
    }
}

void CallStack_Event(DebuggerCallStack *callstack){
    WG_Event event = callstack->debugger->event;

    CallStack_MouseOver(callstack);

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        CallStack_MouseButtonDown(callstack);
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        CallStack_MouseButtonUp(callstack);
    }
}

void CallStack_Draw(DebuggerCallStack *callstack){
    WG_Renderer *renderer = callstack->debugger->renderer;
    DrawExpandButton(callstack->expand_button,renderer);
}

void CallStack_Run(DebuggerCallStack *callstack){
    if(!callstack->debugger->metrics.left.open) return;

    CallStack_Event(callstack);

    CallStack_Draw(callstack);
}

void CallStack_Free(DebuggerCallStack *callstack){
    DestroyExpandButton(callstack->expand_button);
}