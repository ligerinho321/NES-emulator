#pragma once

#include <Emulator.h>
#include <WG/UT/UTarray.h>

typedef struct _BreakpointData{
    bool enabled;
    uint32_t memory_type;
    uint32_t break_flags;
    Range address;
    struct{
        uint16_t *infix;
        UT_array postfix;
    }condition;
    struct _BreakpointData *next;
}BreakpointData;

bool BreakpointsData_FindCondition(DebuggerBreakpoints *debugger_breakpoints,uint32_t memory_type,uint32_t break_flag,Range address);

BreakpointData* BreakpointsData_Insert(DebuggerBreakpoints *debugger_breakpoints);

void BreakpointsData_Remove(DebuggerBreakpoints *debugger_breakpoints,BreakpointData *breakpoint_data);

void BreakpointData_Destroy(BreakpointData *breakpoint_data);

void BreakpointsData_Free(DebuggerBreakpoints *debugger_breakpoints);
