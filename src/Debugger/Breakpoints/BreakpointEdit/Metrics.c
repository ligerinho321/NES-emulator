#include <Debugger/Breakpoints/BreakpointEdit/Metrics.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Utils.h>
#include <ComboBox/ComboBox.h>
#include <Input.h>

static void BreakpointEdit_Row_0_ConfigRect(BreakpointEdit *breakpoint_edit){

    int row_0 = breakpoint_edit->window_size.y + ELEMENT_ROW_HEIGHT * 0.5f - ELEMENT_HEIGHT * 0.5f;
    int width = 0;

    //Memory Type
    breakpoint_edit->memory_type.label->rect = (Rect){
        .x = 0,
        .y = row_0,
        .w = breakpoint_edit->memory_type.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->memory_type.label);

    width += breakpoint_edit->memory_type.label->rect.w;


    //ComboBox
    Vector2 position = {
        .x = breakpoint_edit->memory_type.label->rect.x + breakpoint_edit->memory_type.label->rect.w,
        .y = row_0
    };

    ComboBox_ConfigRect(breakpoint_edit->memory_type.combo_box,position);

    width += breakpoint_edit->memory_type.combo_box->expand_button.rect.w;


    //Max
    breakpoint_edit->memory_type.max->rect = (Rect){
        .x = breakpoint_edit->memory_type.combo_box->expand_button.rect.x + breakpoint_edit->memory_type.combo_box->expand_button.rect.w,
        .y = row_0,
        .w = breakpoint_edit->memory_type.max->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->memory_type.max);

    width += breakpoint_edit->memory_type.max->rect.w;


    breakpoint_edit->window_size.x = max(breakpoint_edit->window_size.x,width);
    breakpoint_edit->window_size.y += ELEMENT_ROW_HEIGHT;
}

