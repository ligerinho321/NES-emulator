#pragma once

#include <Emulator.h>

#define A12WATCHER_MIN_DELAY 10
#define A12WATCHER_FLAG 0x1000

typedef enum A12Watcher_Result{
    A12Watcher_None,
    A12Watcher_Rise,
    A12Watcher_Fall
}A12Watcher_Result;

typedef struct _A12Watcher{
    uint32_t last_cycle;
    uint32_t cycles_down;
}A12Watcher;


A12Watcher_Result A12Watcher_UpdateVramAddress(A12Watcher *a12watcher,uint16_t address,uint32_t cycles){
    A12Watcher_Result result = A12Watcher_None;

    if(a12watcher->cycles_down > 0){
        if(a12watcher->last_cycle > cycles){
            a12watcher->cycles_down += 89342 - a12watcher->last_cycle + cycles;
        }
        else{
            a12watcher->cycles_down += cycles - a12watcher->last_cycle;
        }
    }

    if(address & A12WATCHER_FLAG){
        if(a12watcher->cycles_down > A12WATCHER_MIN_DELAY){
            result = A12Watcher_Rise;
        }
        a12watcher->cycles_down = 0;
    }
    else{
        if(a12watcher->cycles_down == 0){
            a12watcher->cycles_down = 1;
            result = A12Watcher_Fall;
        }
    }

    a12watcher->last_cycle = cycles;

    return result;
}