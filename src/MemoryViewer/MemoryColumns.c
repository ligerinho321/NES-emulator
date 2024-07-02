#include <MemoryViewer/MemoryColumns.h>
#include <MemoryViewer/MemoryViewer_Utils.h>
#include <ComboBox/ComboBox.h>
#include <Utils.h>

static void Column4_Set(void *data){
    MemoryViewer *memory_viewer = data;

    if(memory_viewer->memory_columns.number == 4) return;

    memory_viewer->memory_columns.number = 4;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}

static void Column8_Set(void *data){
    MemoryViewer *memory_viewer = data;

    if(memory_viewer->memory_columns.number == 8) return;

    memory_viewer->memory_columns.number = 8;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}

static void Column16_Set(void *data){
    MemoryViewer *memory_viewer = data;

    if(memory_viewer->memory_columns.number == 16) return;

    memory_viewer->memory_columns.number = 16;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}

static void Column32_Set(void *data){
    MemoryViewer *memory_viewer = data;

    if(memory_viewer->memory_columns.number == 32) return;

    memory_viewer->memory_columns.number = 32;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}

static void Column48_Set(void *data){
    MemoryViewer *memory_viewer = data;
 
    if(memory_viewer->memory_columns.number == 48) return;

    memory_viewer->memory_columns.number = 48;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}

static void Column64_Set(void *data){
    MemoryViewer *memory_viewer = data;

    if(memory_viewer->memory_columns.number == 64) return;

    memory_viewer->memory_columns.number = 64;

    Memory_ConfigRect(memory_viewer);

    Memory_SetInputAddress(memory_viewer,memory_viewer->memory.input.address);
}


void MemoryViewer_MemoryColumns_Init(MemoryViewer *memory_viewer){
    
    memory_viewer->memory_columns.label = CreateLabel(L"Columns:",memory_viewer->atlas);

    memory_viewer->memory_columns.combo_box = ComboBox_Create(
        memory_viewer->emulator->window_class,
        memory_viewer->window,
        memory_viewer->renderer,
        memory_viewer->arrow,
        memory_viewer->atlas
    );
    
    memory_viewer->memory_columns.column4 = ComboBox_CreateString(L"4",Column4_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column4);

    memory_viewer->memory_columns.column8 = ComboBox_CreateString(L"8",Column8_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column8);

    memory_viewer->memory_columns.column16 = ComboBox_CreateString(L"16",Column16_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column16);

    memory_viewer->memory_columns.column32 = ComboBox_CreateString(L"32",Column32_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column32);

    memory_viewer->memory_columns.column48 = ComboBox_CreateString(L"48",Column48_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column48);

    memory_viewer->memory_columns.column64 = ComboBox_CreateString(L"64",Column64_Set,memory_viewer);
    ComboBox_InsertString(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column64);

    ComboBox_SetCurrentElement(memory_viewer->memory_columns.combo_box,memory_viewer->memory_columns.column16);
}

void MemoryViewer_MemoryColumns_Free(MemoryViewer *memory_viewer){
    DestroyLabel(memory_viewer->memory_columns.label);
    ComboBox_Free(memory_viewer->memory_columns.combo_box);
}