#include <Debugger/Breakpoints/Menu.h>
#include <Debugger/Breakpoints/BreakpointView.h>
#include <Debugger/Breakpoints/BreakpointEdit/BreakpointEdit.h>
#include <Menu.h>

static void AddBreakpoint(void *data){
    DebuggerBreakpoints *debugger_breakpoints = data;
    
    if(debugger_breakpoints->breakpoint_edit->open) return;

    BreakpointEdit_OpenWindow(debugger_breakpoints->breakpoint_edit,OPERATION_ADD,NULL);
}

static void EditBreakpoint(void *data){
    DebuggerBreakpoints *debugger_breakpoints = data;

    if(debugger_breakpoints->breakpoint_edit->open) return;

    BreakpointEdit_OpenWindow(debugger_breakpoints->breakpoint_edit,OPERATION_EDIT,debugger_breakpoints->list.breakpoints.current);
}

static void DeleteBreakpoint(void *data){
    DebuggerBreakpoints *debugger_breakpoints = data;
    
    BreakpointsView_Remove(debugger_breakpoints);
}

void Breakpoints_Menu_Init(DebuggerBreakpoints *debugger_breakpoints){
    
    debugger_breakpoints->menu.menu = malloc(sizeof(Menu));

    Menu_Init(
        debugger_breakpoints->menu.menu,
        debugger_breakpoints->debugger->window_class,
        debugger_breakpoints->debugger->window,
        debugger_breakpoints->debugger->renderer,
        debugger_breakpoints->debugger->atlas
    );

    Shortcut *add_shortcut = Menu_CreateShortcut(L"Insert",WG_KEYMOD_NONE,WG_SCANCODE_INSERT);
    debugger_breakpoints->menu.add = Menu_CreateOption(L"Add",add_shortcut,AddBreakpoint,debugger_breakpoints);
    Menu_InsertOption(debugger_breakpoints->menu.menu,debugger_breakpoints->menu.add);

    Shortcut *edit_shortcut = Menu_CreateShortcut(L"F2",WG_KEYMOD_NONE,WG_SCANCODE_F2);
    debugger_breakpoints->menu.edit = Menu_CreateOption(L"Edit",edit_shortcut,EditBreakpoint,debugger_breakpoints);
    Menu_InsertOption(debugger_breakpoints->menu.menu,debugger_breakpoints->menu.edit);

    Shortcut *delete_shortcut = Menu_CreateShortcut(L"Delete",WG_KEYMOD_NONE,WG_SCANCODE_DELETE);
    debugger_breakpoints->menu.delete = Menu_CreateOption(L"Delete",delete_shortcut,DeleteBreakpoint,debugger_breakpoints);
    Menu_InsertOption(debugger_breakpoints->menu.menu,debugger_breakpoints->menu.delete);
}