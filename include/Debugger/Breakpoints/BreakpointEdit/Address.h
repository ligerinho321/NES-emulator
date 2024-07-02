#pragma once

#include <Emulator.h>

typedef struct _BreakpointEdit BreakpointEdit;

void BreakpointEdit_Address_Init(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Address_Validate(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Address_Draw(BreakpointEdit *breakpoint_edit);

void BreakpointEdit_Address_Free(BreakpointEdit *breakpoint_edit);