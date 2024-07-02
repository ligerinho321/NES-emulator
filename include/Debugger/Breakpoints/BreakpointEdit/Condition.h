#pragma once

#include <Emulator.h>

typedef struct _BreakpointEdit BreakpointEdit;

void BreakpointEdit_Condition_Init(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Condition_Draw(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Condition_Free(BreakpointEdit *breakpoint_edit);