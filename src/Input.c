#include <Input.h>
#include <Utils.h>

Input* CreateInput(uint32_t flags,WG_FontAtlas *atlas,Input_Callback callback,void *userdata){
    Input *input = calloc(1,sizeof(Input));
    input->max_length = -1;
    input->flags = flags;
    CreateGapBuffer(input->gap_buffer,uint16_t,20);
    input->atlas = atlas;
    WG_SizeTextByFontAtlas(atlas,L"A",&input->glyph_size.x,&input->glyph_size.y);
    input->callback = callback;
    input->userdata = userdata;
    return input;
}

void UpdateInputCursor(Input *input){

    if(input->cursor.update){
        input->cursor.update = false;
        input->cursor.visible = true;
        input->cursor.time = GetTickCount();
    }
    else if(GetTickCount() - input->cursor.time >= BLINK_TIME){
        input->cursor.visible = !input->cursor.visible;
        input->cursor.time = GetTickCount();
    }
}

void SetInputOffset(Input *input){

    if((input->cursor.index + 1) * input->glyph_size.x + input->offset > (input->rect.w - BORDER_SPACING * 2)){
        input->offset -= ((input->cursor.index + 1) * input->glyph_size.x + input->offset) - (input->rect.w - BORDER_SPACING * 2);
    }
    else if(input->cursor.index * input->glyph_size.x + input->offset < 0){
        input->offset -= input->cursor.index * input->glyph_size.x + input->offset;
    }

    uint32_t content_size = ContentSize(input->gap_buffer,uint16_t);

    if((content_size + 1) * input->glyph_size.x > (input->rect.w - BORDER_SPACING * 2) && (content_size + 1) * input->glyph_size.x + input->offset < (input->rect.w - BORDER_SPACING * 2)){
        input->offset = (input->rect.w - BORDER_SPACING * 2) - (content_size + 1) * input->glyph_size.x;
    }
    else if(content_size * input->glyph_size.x < (input->rect.w - BORDER_SPACING * 2) && input->offset < 0){
        input->offset = 0;
    }
}

void DrawInput(Input *input,WG_Renderer *renderer,Color background,Color border,Color on,Color off){

    WG_RendererSetViewport(renderer,NULL,&input->rect);

    WG_DrawRectangle(renderer,NULL,border);

    Vector2 position;

    position.x = BORDER_SPACING + input->offset;
    position.y = input->rect.h * 0.5f - input->glyph_size.y * 0.5f;

    if(input->on){
        
        UpdateInputCursor(input);

        WG_DrawTextByGapBuffer(renderer,input->atlas,input->gap_buffer,position,on);

        if(input->cursor.visible){

            Rect rect = {
                .x = 5 + input->cursor.index * input->glyph_size.x + input->offset,
                .y = position.y,
                .w = input->glyph_size.x,
                .h = input->glyph_size.y
            };

            WG_DrawFilledRectangle(renderer,&rect,on);

            if(input->cursor.index < ContentSize(input->gap_buffer,uint16_t)){

                position.x = rect.x;
                position.y = rect.y;

                WG_DrawNText(renderer,input->atlas,input->gap_buffer->gap_end,1,position,background);
            }
        }
    }
    else{
        if(input->mouse_over){
            WG_DrawTextByGapBuffer(renderer,input->atlas,input->gap_buffer,position,on);
        }
        else{
            WG_DrawTextByGapBuffer(renderer,input->atlas,input->gap_buffer,position,off);
        }
    }
    
    input->mouse_over = false;

    WG_RendererSetViewport(renderer,NULL,NULL);
}

static void InputCallback(Input *input){
    if(!input->callback) return;

    if(input->flags & INPUT_RETURN_CONTENT){

        uint16_t *content = NULL;

        GapBufferGetContent(input->gap_buffer,uint16_t,content);

        uint64_t length = ContentSize(input->gap_buffer,uint16_t);

        input->callback(input->userdata,content,length);

        free(content);
    }
    else{
        input->callback(input->userdata,NULL,0);
    }
}

void InputEvent(Input *input,WG_Event event,Vector2 mouse_pos){

    if(PointInRect(mouse_pos,input->rect)){
        input->mouse_over = true;
    }

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){

        if(input->mouse_over){

            if(input->on){
                
                int x = round((mouse_pos.x - input->rect.x - BORDER_SPACING - input->offset) / (float)input->glyph_size.x);
                
                uint64_t content_size = ContentSize(input->gap_buffer,uint16_t);
                
                if(x < 0){
                    x = 0;
                }
                else if(x > content_size){
                    x = content_size;
                }
                
                if(x != input->cursor.index){
                    input->cursor.update = true;
                    GapBufferMoveGap(input->gap_buffer,uint16_t,x);
                }
                
                input->cursor.index = x;
            }
            else{
                input->on = true;
                input->cursor.update = true;
                input->cursor.index = ContentSize(input->gap_buffer,uint16_t);
                GapBufferMoveGap(input->gap_buffer,uint16_t,input->cursor.index);
            }

            SetInputOffset(input);
        }
        else{
            input->on = false;
            input->offset = 0;
        }
    }
    else if(event.type == WG_TEXTINPUT && input->on){
        
        wchar_t *entry = NULL;

        if(!(input->flags & INPUT_ANY)){

            if(input->flags & INPUT_HEXADECIMAL){
                entry = wcscasechr(L"0123456789ABCDEF",event.text_input);
            }
            else if(input->flags & INPUT_DECIMAL){
                entry = wcscasechr(L"0123456789",event.text_input);
            }
        }
            
        if(ContentSize(input->gap_buffer,uint16_t) < input->max_length && ((input->flags & INPUT_ANY) || entry)){
                
            GapBufferInsertValue(input->gap_buffer,uint16_t,event.text_input);
            input->cursor.index++;
            input->cursor.update = true;

            if(input->flags & INPUT_VALIDATE) InputCallback(input);

            SetInputOffset(input);
        }
    }
    else if(event.type == WG_KEYDOWN && input->on){

        if(event.key.scancode == WG_SCANCODE_RETURN){

            if(input->flags & INPUT_SUBMIT) InputCallback(input);

            input->on = false;
            input->offset = 0;
        }
        else if(event.key.scancode == WG_SCANCODE_BACKSPACE && input->cursor.index > 0){
            GapBufferRemove(input->gap_buffer,uint16_t,1);
            input->cursor.index--;
            input->cursor.update = true;

            if(input->flags & INPUT_VALIDATE) InputCallback(input);

            SetInputOffset(input);
        }
        else if(event.key.scancode == WG_SCANCODE_LEFT && input->cursor.index > 0){
            GapBufferMoveGapLeft(input->gap_buffer,uint16_t,1);
            input->cursor.index--;
            input->cursor.update = true;

            SetInputOffset(input);
        }
        else if(event.key.scancode == WG_SCANCODE_RIGHT && input->cursor.index < ContentSize(input->gap_buffer,uint16_t)){
            GapBufferMoveGapRight(input->gap_buffer,uint16_t,1);
            input->cursor.index++;
            input->cursor.update = true;
            
            SetInputOffset(input);
        }
    }
}

void DestroyInput(Input *input){
    GapBufferFree(input->gap_buffer);
    free(input);
}