static void BreakpointEdit_Row_1_ConfigRect(BreakpointEdit *breakpoint_edit){

    int row_1 = breakpoint_edit->window_size.y + ELEMENT_ROW_HEIGHT * 0.5f - ELEMENT_HEIGHT * 0.5f;
    int width = 0;

    //Break Flags
    breakpoint_edit->break_flags.label->rect = (Rect){
        .x = 0,
        .y = row_1,
        .w = breakpoint_edit->break_flags.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->break_flags.label);

    width += breakpoint_edit->break_flags.label->rect.w;


    //Write
    breakpoint_edit->break_flags.write.check_button->rect = (Rect){
        .x = breakpoint_edit->break_flags.label->rect.x + breakpoint_edit->break_flags.label->rect.w,
        .y = row_1,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    width += breakpoint_edit->break_flags.write.check_button->rect.w;

    breakpoint_edit->break_flags.write.label->rect = (Rect){
        .x = breakpoint_edit->break_flags.write.check_button->rect.x + breakpoint_edit->break_flags.write.check_button->rect.w,
        .y = row_1,
        .w = breakpoint_edit->break_flags.write.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->break_flags.write.label);

    width += breakpoint_edit->break_flags.write.label->rect.w;


    //Read
    breakpoint_edit->break_flags.read.check_button->rect = (Rect){
        .x = breakpoint_edit->break_flags.write.label->rect.x + breakpoint_edit->break_flags.write.label->rect.w,
        .y = row_1,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    width += breakpoint_edit->break_flags.read.check_button->rect.w;

    breakpoint_edit->break_flags.read.label->rect = (Rect){
        .x = breakpoint_edit->break_flags.read.check_button->rect.x + breakpoint_edit->break_flags.read.check_button->rect.w,
        .y = row_1,
        .w = breakpoint_edit->break_flags.read.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->break_flags.read.label);

    width += breakpoint_edit->break_flags.read.label->rect.w;


    //Execute
    breakpoint_edit->break_flags.execute.check_button->rect = (Rect){
        .x = breakpoint_edit->break_flags.read.label->rect.x + breakpoint_edit->break_flags.read.label->rect.w,
        .y = row_1,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    width += breakpoint_edit->break_flags.execute.check_button->rect.w;

    breakpoint_edit->break_flags.execute.label->rect = (Rect){
        .x = breakpoint_edit->break_flags.execute.check_button->rect.x + breakpoint_edit->break_flags.execute.check_button->rect.w,
        .y = row_1,
        .w = breakpoint_edit->break_flags.execute.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->break_flags.execute.label);

    width += breakpoint_edit->break_flags.execute.label->rect.w;


    breakpoint_edit->window_size.x = max(breakpoint_edit->window_size.x,width);
    breakpoint_edit->window_size.y += ELEMENT_ROW_HEIGHT;
}

static void BreakpointEdit_Row_2_3_ConfigRect(BreakpointEdit *breakpoint_edit){

    int row_2 = breakpoint_edit->window_size.y + ELEMENT_ROW_HEIGHT * 0.5f - ELEMENT_HEIGHT * 0.5f;
    int width = 0;

    //Address
    breakpoint_edit->address.label->rect = (Rect){
        .x = 0,
        .y = row_2,
        .w = breakpoint_edit->address.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->address.label);

    width += breakpoint_edit->address.label->rect.w;


    //Start
    breakpoint_edit->address.start.input->rect = (Rect){
        .x = breakpoint_edit->address.label->rect.x + breakpoint_edit->address.label->rect.w,
        .y = row_2,
        .w = breakpoint_edit->glyph_size.x * 9 + 10,
        .h = ELEMENT_HEIGHT
    };

    width += breakpoint_edit->address.start.input->rect.w;


    //Separation
    breakpoint_edit->address.separation->rect = (Rect){
        .x = breakpoint_edit->address.start.input->rect.x + breakpoint_edit->address.start.input->rect.w,
        .y = row_2,
        .w = breakpoint_edit->address.separation->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->address.separation);

    width += breakpoint_edit->address.separation->rect.w;


    //End
    breakpoint_edit->address.end.input->rect = (Rect){
        .x = breakpoint_edit->address.separation->rect.x + breakpoint_edit->address.separation->rect.w,
        .y = row_2,
        .w = breakpoint_edit->glyph_size.x * 9 + 10,
        .h = ELEMENT_HEIGHT
    };

    width += breakpoint_edit->address.end.input->rect.w;


    //Format
    breakpoint_edit->address.format->rect = (Rect){
        .x = breakpoint_edit->address.end.input->rect.x + breakpoint_edit->address.end.input->rect.w,
        .y = row_2,
        .w = breakpoint_edit->address.format->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->address.format);

    width += breakpoint_edit->address.format->rect.w;


    //Invalid address range
    breakpoint_edit->address.invalid_address_range->rect = (Rect){
        .x = breakpoint_edit->address.start.input->rect.x,
        .y = breakpoint_edit->address.start.input->rect.y + breakpoint_edit->address.start.input->rect.h,
        .w = breakpoint_edit->address.invalid_address_range->size.x + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->address.invalid_address_range);
    

    breakpoint_edit->window_size.x = max(breakpoint_edit->window_size.x,width);
    breakpoint_edit->window_size.y += ELEMENT_ROW_HEIGHT * 2;
}

static void BreakpointEdit_Row_4_5_ConfigRect(BreakpointEdit *breakpoint_edit){

    int row_3 = breakpoint_edit->window_size.y + ELEMENT_ROW_HEIGHT * 0.5f - ELEMENT_HEIGHT * 0.5f;
    int width = 0;

    //Condition
    breakpoint_edit->condition.label->rect = (Rect){
        .x = 0,
        .y = row_3,
        .w = breakpoint_edit->condition.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->condition.label);

    width += breakpoint_edit->condition.label->rect.w;


    //Input
    breakpoint_edit->condition.input->rect = (Rect){
        .x = breakpoint_edit->condition.label->rect.x + breakpoint_edit->condition.label->rect.w,
        .y = row_3,
        .w = breakpoint_edit->glyph_size.x * 30 + 10,
        .h = ELEMENT_HEIGHT
    };

    width += breakpoint_edit->condition.input->rect.w;


    //Help
    breakpoint_edit->condition.help->button.rect = (Rect){
        .x = breakpoint_edit->condition.input->rect.x + breakpoint_edit->condition.input->rect.w,
        .y = row_3,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    width += breakpoint_edit->condition.help->button.rect.w;


    //Invalid Expression
    breakpoint_edit->condition.invalid_expression->rect = (Rect){
        .x = breakpoint_edit->condition.input->rect.x,
        .y = breakpoint_edit->condition.input->rect.y + breakpoint_edit->condition.input->rect.h,
        .w = breakpoint_edit->condition.invalid_expression->size.x + BORDER_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->condition.invalid_expression);

    
    breakpoint_edit->window_size.x = max(breakpoint_edit->window_size.x,width);
    breakpoint_edit->window_size.y += ELEMENT_ROW_HEIGHT * 2;
}

static void BreakpointEdit_Row_6_ConfigRect(BreakpointEdit *breakpoint_edit){

    int row_5 = breakpoint_edit->window_size.y + ELEMENT_ROW_HEIGHT * 0.5f - ELEMENT_HEIGHT * 0.5f;

    //Enabled
    breakpoint_edit->enabled.check_button->rect = (Rect){
        .x = 0,
        .y = row_5,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    breakpoint_edit->enabled.label->rect = (Rect){
        .x = breakpoint_edit->enabled.check_button->rect.x + breakpoint_edit->enabled.check_button->rect.w,
        .y = row_5,
        .w = breakpoint_edit->enabled.label->size.x + 10,
        .h = ELEMENT_HEIGHT
    };

    LabelConfigPosition(breakpoint_edit->enabled.label);


    //Cancel
    breakpoint_edit->cancel->rect = (Rect){
        .x = breakpoint_edit->window_size.x - (breakpoint_edit->cancel->s.size.x + BUTTON_SPACING * 2),
        .y = row_5,
        .w = breakpoint_edit->cancel->s.size.x + BUTTON_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    ButtonConfigPosition(breakpoint_edit->cancel);


    //Ok
    breakpoint_edit->ok->rect = (Rect){
        .x = breakpoint_edit->cancel->rect.x - (breakpoint_edit->ok->s.size.x + BUTTON_SPACING * 2),
        .y = row_5,
        .w = breakpoint_edit->ok->s.size.x + BUTTON_SPACING * 2,
        .h = ELEMENT_HEIGHT
    };

    ButtonConfigPosition(breakpoint_edit->ok);


    breakpoint_edit->window_size.y += ELEMENT_ROW_HEIGHT;
}

void BreakpointEdit_ConfigRect(BreakpointEdit *breakpoint_edit){
    
    breakpoint_edit->window_size = (Rect){
        .x = 0,
        .y = 0
    };

    //Row 0
    BreakpointEdit_Row_0_ConfigRect(breakpoint_edit);

    //Row 1
    BreakpointEdit_Row_1_ConfigRect(breakpoint_edit);

    //Row 2
    BreakpointEdit_Row_2_3_ConfigRect(breakpoint_edit);

    //Row 3
    BreakpointEdit_Row_4_5_ConfigRect(breakpoint_edit);

    //Row 5
    BreakpointEdit_Row_6_ConfigRect(breakpoint_edit);


    WG_SetWindowClientSize(breakpoint_edit->window,breakpoint_edit->window_size.x,breakpoint_edit->window_size.y);
}