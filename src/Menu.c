#include <Menu.h>

#define SPACING 20

Shortcut* Menu_CreateShortcut(const uint16_t *text,WG_KeyMod keymod,WG_ScanCode key){
    Shortcut *shortcut = calloc(1,sizeof(Shortcut));
    shortcut->s.string = wcsdup(text);
    shortcut->keymod = keymod;
    shortcut->key = key;
    return shortcut;
}

MenuOption* Menu_CreateOption(const uint16_t *name,Shortcut *shortcut,Menu_Callback callback,void *userdata){
    MenuOption *option = calloc(1,sizeof(MenuOption));
    option->s.string = wcsdup(name);
    option->shortcut = shortcut;
    option->enabled = true;
    option->callback = callback;
    option->userdata = userdata;
    return option;
}

void Menu_InsertOption(Menu *menu,MenuOption *option){

    int width = 0;

    WG_SizeTextByFontAtlas(menu->atlas,option->s.string,&option->s.size.x,&option->s.size.y);
    width += option->s.size.x;

    if(option->shortcut){
        WG_SizeTextByFontAtlas(menu->atlas,option->shortcut->s.string,&option->shortcut->s.size.x,&option->shortcut->s.size.y);
        width += option->shortcut->s.size.x;
    }

    width += BORDER_SPACING * 2 + SPACING;

    menu->window_size.x = max(menu->window_size.x,width);
    menu->window_size.y += ELEMENT_HEIGHT;

    option->next = NULL;

    if(menu->options){
        MenuOption *buffer = menu->options;
        while(buffer->next){buffer = buffer->next;}
        buffer->next = option;
    }
    else{
        menu->options = option;
    }
}


void Menu_Init(Menu *menu,WNDCLASS class,WG_Window *window_parent,WG_Renderer *share_renderer,WG_FontAtlas *atlas){
    
    memset(menu,0,sizeof(Menu));
    
    menu->window_parent = window_parent;

    menu->share_renderer = share_renderer;
    
    menu->window = WG_CreateWindow(
        &class,
        window_parent,
        L"Menu",
        0,
        0,
        0,
        0,
        WG_WINDOW_HIDDEN | WG_WINDOW_POPUP,
        0
    );

    menu->renderer = WG_CreateRenderer(menu->window,share_renderer);

    menu->atlas = atlas;

    WG_SizeTextByFontAtlas(atlas,L"A",&menu->glyph_size.x,&menu->glyph_size.y);

    menu->open = false;
}

void Menu_OpenWindow(Menu *menu,Vector2 position){
    if(menu->open && WG_WindowIsVisible(menu->window)) return;

    menu->option_selected = NULL;

    MenuOption *option = menu->options;

    int y = 0;

    while(option){

        option->rect = (Rect){
            .x = 0,
            .y = y,
            .w = menu->window_size.x,
            .h = ELEMENT_HEIGHT
        };

        option->s.position = (Vector2){
            .x = option->rect.x + BORDER_SPACING,
            .y = option->rect.y + BORDER_SPACING
        };

        if(option->shortcut){
            option->shortcut->s.position = (Vector2){
                .x = option->rect.x + option->rect.w - option->shortcut->s.size.x - BORDER_SPACING,
                .y = option->rect.y + BORDER_SPACING
            };
        }

        y += ELEMENT_HEIGHT;

        option = option->next;
    }

    WG_SetWindowPos(
        menu->window,
        HWND_TOP,
        position.x,
        position.y,
        menu->window_size.x,
        menu->window_size.y,
        SWP_SHOWWINDOW
    );

    if(WG_WindowIsVisible(menu->window)){
        menu->open = true;
    }
    else{
        printf("%s: activation failure in window\n",__func__);
        menu->open = false;
    }
    
}

void Menu_CloseWindow(Menu *menu){
    if(!menu->open && !WG_WindowIsVisible(menu->window)) return;
    
    WG_ShowWindow(menu->window,SW_HIDE);

    if(!WG_WindowIsVisible(menu->window)){
        menu->open = false;
        WG_BringWindowToTop(menu->window_parent);
    }
    else{
        printf("%s: window deactivation failed\n",__func__);
        menu->open = true;
    }
}

