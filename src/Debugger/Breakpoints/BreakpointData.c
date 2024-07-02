#include <Debugger/Breakpoints/BreakpointData.h>
#include <SyntacticAnalyzer.h>

bool BreakpointsData_FindCondition(DebuggerBreakpoints *debugger_breakpoints,uint32_t memory_type,uint32_t break_flag,Range address){
    
    WaitForSingleObject(debugger_breakpoints->debugger->breakpoint_mutex,INFINITE);

    bool result = false;

    if(debugger_breakpoints->list.breakpoints.data){

        BreakpointData *breakpoint_data = debugger_breakpoints->list.breakpoints.data;

        while(breakpoint_data){

            bool is_enabled = breakpoint_data->enabled;

            bool is_correct_memory_type = breakpoint_data->memory_type == memory_type;

            bool is_correct_break_flag = breakpoint_data->break_flags & break_flag;

            bool is_address_within_range = false;
            if(
                breakpoint_data->address.start >= address.start && breakpoint_data->address.end <= address.end ||
                address.start >= breakpoint_data->address.start && address.end <= breakpoint_data->address.end
            ){
                is_address_within_range = true;
            }

            bool is_condition_satisfied = !utarray_len(&breakpoint_data->condition.postfix) || SyntacticAnalyzer_GetPostfixResult(&breakpoint_data->condition.postfix);

            if(is_enabled && is_correct_memory_type && is_correct_break_flag && is_address_within_range && is_condition_satisfied){
                result = true;
                break;
            }

            breakpoint_data = breakpoint_data->next;
        }
    }

    ReleaseMutex(debugger_breakpoints->debugger->breakpoint_mutex);

    return result;
}

BreakpointData* BreakpointsData_Insert(DebuggerBreakpoints *debugger_breakpoints){
    BreakpointData *breakpoint_data = calloc(1,sizeof(BreakpointData));

    SyntacticAnalyzer_InitPostfix(&breakpoint_data->condition.postfix);

    WaitForSingleObject(debugger_breakpoints->debugger->breakpoint_mutex,INFINITE);

    if(debugger_breakpoints->list.breakpoints.data){
        BreakpointData *buffer = debugger_breakpoints->list.breakpoints.data;
        while(buffer->next){buffer = buffer->next;}
        buffer->next = breakpoint_data;
    }
    else{
        debugger_breakpoints->list.breakpoints.data = breakpoint_data;
    }

    ReleaseMutex(debugger_breakpoints->debugger->breakpoint_mutex);

    return breakpoint_data;
}

void BreakpointsData_Remove(DebuggerBreakpoints *debugger_breakpoints,BreakpointData *breakpoint_data){
    
    WaitForSingleObject(debugger_breakpoints->debugger->breakpoint_mutex,INFINITE);

    if(debugger_breakpoints->list.breakpoints.data == breakpoint_data){
        debugger_breakpoints->list.breakpoints.data = breakpoint_data->next;
    }
    else{
        BreakpointData *buffer = debugger_breakpoints->list.breakpoints.data;
        while(buffer && buffer->next != breakpoint_data){buffer = buffer->next;}
        if(buffer){
            buffer->next = breakpoint_data->next;
        }
        else{
            printf("%s: breakpoint data not found\n",__func__);
        }
    }

    ReleaseMutex(debugger_breakpoints->debugger->breakpoint_mutex);
    
    BreakpointData_Destroy(breakpoint_data);
}

void BreakpointData_Destroy(BreakpointData *breakpoint_data){
    free(breakpoint_data->condition.infix);
    utarray_done(&breakpoint_data->condition.postfix);
    free(breakpoint_data);
}

void BreakpointsData_Free(DebuggerBreakpoints *debugger_breakpoints){
    BreakpointData *breakpoint_data = debugger_breakpoints->list.breakpoints.data;
    BreakpointData *buffer = NULL;
    while(breakpoint_data){
        buffer = breakpoint_data->next;
        BreakpointData_Destroy(breakpoint_data);
        breakpoint_data = buffer;
    }
}