#pragma once

#include <Emulator.h>

void MenuBar_Init(MenuBar *menubar,Emulator *emulator);

void MenuBar_ConfigRect(MenuBar *menubar);

void MenuBar_Run(MenuBar *menubar);

void MenuBar_RunChildrenWindow(MenuBar *menubar);

void MenuBar_Free(MenuBar *menubar);