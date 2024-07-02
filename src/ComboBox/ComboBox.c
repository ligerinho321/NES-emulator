#include <ComboBox/ComboBox.h>

ComboBox* ComboBox_Create(WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer,WG_Texture *arrow,WG_FontAtlas *atlas){
    
    ComboBox *combo_box = malloc(sizeof(ComboBox));
    memset(combo_box,0,sizeof(ComboBox));

    combo_box->atlas = atlas;
    WG_SizeTextByFontAtlas(atlas,L"A",&combo_box->glyph_size.x,&combo_box->glyph_size.y);

    combo_box->expand_button.icon.arrow = arrow;

    combo_box->menu = malloc(sizeof(ComboBoxMenu));
    ComboBoxMenu_Init(combo_box->menu,combo_box,class,window_parent,share_renderer);

    return combo_box;
}

void ComboBox_SetCurrentElement(ComboBox *combo_box,ComboBoxElement *cbe){
    if(!cbe->show) return;
    combo_box->cbe = cbe;
    cbe->callback(cbe->data);
}

void ComboBox_SetDefaultElement(ComboBox *combo_box){
    ComboBoxElement *cbe = combo_box->list;
    while(cbe){
        if(cbe->show){
            combo_box->cbe = cbe;
            cbe->callback(cbe->data);
            return;
        }
        cbe = cbe->next;
    }
    combo_box->cbe = NULL;
}

ComboBoxElement* ComboBox_CreateString(const wchar_t *string,ComboBox_Callback callback,void *data){
    ComboBoxElement *cbe = malloc(sizeof(ComboBoxElement));
    memset(cbe,0,sizeof(ComboBoxElement));
    cbe->name.string = wcsdup(string);
    cbe->show = true;
    cbe->callback = callback;
    cbe->data = data;
    return cbe;
}

void ComboBox_InsertString(ComboBox *combo_box,ComboBoxElement *cbe){
    
    WG_SizeTextByFontAtlas(combo_box->atlas,cbe->name.string,&cbe->name.size.x,&cbe->name.size.y);

    if(!combo_box->list){
        combo_box->list = cbe;
    }
    else{
        ComboBoxElement *buffer = combo_box->list;
        while(buffer->next){buffer = buffer->next;}
        buffer->next = cbe;
    }
}

void ComboBox_SetSize(ComboBox *combo_box){
    ComboBoxElement *cbe = combo_box->list;
    combo_box->size = (Vector2){0};
    while(cbe){
        if(cbe->show){
            combo_box->size.x = max(combo_box->size.x,cbe->name.size.x + BORDER_SPACING * 2);
            combo_box->size.y += ELEMENT_HEIGHT;
        }
        cbe = cbe->next;
    }
}

void ComboBox_ConfigRect(ComboBox *combo_box,Vector2 position){
    
    ComboBox_SetSize(combo_box);

    combo_box->menu->window_size = (Vector2){
        .x = combo_box->size.x + ICON_SIZE,
        .y = combo_box->size.y
    };

    combo_box->expand_button.rect = (Rect){
        .x = position.x,
        .y = position.y,
        .w = combo_box->menu->window_size.x,
        .h = ELEMENT_HEIGHT
    };

    combo_box->expand_button.name.rect = (Rect){
        .x = combo_box->expand_button.rect.x,
        .y = combo_box->expand_button.rect.y,
        .w = combo_box->size.x,
        .h = ELEMENT_HEIGHT
    };

    combo_box->expand_button.name.position = (Vector2){
        .x = combo_box->expand_button.name.rect.x + BORDER_SPACING,
        .y = combo_box->expand_button.name.rect.y + combo_box->expand_button.rect.h * 0.5f - combo_box->glyph_size.y * 0.5f
    };

    combo_box->expand_button.icon.rect = (Rect){
        .x = combo_box->expand_button.name.rect.x + combo_box->expand_button.name.rect.w,
        .y = combo_box->expand_button.rect.y + combo_box->expand_button.rect.h * 0.5f - ICON_SIZE * 0.5f,
        .w = ICON_SIZE,
        .h = ICON_SIZE
    };

    ComboBoxElement *cbe = combo_box->list;

    uint32_t y = 0;

    while(cbe){

        if(cbe->show){
            
            cbe->rect = (Rect){
                .x = 0,
                .y = y,
                .w = combo_box->expand_button.rect.w,
                .h = ELEMENT_HEIGHT
            };
            
            cbe->name.position = (Vector2){
                .x = cbe->rect.x + BORDER_SPACING,
                .y = cbe->rect.y + cbe->rect.h * 0.5f - combo_box->glyph_size.y * 0.5f,
            };
            
            y += cbe->rect.h;
        }

        cbe = cbe->next;
    }
}

void ComboBox_Draw(ComboBox *combo_box,WG_Renderer *renderer){

    WG_DrawFilledRectangle(renderer,&combo_box->expand_button.rect,BACKGROUND1_COLOR);

    WG_DrawRectangle(renderer,&combo_box->expand_button.rect,BORDER_COLOR);

    if(combo_box->cbe){

        if(combo_box->expand_button.mouse.over){

            combo_box->expand_button.mouse.over = false;

            WG_DrawText(renderer,combo_box->atlas,combo_box->cbe->name.string,combo_box->expand_button.name.position,ON_COLOR);
        }
        else{
            WG_DrawText(renderer,combo_box->atlas,combo_box->cbe->name.string,combo_box->expand_button.name.position,OFF_COLOR);
        }
    }

    if(combo_box->menu->open){
        WG_BlitEx(renderer,combo_box->expand_button.icon.arrow,NULL,&combo_box->expand_button.icon.rect,0,NULL,FLIP_VERTICAL);
    }
    else{
        WG_BlitEx(renderer,combo_box->expand_button.icon.arrow,NULL,&combo_box->expand_button.icon.rect,0,NULL,FLIP_NONE);
    }
}

void ComboBox_Event(ComboBox *combo_box,WG_Event event,Vector2 mouse){

    if(PointInRect(mouse,combo_box->expand_button.rect) && !combo_box->menu->open && combo_box->list){
        combo_box->expand_button.mouse.over = true;
    }

    if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(combo_box->expand_button.mouse.over){
            combo_box->expand_button.mouse.pressed = true;
        }
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(combo_box->expand_button.mouse.pressed){
            
            if(combo_box->expand_button.mouse.over){

                ComboBoxMenu_OpenWindow(combo_box->menu);
            }

            combo_box->expand_button.mouse.pressed = false;
        }
    }
}

void ComboBox_Free(ComboBox *combo_box){

    ComboBoxMenu_Free(combo_box->menu);

    ComboBoxElement *cbe = combo_box->list;
    ComboBoxElement *buffer = NULL;

    while(cbe){
        buffer = cbe->next;
        free(cbe->name.string);
        free(cbe);
        cbe = buffer;
    }

    free(combo_box);
}