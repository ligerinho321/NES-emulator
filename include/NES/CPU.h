#pragma once

#include <Emulator.h>

//Sytem Vectors
//$FFFA, $FFFB ... NMI (Non-Maskable Interrupt) vector, 16-bit (LB, HB)
#define NMI_ADDRESS 0xFFFA
//$FFFC, $FFFD ... RES (Reset) vector, 16-bit (LB, HB)
#define RES_ADDRESS 0xFFFC
//$FFFE, $FFFF ... IRQ (Interrupt Request) vector, 16-bit (LB, HB)
#define IRQ_ADDRESS 0xFFFE

//Stack Register Flags (bit 7 to bit 0)
#define CPU_FLAG_N BIT_7 // bit7 ... Negative (N)
#define CPU_FLAG_V BIT_6 // bit6 ... Overflow (V)
#define CPU_FLAG_U BIT_5 // bit5 ... ignored
#define CPU_FLAG_B BIT_4 // bit4 ... Break (B)
#define CPU_FLAG_D BIT_3 // bit3 ... Decimal (use BCD for arithmetics) (D)
#define CPU_FLAG_I BIT_2 // bit2 ... Interrupt (IRQ disable) (I)
#define CPU_FLAG_Z BIT_1 // bit1 ... Zero (Z)
#define CPU_FLAG_C BIT_0 // bit0 ... Carry (C)

#define	NN BREAK_NONE // None
#define	RD BREAK_READ // Read
#define	WR BREAK_WRITE // Write
#define	RW (BREAK_READ | BREAK_WRITE) // Read/Write

//IRQ source
#define IRQ_FRAME  0x01
#define IRQ_DMC    0x02
#define IRQ_MAPPER 0x04
#define IRQ_MASK   (IRQ_FRAME | IRQ_DMC | IRQ_MAPPER)

//Executing
#define EXECUTING_INSTRUCTION 0x01
#define EXECUTING_RESET       0x02
#define EXECUTING_NMI         0x04
#define EXECUTING_IRQ         0x08

#define READ_8_BITS(cpu,address)  ((uint8_t)cpu->nes->read((cpu)->nes,address))

#define READ_16_BITS(cpu,address) ((uint16_t)(cpu->nes->read((cpu)->nes,(address) + 1) << 8) | (uint16_t)(cpu->nes->read((cpu)->nes,address)))

#define WRITE_8_BITS(cpu,address,value) (cpu->nes->write((cpu)->nes,value,address))

#define WRITE_16_BITS(cpu,address,value)                       \
do{                                                            \
    cpu->nes->write((cpu)->nes,(value) & 0xFF,address);        \
    cpu->nes->write((cpu)->nes,((value) >> 8) & 0xFF,address); \
}while(0)

uint16_t REL(CPU *cpu);
uint16_t IMP(CPU *cpu);
uint16_t ACU(CPU *cpu);
uint16_t IME(CPU *cpu);
uint16_t ABS(CPU *cpu);
uint16_t ZP0(CPU *cpu);
uint16_t ABX(CPU *cpu);
uint16_t ABY(CPU *cpu);
uint16_t ZPX(CPU *cpu);
uint16_t ZPY(CPU *cpu);
uint16_t IND(CPU *cpu);
uint16_t IDX(CPU *cpu);
uint16_t IDY(CPU *cpu);

void CPU_IRQ(CPU *cpu,uint8_t source);
void CPU_ClearIRQ(CPU *cpu,uint8_t source);

void CPU_NMI(CPU *cpu,uint32_t delay);
void CPU_ClearNMI(CPU *cpu);

void CPU_Reset(CPU *cpu,uint8_t type);

void CPU_OAMDMA(CPU *cpu,uint16_t start_address);

void CPU_Init(CPU *cpu,NES *nes);
void CPU_Execute(CPU *cpu,uint32_t cicles);
