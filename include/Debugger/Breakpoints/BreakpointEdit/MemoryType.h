#pragma once

#include <Emulator.h>

typedef struct _BreakpointEdit BreakpointEdit;

void BreakpointEdit_MemoryType_Init(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_MemoryType_SetShowStrings(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_MemoryType_Free(BreakpointEdit *breakpoint_edit);