void Menu_ShortcutEvent(Menu *menu,WG_Event event){

    if(event.type != WG_KEYDOWN) return;

    MenuOption *option = menu->options;
        
    while(option){
            
        if(option->enabled && option->shortcut && (option->shortcut->keymod || option->shortcut->key)){
                
            if(!option->shortcut->keymod || (WG_GetKeyModState() & option->shortcut->keymod)){

                if(!option->shortcut->key || event.key.scancode == option->shortcut->key){

                    option->callback(option->userdata);

                    Menu_CloseWindow(menu);
                }
            }
        }

        option = option->next;
    }
}

static void Menu_Event(Menu *menu){

    WG_PollEvent(menu->window,&menu->event);

    WG_GetMousePosInWindow(menu->window,&menu->mouse.window.x,&menu->mouse.window.y);

    WG_GetMousePos(&menu->mouse.desktop.x,&menu->mouse.desktop.y);

    if(WG_PointOverWindow(menu->window,menu->mouse.desktop.x,menu->mouse.desktop.y)){
        MenuOption *option = menu->options;
        while(option){
            if(option->enabled && PointInRect(menu->mouse.window,option->rect)){
                option->m.over = true;
                break;
            }
            option = option->next;
        }
    }
    
    if(menu->event.type == WG_WINDOW_FOCUS_LOST){
        menu->option_selected = NULL;
        menu->open = false;
    }
    else if(menu->event.type == WG_MOUSEBUTTONDOWN && menu->event.mouse.button == WG_MOUSEBUTTON_LEFT){
        MenuOption *option = menu->options;
        while(option){
            if(option->enabled && option->m.over){
                option->m.pressed = true;
                break;
            }
            option = option->next;
        }
    }
    else if(menu->event.type == WG_MOUSEBUTTONUP && menu->event.mouse.button == WG_MOUSEBUTTON_LEFT){
        MenuOption *option = menu->options;
        while(option){
            if(option->enabled && option->m.pressed){
                if(option->m.over){
                    menu->option_selected = option;
                    menu->open = false;
                }
                option->m.pressed = false;
                break;
            }
            option = option->next;
        }
    }
}

static void Menu_Draw(Menu *menu){

    WG_SetCurrentRenderer(menu->renderer);

    WG_ClearColor(menu->renderer,BACKGROUND1_COLOR);

    MenuOption *option = menu->options;
    while(option){

        if(option->enabled){

            if(option->m.over){
                option->m.over = false;

                WG_DrawText(menu->renderer,menu->atlas,option->s.string,option->s.position,SELECTED_COLOR);
                
                if(option->shortcut){
                    WG_DrawText(menu->renderer,menu->atlas,option->shortcut->s.string,option->shortcut->s.position,SELECTED_COLOR);
                }
            }
            else{

                WG_DrawText(menu->renderer,menu->atlas,option->s.string,option->s.position,ON_COLOR);
                
                if(option->shortcut){
                    WG_DrawText(menu->renderer,menu->atlas,option->shortcut->s.string,option->shortcut->s.position,ON_COLOR);
                }
            }
        }
        else{
            WG_DrawText(menu->renderer,menu->atlas,option->s.string,option->s.position,OFF_COLOR);

            if(option->shortcut){
                WG_DrawText(menu->renderer,menu->atlas,option->shortcut->s.string,option->shortcut->s.position,OFF_COLOR);
            }
        }

        option = option->next;
    }

    WG_DrawRectangle(menu->renderer,NULL,BORDER_COLOR);

    WG_Flip(menu->renderer);
}

void Menu_Run(Menu *menu){
    
    if(!menu->open) return;

    Menu_Event(menu);
    
    Menu_Draw(menu);

    if(!menu->open){
        
        Menu_CloseWindow(menu);
        
        if(menu->option_selected){

            menu->option_selected->callback(menu->option_selected->userdata);

            menu->option_selected = NULL;
        }
    }
}

void Menu_Free(Menu *menu){

    WG_DestroyRenderer(menu->renderer);

    WG_DestroyWindow(menu->window);

    MenuOption *option = menu->options;
    MenuOption *buffer = NULL;
    while(option){
        buffer = option->next;
        if(option->shortcut){
            free(option->shortcut->s.string);
            free(option->shortcut);
        }
        free(option->s.string);
        free(option);
        option = buffer;
    }

    free(menu);
}