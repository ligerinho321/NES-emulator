#include <Emulator.h>

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR pCmdLine,int nCmdShow){
    
    Emulator emulator;

    Emulator_Init(&emulator,hInstance);

    Emulator_Run(&emulator);

    Emulator_Free(&emulator);

    return 0;
}