#pragma once

#include <Emulator.h>

//Status Flags
#define STATUS_PULSE1    0x01
#define STATUS_PULSE2    0x02
#define STATUS_TRIANGLE  0x04
#define STATUS_NOISE     0x08
#define STATUS_DMC       0x10
#define STATUS_OPEN_BUS  0x20
#define STATUS_FRAME_IRQ 0x40
#define STATUS_DMC_IRQ   0x80

//Sequence Mode
#define SEQUENCE_FOUR_STEP 0x00
#define SEQUENCE_FIVE_STEP 0x01

#define SWEEP_SILENCED(sweep) ((sweep)->pulse->period < 8 || (sweep)->target_period > 0x7FF)

void APU_Init(APU *apu,NES *nes);
void APU_Mixer(APU *apu);
void APU_Execute(APU *apu,uint32_t cicles);
void APU_Reset(APU *apu);

void APU_WritePulse1(APU *apu,uint16_t address,uint8_t value);
void APU_WritePulse2(APU *apu,uint16_t address,uint8_t value);
void APU_WriteTriangle(APU *apu,uint16_t address,uint8_t value);
void APU_WriteNoise(APU *apu,uint16_t address, uint8_t value);
void APU_WriteDMC(APU *apu,uint16_t address,uint8_t value);
void APU_WriteStatus(APU *apu,uint8_t value);
void APU_WriteFrameCounter(APU *apu,uint8_t value);

uint8_t APU_ReadStatus(APU *apu);

void FrameCounter_Clock(FrameCounter *frame_counter,APU *apu);

void LengthCounter_Load(LengthCounter *length_counter,uint8_t index,uint32_t current_cicle);
uint8_t LengthCounter_Value(LengthCounter *length_counter);
void LengthCounter_Clear(LengthCounter *length_counter);
void LengthCounter_Halt(LengthCounter *length_counter,bool value,uint32_t current_cicle);
void LengthCounter_Clock(LengthCounter *length_counter,uint32_t current_cicle);

void LinearCounter_Clock(LinearCounter *linear_counter);

uint16_t Sweep_UpdateTargetPeriod(Sweep *sweep);
void Sweep_SetControl(Sweep *sweep,uint8_t value);
void Sweep_Clock(Sweep *sweep);

void Envelope_SetControl(Envelope *envelope,uint8_t value);
void Envelope_Clock(Envelope *envelope);

void Pulse_Clock(Pulse *pulse);
void Triangle_Clock(Triangle *triangle);
void Noise_Clock(Noise *noise);
void DMC_StartClock(DMC *dmc,APU *apu);
bool DMC_OutputClock(DMC *dmc);
void DMC_Clock(DMC *dmc,APU *apu);

uint8_t Pulse_Output(Pulse *pulse);
uint8_t Triangle_Output(Triangle *triangle);
uint8_t Noise_Output(Noise *noise);
uint8_t DMC_Output(DMC *dmc);