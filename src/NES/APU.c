#include <NES/APU.h>
#include <NES/CPU.h>

static const uint32_t average_queue_size = AUDIO_RATE / 60;
static const uint32_t max_queue_size = (AUDIO_RATE / 60) * 10;

static const uint8_t length_table[32] = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

static const bool pulse_sequence[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1}
};

static const uint8_t triangle_sequence[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const uint16_t noise_period[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

static const uint16_t dmc_period[16] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54
};


static void QuarterFrame_Clock(APU *apu){

    Envelope_Clock(&apu->pulse1.envelope);
    Envelope_Clock(&apu->pulse2.envelope);
    Envelope_Clock(&apu->noise.envelope);

    LinearCounter_Clock(&apu->triangle.linear_counter);
}

static void HalfFrame_Clock(APU *apu){

    LengthCounter_Clock(&apu->pulse1.length_counter,apu->cycles);
    LengthCounter_Clock(&apu->pulse2.length_counter,apu->cycles);
    LengthCounter_Clock(&apu->noise.length_counter,apu->cycles);
    LengthCounter_Clock(&apu->triangle.length_counter,apu->cycles);

    Sweep_Clock(&apu->pulse1.sweep);
    Sweep_Clock(&apu->pulse2.sweep);
}


void APU_Init(APU *apu,NES *nes){
    apu->nes = nes;

    apu->pulse_table[0] = 0.0f;
    for(uint16_t i=1; i<PULSE_TABLE_SIZE; ++i){
        apu->pulse_table[i] = 95.52f / (8128.0f / i + 100);
    }

    apu->tnd_table[0] = 0.0f;
    for(uint16_t i=1; i<TND_TABLE_SIZE; ++i){
        apu->tnd_table[i] = 163.67f / (24329.0f / i + 100);
    }

    apu->output_count = 0;

    apu->cycles_per_samples = CPU_6502_RATE / AUDIO_RATE;
    apu->cycles_per_samples_count = 0;
}

void APU_Mixer(APU *apu){

    if(apu->output_count >= OUTPUT_SIZE) return;

    uint8_t pulse1_output = Pulse_Output(&apu->pulse1);
    uint8_t pulse2_output = Pulse_Output(&apu->pulse2);
    uint8_t triangle_output = Triangle_Output(&apu->triangle);
    uint8_t noise_output = Noise_Output(&apu->noise);
    uint8_t dmc_output = DMC_Output(&apu->dmc);

    float pulse_out = apu->pulse_table[pulse1_output + pulse2_output];
    float tnd_out = apu->tnd_table[3 * triangle_output + 2 * noise_output + dmc_output];

    apu->output_samples[apu->output_count++] = (pulse_out + tnd_out) * AUDIO_MAX;

    if(apu->output_count >= average_queue_size){
        
        uint32_t queue_size = WG_GetQueuedAudioSize(apu->nes->emulator->audio_device) / sizeof(AUDIO_TYPE);
        
        if(queue_size < max_queue_size){
            WG_QueueAudio(apu->nes->emulator->audio_device,apu->output_samples,sizeof(AUDIO_TYPE) * apu->output_count);
        }
        
        apu->output_count = 0;
    }
}

void APU_Execute(APU *apu,uint32_t cycles){

    for(uint32_t i=0; i<cycles; ++i){
        
        if(apu->frame_interrupt == true && apu->frame_counter.interrupt_inhibit == false){
            CPU_IRQ(&apu->nes->cpu,IRQ_FRAME);
        }
        
        FrameCounter_Clock(&apu->frame_counter,apu);
        
        Pulse_Clock(&apu->pulse1);
        
        Pulse_Clock(&apu->pulse2);
        
        Triangle_Clock(&apu->triangle);
        
        Noise_Clock(&apu->noise);
        
        DMC_Clock(&apu->dmc,apu);
        
        apu->cycles_per_samples_count++;
        if(apu->cycles_per_samples_count >= apu->cycles_per_samples){
            apu->cycles_per_samples_count -= apu->cycles_per_samples;
            APU_Mixer(apu);
        }
        
        apu->cycles++;
    }
}

void APU_Reset(APU *apu){
    //Pulse1
    memset(&apu->pulse1,0,sizeof(Pulse));
    apu->pulse1.sweep.pulse = &apu->pulse1;
    apu->pulse1.carry = true;

    //Pulse2
    memset(&apu->pulse2,0,sizeof(Pulse));
    apu->pulse2.sweep.pulse = &apu->pulse2;
    apu->pulse2.carry = false;

    //Triangle
    memset(&apu->triangle,0,sizeof(Triangle));

    //Noise
    memset(&apu->noise,0,sizeof(Noise));
    apu->noise.lfsr = 1;

    //DMC
    memset(&apu->dmc,0,sizeof(DMC));

    //Frame Counter
    memset(&apu->frame_counter,0,sizeof(FrameCounter));

    apu->dmc_interrupt = false;
    apu->frame_interrupt = false;

    apu->cycles = 0;

    APU_Execute(apu,2);
}


//Write Registers
void APU_WritePulse1(APU *apu,uint16_t address,uint8_t value){
    switch(address){
        case 0x4000:
            //DDLC.VVVV
            apu->pulse1.duty = (value >> 0x06) & 0x03;

            LengthCounter_Halt(&apu->pulse1.length_counter,value & 0x20,apu->cycles);

            Envelope_SetControl(&apu->pulse1.envelope,value);
            break;
        case 0x4001:
            //EPPP.NSSS
            Sweep_SetControl(&apu->pulse1.sweep,value);
            break;
        case 0x4002:
            //LLLL.LLLL
            apu->pulse1.period = (apu->pulse1.period & 0xFF00) | (value & 0x00FF);
            
            Sweep_UpdateTargetPeriod(&apu->pulse1.sweep);
            break;
        case 0x4003:
            //LLLL.LHHH
             if(apu->pulse1.channel_enabled){
                LengthCounter_Load(&apu->pulse1.length_counter,value >> 0x03,apu->cycles);
            }

            apu->pulse1.period = ((value & 0x07) << 0x08) | (apu->pulse1.period & 0x00FF);

            Sweep_UpdateTargetPeriod(&apu->pulse1.sweep);

            apu->pulse1.duty_position = 0;

            apu->pulse1.envelope.start_flag = true;
            break;
    }
}

void APU_WritePulse2(APU *apu,uint16_t address,uint8_t value){
    switch(address){
        case 0x4004:
            //DDLC.VVVV
            apu->pulse2.duty = (value >> 0x06) & 0x03;

            LengthCounter_Halt(&apu->pulse2.length_counter,value & 0x20,apu->cycles);

            Envelope_SetControl(&apu->pulse2.envelope,value);
            break;
        case 0x4005:
            //EPPP.NSSS
            Sweep_SetControl(&apu->pulse2.sweep,value);
            break;
        case 0x4006:
            //LLLL.LLLL
            apu->pulse2.period = (apu->pulse2.period & 0xFF00) | (value & 0x00FF);

            Sweep_UpdateTargetPeriod(&apu->pulse2.sweep);
            break;
        case 0x4007:
            //LLLL.LHHH
            if(apu->pulse2.channel_enabled){
                LengthCounter_Load(&apu->pulse2.length_counter,value >> 0x03,apu->cycles);
            }

            apu->pulse2.period = ((value & 0x07) << 0x08) | (apu->pulse2.period & 0x00FF);

            Sweep_UpdateTargetPeriod(&apu->pulse2.sweep);

            apu->pulse2.duty_position = 0;

            apu->pulse2.envelope.start_flag = true;
            break;
    }
}

void APU_WriteTriangle(APU *apu,uint16_t address,uint8_t value){
    switch(address){
        case 0x4008:
            //CRRR.RRRR
            LengthCounter_Halt(&apu->triangle.length_counter,value & 0x80,apu->cycles);

            apu->triangle.linear_counter.halt_flag = (value & 0x80) ? true : false;
            apu->triangle.linear_counter.counter.reload_value = value & 0x7F;
            break;
        case 0x4009:
            //Unused
            break;
        case 0x400A:
            //LLLL.LLLL
            apu->triangle.period = (apu->triangle.period & 0x700) | (value & 0xFF);
            break;
        case 0x400B:
            //LLLL.LHHH
            if(apu->triangle.channel_enabled){
               LengthCounter_Load(&apu->triangle.length_counter,value >> 0x03,apu->cycles);
            }

            apu->triangle.period = ((value & 0x07) << 0x08) | (apu->triangle.period & 0xFF);

            apu->triangle.linear_counter.counter.reload_flag = true;
            break;
    }
}

void APU_WriteNoise(APU *apu,uint16_t address, uint8_t value){
    switch(address){
        case 0x400C:
            //--LC VVVV
            LengthCounter_Halt(&apu->noise.length_counter,value & 0x20,apu->cycles);

            Envelope_SetControl(&apu->noise.envelope,value);
            break;
        case 0x400D:
            //Unused
            break;
        case 0x400E:
            //L--- PPPP
            apu->noise.mode_flag = (value & 0x80) ? true : false;

            apu->noise.period = noise_period[value & 0x0F];
            break;
        case 0x400F:
            //LLLL L---
            if(apu->noise.channel_enabled){
                LengthCounter_Load(&apu->noise.length_counter,value >> 0x03,apu->cycles);
            }

            apu->noise.envelope.start_flag = true;
            break;
    }
}

void APU_WriteDMC(APU *apu,uint16_t address,uint8_t value){
    switch(address){
        case 0x4010:
            //IL-- RRRR
            apu->dmc.irq_enabled = (value & 0x80) ? true : false;

            if(!apu->dmc.irq_enabled){
                apu->dmc_interrupt = false;
                CPU_ClearIRQ(&apu->nes->cpu,IRQ_DMC);
            }

            apu->dmc.loop_flag = (value & 0x40) ? true : false;

            apu->dmc.period = dmc_period[value & 0x0F];
            break;
        case 0x4011:
            //-DDD DDDD
            apu->dmc.output_level = value & 0x7F;
            break;
        case 0x4012:
            //AAAA AAAA
            apu->dmc.sample_address = 0xC000 + (value * 64);

            apu->dmc.current_address = apu->dmc.sample_address;
            break;
        case 0x4013:
            //LLLL LLLL
            apu->dmc.sample_length = (value * 16) + 1;

            if(apu->dmc.bytes_remaining == 0){
                apu->dmc.bytes_remaining = apu->dmc.sample_length;
            }
            break;
    }
}

void APU_WriteStatus(APU *apu,uint8_t value){
    //---D NT21

    apu->dmc_interrupt = false;
    CPU_ClearIRQ(&apu->nes->cpu,IRQ_DMC);

    //Pulse1
    apu->pulse1.channel_enabled = (value & STATUS_PULSE1) ? true : false;
    
    if(!apu->pulse1.channel_enabled){
        LengthCounter_Clear(&apu->pulse1.length_counter);
    }

    //Pulse2
    apu->pulse2.channel_enabled = (value & STATUS_PULSE2) ? true : false;

    if(!apu->pulse2.channel_enabled){
        LengthCounter_Clear(&apu->pulse2.length_counter);
    }

    //Triangle
    apu->triangle.channel_enabled = (value & STATUS_TRIANGLE) ? true : false;

    if(!apu->triangle.channel_enabled){
        LengthCounter_Clear(&apu->triangle.length_counter);
    }

    //Noise
    apu->noise.channel_enabled = (value & STATUS_NOISE) ? true : false;

    if(!apu->noise.channel_enabled){
        LengthCounter_Clear(&apu->noise.length_counter);
    }

    //DMC
    apu->dmc.channel_enabled = (value & STATUS_DMC) ? true : false;

    if(apu->dmc.channel_enabled){
        
        if(apu->dmc.bytes_remaining == 0){
            apu->dmc.bytes_remaining = apu->dmc.sample_length;
        }

        if(DMC_OutputClock(&apu->dmc)){
            DMC_StartClock(&apu->dmc,apu);
        }
    }
    else{
        apu->dmc.bytes_remaining = 0;
    }
}

void APU_WriteFrameCounter(APU *apu,uint8_t value){
    
    apu->frame_counter.last_value = value;
    apu->frame_counter.sequence_mode = (value & 0x80) ? SEQUENCE_FIVE_STEP : SEQUENCE_FOUR_STEP;

    apu->frame_counter.sequence_position = 0;

    apu->frame_counter.next_clock = apu->cycles + (apu->cycles & 1) + 1;

    if(apu->frame_counter.sequence_mode == SEQUENCE_FOUR_STEP){
        apu->frame_counter.next_clock += 7458;
    }

    apu->frame_counter.interrupt_inhibit = (value & 0x40) ? true : false;

    if(apu->frame_counter.interrupt_inhibit){
        apu->frame_interrupt = false;
        CPU_ClearIRQ(&apu->nes->cpu,IRQ_FRAME);
    }
}


uint8_t APU_ReadStatus(APU *apu){
    //IF-D.NT21
    
    uint8_t value = 0;

    if(LengthCounter_Value(&apu->pulse1.length_counter) > 0) value |= STATUS_PULSE1;

    if(LengthCounter_Value(&apu->pulse2.length_counter) > 0) value |= STATUS_PULSE2;

    if(LengthCounter_Value(&apu->triangle.length_counter) > 0) value |= STATUS_TRIANGLE;

    if(LengthCounter_Value(&apu->noise.length_counter) > 0) value |= STATUS_NOISE;

    if(apu->dmc.bytes_remaining > 0) value |= STATUS_DMC;

    value |= apu->nes->cpu.bus & STATUS_OPEN_BUS;

    if(apu->frame_interrupt) value |= STATUS_FRAME_IRQ;

    if(apu->dmc_interrupt) value |= STATUS_DMC_IRQ;


    apu->frame_interrupt = false;
    CPU_ClearIRQ(&apu->nes->cpu,IRQ_FRAME);

    apu->nes->cpu.bus = value;
    
    return value;
}


void FrameCounter_Clock(FrameCounter *frame_counter,APU *apu){
    
    if(apu->cycles == frame_counter->next_clock){

        if(frame_counter->sequence_mode == SEQUENCE_FOUR_STEP){

            switch(frame_counter->sequence_position){
                case 0:
                    QuarterFrame_Clock(apu);

                    frame_counter->next_clock += 7456;

                    break;
                case 1:
                    QuarterFrame_Clock(apu);

                    HalfFrame_Clock(apu);

                    frame_counter->next_clock += 7458;

                    break;
                case 2:
                    QuarterFrame_Clock(apu);

                    frame_counter->next_clock += 7457;

                    break;
                case 3:
                    if(frame_counter->interrupt_inhibit == false){
                        apu->frame_interrupt = true;
                    }

                    frame_counter->next_clock += 1;

                    break;
                case 4:
                    QuarterFrame_Clock(apu);

                    HalfFrame_Clock(apu);

                    if(frame_counter->interrupt_inhibit == false){
                        apu->frame_interrupt = true;
                    }
                    
                    frame_counter->next_clock += 1;

                    break;
                case 5:
                    if(frame_counter->interrupt_inhibit == false){
                        apu->frame_interrupt = true;
                    }

                    frame_counter->next_clock += 7457;

                    break;
            }

            frame_counter->sequence_position = (frame_counter->sequence_position + 1) % 6; 
        }
        else if(frame_counter->sequence_mode == SEQUENCE_FIVE_STEP){

            switch(frame_counter->sequence_position){
                case 0:
                    QuarterFrame_Clock(apu);

                    HalfFrame_Clock(apu);

                    frame_counter->next_clock += 7458;

                    break;
                case 1:
                    QuarterFrame_Clock(apu);

                    frame_counter->next_clock += 7456;

                    break;
                case 2:
                    QuarterFrame_Clock(apu);

                    HalfFrame_Clock(apu);

                    frame_counter->next_clock += 7458;

                    break;
                case 3:
                    QuarterFrame_Clock(apu);

                    frame_counter->next_clock += 7456;

                    break;
                case 4:
                    frame_counter->next_clock += 7454;

                    break;
            }

            frame_counter->sequence_position = (frame_counter->sequence_position + 1) % 5;
        }
    }

}



void LengthCounter_Load(LengthCounter *length_counter,uint8_t index,uint32_t current_cicle){
    
    if(length_counter->counter.reload){
        length_counter->counter.value = length_counter->counter.reload_value;
    }

    length_counter->counter.reload_value = length_table[index];
    length_counter->counter.reload = true;
    length_counter->counter.reload_cicle = current_cicle;
}

uint8_t LengthCounter_Value(LengthCounter *length_counter){
    if(length_counter->counter.reload){
        length_counter->counter.value = length_counter->counter.reload_value;
        length_counter->counter.reload = false;
    }
    return length_counter->counter.value;
}

void LengthCounter_Clear(LengthCounter *length_counter){
    length_counter->counter.reload_value = 0;
    length_counter->counter.reload = true;
}

void LengthCounter_Halt(LengthCounter *length_counter,bool value,uint32_t current_cicle){
    length_counter->halt_flag.reload_value = value;
    length_counter->halt_flag.reload = true;
    length_counter->halt_flag.reload_cicle = current_cicle;
}

void LengthCounter_Clock(LengthCounter *length_counter,uint32_t current_cicle){
    
    bool length_clocked = true;

    if(length_counter->counter.reload){
        
        if(length_counter->counter.reload_cicle == current_cicle){
            
            if(length_counter->counter.value == 0){
                length_counter->counter.value = length_counter->counter.reload_value;
                length_clocked = false;
            }
            else{
                //Not Reload
            }
        }
        else{
            length_counter->counter.value = length_counter->counter.reload_value;
        }

        length_counter->counter.reload = false;
    }

    if(length_counter->halt_flag.reload && length_counter->halt_flag.reload_cicle != current_cicle){
        length_counter->halt_flag.value = length_counter->halt_flag.reload_value;
        length_counter->halt_flag.reload = false;
    }

    if(length_clocked && !length_counter->halt_flag.value && length_counter->counter.value > 0){
        length_counter->counter.value--;
    }
}



void LinearCounter_Clock(LinearCounter *linear_counter){
    if(linear_counter->counter.reload_flag){
        linear_counter->counter.value = linear_counter->counter.reload_value;
    }
    else if(linear_counter->counter.value > 0){
        linear_counter->counter.value--;
    }

    if(linear_counter->halt_flag == false){
        linear_counter->counter.reload_flag = false;
    }
}



uint16_t Sweep_UpdateTargetPeriod(Sweep *sweep){
    int change_amount = sweep->pulse->period >> sweep->shift;
    if(sweep->negate_flag) change_amount = -(change_amount + sweep->pulse->carry);
    sweep->target_period = sweep->pulse->period + change_amount;
}

void Sweep_SetControl(Sweep *sweep,uint8_t value){
    //EPPP.NSSS
    sweep->enabled = (value & 0x80) ? true : false;
    sweep->period = ((value & 0x70) >> 0x04) + 1;
    sweep->negate_flag = (value & 0x08) ? true : false;
    sweep->shift = value & 0x07;

    Sweep_UpdateTargetPeriod(sweep);

    sweep->reload_flag = true;
}

void Sweep_Clock(Sweep *sweep){

    if(!sweep->counter && sweep->enabled && sweep->shift && !SWEEP_SILENCED(sweep)){
        sweep->pulse->period = sweep->target_period;
        Sweep_UpdateTargetPeriod(sweep);
    }

    if(!sweep->counter || sweep->reload_flag){
        sweep->counter = sweep->period;
        sweep->reload_flag = false;
    }
    else{
        sweep->counter--;
    }
}


void Envelope_SetControl(Envelope *envelope,uint8_t value){
    //--LC.VVVV
    envelope->loop_flag = (value & 0x20) ? true : false;
    envelope->constant_volume = (value & 0x10) ? true : false;
    envelope->volume = value & 0x0F;
}

void Envelope_Clock(Envelope *envelope){
    if(envelope->start_flag){
        envelope->start_flag = false;
        envelope->counter = 0x0F;
        envelope->devider = envelope->volume;
    }
    else{
        if(envelope->devider-- == 0){
            envelope->devider = envelope->volume;

            if(envelope->counter > 0){
                envelope->counter--;
            }
            else if(envelope->loop_flag){
                envelope->counter = 0x0F;
            }
        }
    }
}


void Pulse_Clock(Pulse *pulse){
    if(pulse->timer-- == 0){
        pulse->timer = (pulse->period + 1) * 2;
        pulse->duty_position = (pulse->duty_position + 1) % 8;
    }
}

void Triangle_Clock(Triangle *triangle){
    if(triangle->timer-- == 0){
        triangle->timer = triangle->period + 1;
        if(LengthCounter_Value(&triangle->length_counter) > 0 && triangle->linear_counter.counter.value > 0){
            triangle->sequence_position = (triangle->sequence_position + 1) % 32;
        }
    }
}

void Noise_Clock(Noise *noise){
    if(noise->timer-- == 0){
        noise->timer = noise->period;
        uint8_t shift = (noise->mode_flag) ? 0x06 : 0x01;
        uint8_t feedback = (noise->lfsr ^ (noise->lfsr >> shift)) & 0x01;
        noise->lfsr >>= 0x01;
        noise->lfsr |= feedback << 0x0E;
    }
}

void DMC_StartClock(DMC *dmc,APU *apu){
    
    if(dmc->bytes_remaining == 0) return;

    dmc->bits_remaining = 8;

    if(dmc->sample_buffer_filled == false){
        dmc->silence = true;
    }
    else{
        dmc->silence = false;
        dmc->shift = dmc->sample_buffer;
        dmc->sample_buffer_filled = false;
    }

    dmc->sample_buffer = apu->nes->read(apu->nes,dmc->current_address);
    dmc->sample_buffer_filled = true;
    
    if(++dmc->current_address == 0){
        dmc->current_address = 0x8000;
    }
            
    if(--dmc->bytes_remaining == 0){
        if(dmc->loop_flag){
            dmc->current_address = dmc->sample_address;
            dmc->bytes_remaining = dmc->sample_length;
        }
        else if(dmc->irq_enabled){
            apu->dmc_interrupt = true;
            CPU_IRQ(&apu->nes->cpu,IRQ_DMC);
        }
    }
}

bool DMC_OutputClock(DMC *dmc){
    
    if(dmc->bits_remaining > 0){
        
        if(dmc->silence == false){
            int16_t change = (dmc->shift & 0x01) ? 2 : -2;
            int16_t new_level = dmc->output_level + change; 
                
            if(new_level >= 0 && new_level <= 127){
                dmc->output_level = new_level;
            }
        }

        dmc->shift >>= 1;

        dmc->bits_remaining--;

        return dmc->bits_remaining == 0;
    }

    return true;
}

void DMC_Clock(DMC *dmc,APU *apu){
    if(dmc->timer-- == 0){
        dmc->timer = dmc->period;
        
        if(DMC_OutputClock(dmc)){
            DMC_StartClock(dmc,apu);
        }
    }
}


uint8_t Pulse_Output(Pulse *pulse){
    if(
        pulse_sequence[pulse->duty][pulse->duty_position] == 0 || 
        SWEEP_SILENCED(&pulse->sweep) || 
        LengthCounter_Value(&pulse->length_counter) == 0
    ){
        return 0;
    }

    uint8_t output = 0;

    if(pulse->envelope.constant_volume){
        output = pulse->envelope.volume & 0x0F;
    }
    else{
        output = pulse->envelope.counter & 0x0F;
    }
    
    return output;
}

uint8_t Triangle_Output(Triangle *triangle){
    return triangle_sequence[triangle->sequence_position];
}

uint8_t Noise_Output(Noise *noise){
    if((noise->lfsr & 0x01) || LengthCounter_Value(&noise->length_counter) == 0){
        return 0;
    }
    
    uint8_t output = 0;

    if(noise->envelope.constant_volume){
        output = noise->envelope.volume & 0x0F;
    }
    else{
        output = noise->envelope.counter & 0x0F;
    }

    return output;
}

uint8_t DMC_Output(DMC *dmc){
    return dmc->output_level;
}