#include <Debugger/Breakpoints/BreakpointEdit/Address.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Input.h>
#include <Utils.h>

static void BreakpointEdit_Address_SetStart(void *userdata,const uint16_t *content,uint64_t length){
    BreakpointEdit *breakpoint_edit = userdata;
    breakpoint_edit->address.start.value = wcstol(content,NULL,16);
    BreakpointEdit_Address_Validate(breakpoint_edit);
}

static void BreakpointEdit_Address_SetEnd(void *userdata,const uint16_t *content,uint64_t length){
    BreakpointEdit *breakpoint_edit = userdata;
    breakpoint_edit->address.end.value = wcstol(content,NULL,16);
    BreakpointEdit_Address_Validate(breakpoint_edit);
}

void BreakpointEdit_Address_Init(BreakpointEdit *breakpoint_edit){
    //Address
    breakpoint_edit->address.label = CreateLabel(L"Address:",breakpoint_edit->atlas);

    breakpoint_edit->address.start.input = CreateInput(
        INPUT_HEXADECIMAL | INPUT_VALIDATE | INPUT_RETURN_CONTENT,
        breakpoint_edit->atlas,
        BreakpointEdit_Address_SetStart,
        breakpoint_edit
    );

    breakpoint_edit->address.separation = CreateLabel(L"-",breakpoint_edit->atlas);

    breakpoint_edit->address.end.input = CreateInput(
        INPUT_HEXADECIMAL | INPUT_VALIDATE | INPUT_RETURN_CONTENT,
        breakpoint_edit->atlas,
        BreakpointEdit_Address_SetEnd,
        breakpoint_edit
    );

    breakpoint_edit->address.format = CreateLabel(L"(Format: Hex)",breakpoint_edit->atlas);

    breakpoint_edit->address.invalid_address_range = CreateLabel(L"Invalid address range",breakpoint_edit->atlas);
}

void BreakpointEdit_Address_Validate(BreakpointEdit *breakpoint_edit){
    if(breakpoint_edit->address.start.value > breakpoint_edit->address.end.value){
        breakpoint_edit->address.is_valid = false;
    }
    else if(breakpoint_edit->address.start.value >= breakpoint_edit->memory_type.length){
        breakpoint_edit->address.is_valid = false;
    }
    else if(breakpoint_edit->address.end.value >= breakpoint_edit->memory_type.length){
        breakpoint_edit->address.is_valid = false;
    }
    else{
        breakpoint_edit->address.is_valid = true;
    }
}

void BreakpointEdit_Address_Draw(BreakpointEdit *breakpoint_edit){
    WG_Renderer *renderer = breakpoint_edit->renderer;

    DrawLabel(renderer,breakpoint_edit->address.label,OFF_COLOR);

    DrawInput(
        breakpoint_edit->address.start.input,
        renderer,
        BACKGROUND1_COLOR,
        (breakpoint_edit->address.is_valid) ? BORDER_COLOR : ALERT_COLOR,
        ON_COLOR,
        OFF_COLOR
    );

    DrawLabel(renderer,breakpoint_edit->address.separation,OFF_COLOR);
    
    DrawInput(
        breakpoint_edit->address.end.input,
        renderer,
        BACKGROUND1_COLOR,
        (breakpoint_edit->address.is_valid) ? BORDER_COLOR : ALERT_COLOR,
        ON_COLOR,
        OFF_COLOR
    );

    DrawLabel(renderer,breakpoint_edit->address.format,OFF_COLOR);

    if(!breakpoint_edit->address.is_valid){
        DrawLabel(renderer,breakpoint_edit->address.invalid_address_range,ALERT_COLOR);
    }
}

void BreakpointEdit_Address_Free(BreakpointEdit *breakpoint_edit){
    DestroyLabel(breakpoint_edit->address.label);
    DestroyInput(breakpoint_edit->address.start.input);
    DestroyLabel(breakpoint_edit->address.separation);
    DestroyInput(breakpoint_edit->address.end.input);
    DestroyLabel(breakpoint_edit->address.format);
    DestroyLabel(breakpoint_edit->address.invalid_address_range);
}