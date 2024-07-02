#include <ComboBox/ComboBoxMenu.h>
#include <ComboBox/ComboBox.h>

void ComboBoxMenu_Init(ComboBoxMenu *combo_box_menu,ComboBox *combo_box,WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer){
    memset(combo_box_menu,0,sizeof(ComboBoxMenu));

    combo_box_menu->combo_box = combo_box;

    combo_box_menu->window_parent = window_parent;

    combo_box_menu->share_renderer = share_renderer;

    combo_box_menu->window = WG_CreateWindow(
        &class,
        window_parent,
        L"ComboBox Menu",
        0,
        0,
        0,
        0,
        WG_WINDOW_HIDDEN | WG_WINDOW_POPUP,
        0
    );

    combo_box_menu->renderer = WG_CreateRenderer(combo_box_menu->window,share_renderer);

    combo_box_menu->atlas = combo_box->atlas;
    combo_box_menu->glyph_size = combo_box->glyph_size;
}

void ComboBoxMenu_OpenWindow(ComboBoxMenu *combo_box_menu){
    if(combo_box_menu->open && WG_WindowIsVisible(combo_box_menu->window)) return;

    combo_box_menu->selected_cbe = NULL;

    Vector2 position = {
        .x = combo_box_menu->combo_box->expand_button.rect.x,
        .y = combo_box_menu->combo_box->expand_button.rect.y + combo_box_menu->combo_box->expand_button.rect.h,
    };

    WG_ClientToScreen(combo_box_menu->window_parent,&position.x,&position.y);
    
    WG_SetWindowPos(
        combo_box_menu->window,
        HWND_TOP,
        position.x,
        position.y,
        combo_box_menu->window_size.x,
        combo_box_menu->window_size.y,
        SWP_SHOWWINDOW
    );

    if(WG_WindowIsVisible(combo_box_menu->window)){
        combo_box_menu->open = true;
    }
    else{
        printf("%s: activation failure in window\n",__func__);
        combo_box_menu->open = false;
    }
}

void ComboBoxMenu_CloseWindow(ComboBoxMenu *combo_box_menu){
    if(!combo_box_menu->open && !WG_WindowIsVisible(combo_box_menu->window)) return;
    
    WG_ShowWindow(combo_box_menu->window,SW_HIDE);

    if(!WG_WindowIsVisible(combo_box_menu->window)){
        combo_box_menu->open = false;
        WG_BringWindowToTop(combo_box_menu->window_parent);
    }
    else{
        printf("%s: window deactivation failed\n",__func__);
        combo_box_menu->open = true;
    }
}

static void ComboBoxMenu_Event(ComboBoxMenu *combo_box_menu){

    WG_PollEvent(combo_box_menu->window,&combo_box_menu->event);

    WG_Event event = combo_box_menu->event;

    WG_GetMousePosInWindow(combo_box_menu->window,&combo_box_menu->mouse.window.x,&combo_box_menu->mouse.window.y);

    WG_GetMousePos(&combo_box_menu->mouse.desktop.x,&combo_box_menu->mouse.desktop.y);

    if(WG_PointOverWindow(combo_box_menu->window,combo_box_menu->mouse.desktop.x,combo_box_menu->mouse.desktop.y)){
        
        ComboBoxElement *cbe = combo_box_menu->combo_box->list;
        
        while(cbe){
            if(cbe->show && PointInRect(combo_box_menu->mouse.window,cbe->rect)){
                cbe->mouse.over = true;
                break;
            }
            cbe = cbe->next;
        }
    }

    if(event.type == WG_WINDOW_FOCUS_LOST){
        combo_box_menu->selected_cbe = NULL;
        combo_box_menu->open = false;
    }
    else if(event.type == WG_MOUSEBUTTONDOWN && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(WG_PointOverWindow(combo_box_menu->window,combo_box_menu->mouse.desktop.x,combo_box_menu->mouse.desktop.y)){
            
            ComboBoxElement *cbe = combo_box_menu->combo_box->list;
            
            while(cbe){
                if(cbe->show && cbe->mouse.over){
                    cbe->mouse.pressed = true;
                    break;
                }
                cbe = cbe->next;
            }
        }
    }
    else if(event.type == WG_MOUSEBUTTONUP && event.mouse.button == WG_MOUSEBUTTON_LEFT){
        
        if(WG_PointOverWindow(combo_box_menu->window,combo_box_menu->mouse.desktop.x,combo_box_menu->mouse.desktop.y)){
            
            ComboBoxElement *cbe = combo_box_menu->combo_box->list;
            
            while(cbe){
                if(cbe->show && cbe->mouse.pressed){
                    if(cbe->mouse.over){
                        combo_box_menu->selected_cbe = cbe;
                        combo_box_menu->open = false;
                    }
                    cbe->mouse.pressed = false;
                    break;
                }
                cbe = cbe->next;
            }
        }
    }
}

static void ComboBoxMenu_Draw(ComboBoxMenu *combo_box_menu){
    
    WG_Renderer *renderer = combo_box_menu->renderer;

    WG_SetCurrentRenderer(combo_box_menu->renderer);
    
    WG_ClearColor(renderer,BACKGROUND1_COLOR);
        
    ComboBoxElement *cbe = combo_box_menu->combo_box->list;
        
    while(cbe){
        
        if(cbe->show){

            if(cbe->mouse.over){

                cbe->mouse.over = false;

                WG_DrawText(renderer,combo_box_menu->atlas,cbe->name.string,cbe->name.position,ON_COLOR);
            }
            else{
                WG_DrawText(renderer,combo_box_menu->atlas,cbe->name.string,cbe->name.position,OFF_COLOR);
            }
        }
        cbe = cbe->next;
    }
        
    WG_DrawRectangle(renderer,NULL,BORDER_COLOR);

    WG_Flip(renderer);
}

void ComboBoxMenu_Run(ComboBoxMenu *combo_box_menu){
    if(!combo_box_menu->open) return;

    ComboBoxMenu_Event(combo_box_menu);
    
    ComboBoxMenu_Draw(combo_box_menu);

    if(!combo_box_menu->open){
        
        ComboBoxMenu_CloseWindow(combo_box_menu);

        if(combo_box_menu->selected_cbe){
            combo_box_menu->combo_box->cbe = combo_box_menu->selected_cbe;    
            combo_box_menu->selected_cbe->callback(combo_box_menu->selected_cbe->data);
            combo_box_menu->selected_cbe = NULL;
        }
    }
}

void ComboBoxMenu_Free(ComboBoxMenu *combo_box_menu){
    WG_DestroyRenderer(combo_box_menu->renderer);
    WG_DestroyWindow(combo_box_menu->window);
    free(combo_box_menu);
}