#include <Debugger/Breakpoints/BreakpointEdit/Condition.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Input.h>
#include <Utils.h>
#include <SyntacticAnalyzer.h>

static void BreakpointEdit_Condition_Validate(void *userdata,const uint16_t *content,uint64_t length){
    BreakpointEdit *breakpoint_edit = userdata;
    if(ContentSize(breakpoint_edit->condition.input->gap_buffer,uint16_t)){
        breakpoint_edit->condition.is_valid = SyntacticAnalyzer_InfixToPostfix(breakpoint_edit->condition.input->gap_buffer,&breakpoint_edit->condition.postfix);
    }
    else{
        breakpoint_edit->condition.is_valid = true;
    }
}

void BreakpointEdit_Condition_Init(BreakpointEdit *breakpoint_edit){

    breakpoint_edit->condition.label = CreateLabel(L"Condition:",breakpoint_edit->atlas);

    breakpoint_edit->condition.input = CreateInput(
        INPUT_ANY | INPUT_VALIDATE,
        breakpoint_edit->atlas,
        BreakpointEdit_Condition_Validate,
        breakpoint_edit
    );

    SyntacticAnalyzer_InitPostfix(&breakpoint_edit->condition.postfix);

    breakpoint_edit->condition.help = CreateIconButton(breakpoint_edit->resources.help,L"Help",breakpoint_edit->atlas);

    breakpoint_edit->condition.invalid_expression = CreateLabel(L"Invalid expression",breakpoint_edit->atlas);
}

void BreakpointEdit_Condition_Draw(BreakpointEdit *breakpoint_edit){
    WG_Renderer *renderer = breakpoint_edit->renderer;

    DrawLabel(renderer,breakpoint_edit->condition.label,OFF_COLOR);

    DrawInput(
        breakpoint_edit->condition.input,
        renderer,
        BACKGROUND1_COLOR,
        (breakpoint_edit->condition.is_valid) ? BORDER_COLOR : ALERT_COLOR,
        ON_COLOR,
        OFF_COLOR
    );

    DrawIconButton(breakpoint_edit->condition.help,renderer);
    
    if(!breakpoint_edit->condition.is_valid){
        DrawLabel(renderer,breakpoint_edit->condition.invalid_expression,ALERT_COLOR);
    }
}

void BreakpointEdit_Condition_Free(BreakpointEdit *breakpoint_edit){
    DestroyLabel(breakpoint_edit->condition.label);
    DestroyInput(breakpoint_edit->condition.input);
    utarray_done(&breakpoint_edit->condition.postfix);
    DestroyIconButton(breakpoint_edit->condition.help);
    DestroyLabel(breakpoint_edit->condition.invalid_expression);
}