#pragma once

#include <Emulator.h>
#include <Debugger/Breakpoints/Breakpointdata.h>

typedef struct _BreakpointView{
    BreakpointData *data;
    ToggleButton *enabled;
    Label *description;
    struct{
        bool over;
        bool left_pressed;
        bool right_pressed;
    }mouse;
    Rect rect;
    struct _BreakpointView *next;
}BreakpointView;

void BreakpointsView_Init(DebuggerBreakpoints *debugger_breakpoints);

BreakpointView* BreakpointsView_Insert(DebuggerBreakpoints *debugger_breakpoints);

void BreakpointsView_Remove(DebuggerBreakpoints *debugger_breakpoints);

void BreakpointsView_ConfigVerticalScrollBar(DebuggerBreakpoints *debugger_breakpoints);

void BreakpointsView_ConfigRect(DebuggerBreakpoints *debugger_breakpoints);

void BreakpointView_Update(DebuggerBreakpoints *debugger_breakpoints,BreakpointView *breakpoint_view);

void BreakpointView_Destroy(DebuggerBreakpoints *debugger_breakpoints,BreakpointView *breakpoint_view,bool destroy_data);

void BreakpointsView_Free(DebuggerBreakpoints *debugger_breakpoints);