#include <NES/CPU.h>
#include <NES/PPU.h>
#include <Debugger/Breakpoints/BreakpointData.h>
#include <DebugMemory.h>


#define IsCrossingPage(address,offset)((address & 0xFF00) != ((address + offset) & 0xFF00))


static void SetFlag(uint8_t *dst,uint8_t flag,bool condition){
    if(condition) *dst |= flag;
    else          *dst &= ~flag;
}

static void push_16_bits(CPU *cpu,uint16_t value){
    cpu->nes->write(cpu->nes,(value >> 0x08) & 0xFF,0x0100 + cpu->stack_pointer--);
    cpu->nes->write(cpu->nes,value & 0xFF,0x0100 + cpu->stack_pointer--);
}

static uint16_t pop_16_bits(CPU *cpu){
    uint16_t low_byte  = cpu->nes->read(cpu->nes,0x0100 + ++cpu->stack_pointer);
    uint16_t high_byte = cpu->nes->read(cpu->nes,0x0100 + ++cpu->stack_pointer);
    return (high_byte << 0x08) | low_byte;
}

static void push_8_bits(CPU *cpu,uint8_t value){
    cpu->nes->write(cpu->nes,value,0x0100 + cpu->stack_pointer--);
}

static uint8_t pop_8_bits(CPU *cpu){
    return cpu->nes->read(cpu->nes,0x0100 + ++cpu->stack_pointer);
}


static void branch(CPU *cpu,bool condition){
    uint16_t pc = cpu->program_counter + cpu->instruction_table[cpu->opcode].bytes;

    if(condition){
        int8_t offset = READ_8_BITS(cpu,cpu->program_counter + 1);
        uint16_t address = pc + offset;
        
        if((address & 0xFF00) == (pc & 0xFF00)){
            if((cpu->irq_source & IRQ_MASK) && !(cpu->status_register & CPU_FLAG_I)) cpu->irq_delay = true;   
        }

        pc = address;
    }

    cpu->program_counter = pc;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Address Mode

//REL address relative
uint16_t REL(CPU *cpu){
    bool condition = false;

    switch(cpu->opcode){
        //BCC branch on carry clear
        case 0x90: if(!(cpu->status_register & CPU_FLAG_C)) condition = true; break;

        //BCS branch on carry set
        case 0xB0: if(cpu->status_register & CPU_FLAG_C)    condition = true; break;

        //BEQ branch on equal (zero set)
        case 0xF0: if(cpu->status_register & CPU_FLAG_Z)    condition = true; break;

        //BMI branch on minus (negative set)
        case 0x30: if(cpu->status_register & CPU_FLAG_N)    condition = true; break;

        //BNE branch on not equal (zero clear)
        case 0xD0: if(!(cpu->status_register & CPU_FLAG_Z)) condition = true; break;

        //BPL branch on plus (negative clear)
        case 0x10: if(!(cpu->status_register & CPU_FLAG_N)) condition = true; break;

        //BVC branch on overflow clear 
        case 0x50: if(!(cpu->status_register & CPU_FLAG_V)) condition = true; break;

        //BVS branch on overflow set
        case 0x70: if(cpu->status_register & CPU_FLAG_V)    condition = true; break;
    }

    if(condition){
        uint16_t pc = cpu->program_counter + cpu->instruction_table[cpu->opcode].bytes;
        int8_t offset = READ_8_BITS(cpu,cpu->program_counter + 1);
        uint16_t address = pc + offset;
        
        if((address & 0xFF00) == (pc & 0xFF00)){
            cpu->cycles += 1;
        }
        else{
            cpu->cycles += 2;
        }
    }
    
    return 0;
}

//IMP address implied
uint16_t IMP(CPU *cpu){
    return 0;
}

//ACU address accumulator
uint16_t ACU(CPU *cpu){
    return 0;
}

//IME address imediate
uint16_t IME(CPU *cpu){
    return cpu->program_counter + 1;
}

//ABS address absolute
uint16_t ABS(CPU *cpu){ 
    return READ_16_BITS(cpu,cpu->program_counter + 1);
}

//ZP0 address zero page
uint16_t ZP0(CPU *cpu){
    return READ_8_BITS(cpu,cpu->program_counter + 1) & 0xFF;
}

//ABX address absolute x
uint16_t ABX(CPU *cpu){
    uint16_t address = READ_16_BITS(cpu,cpu->program_counter + 1);

    bool is_page_crossing = IsCrossingPage(address,cpu->x_register);

    //Dummy Read
    if(is_page_crossing){
        cpu->nes->read(cpu->nes,(address & 0xFF00) | ((address + cpu->x_register) & 0xFF));
    }

    //(ADC,AND,CMP,EOR,LDA,LDY,ORA,SBC) contains 1 more cycle if the page is crossed
    //(ASL,DEC,INC,LSR,ROL,ROR,STA) spends fixed cycles
    switch(cpu->opcode){
        case 0x7D: case 0x3D: case 0xDD: case 0x5D: case 0xBD: case 0xBC: case 0x1D: case 0xFD:
            if(is_page_crossing){
                cpu->cycles++;
            }
            break;
    }

    return address + cpu->x_register;
}

//ABY address absolute y
uint16_t ABY(CPU *cpu){
    uint16_t address = READ_16_BITS(cpu,cpu->program_counter + 1);
    //(ADC,AND,CMP,EOR,LAS,LAX,LDA,LDX,ORA,SBC) contains 1 more cycle if the page is crossed
    //(DCP,ISC,RLA,RRA,SHA,SHS,SHX,SLO,SRE,STA) spends fixed cycles
    switch(cpu->opcode){
        case 0x79: case 0x39: case 0xD9: case 0x59: case 0xBB: case 0xBF: case 0xB9: case 0xBE: case 0x19: case 0xF9:
            if((address & 0xFF00) != ((address + cpu->y_register) & 0xFF00)){
                cpu->cycles++;
            }
            break;
    }
    return address + cpu->y_register;
}

//ZPX address zero page x
uint16_t ZPX(CPU *cpu){
    uint16_t address = READ_8_BITS(cpu,cpu->program_counter + 1);
    uint16_t effective_address = (address + cpu->x_register) & 0xFF;
    return effective_address;
}

//ZPY address zero page y
uint16_t ZPY(CPU *cpu){
    uint16_t address = READ_8_BITS(cpu,cpu->program_counter + 1);
    uint16_t effective_address = (address + cpu->y_register) & 0xFF;
    return effective_address;
}

//IND address indirect
uint16_t IND(CPU *cpu){
    uint16_t address = READ_16_BITS(cpu,cpu->program_counter + 1);
    uint16_t lsb = cpu->nes->read(cpu->nes,address);
    uint16_t msb = cpu->nes->read(cpu->nes,(address & 0xFF00) | ((address + 1) & 0xFF));
    return (msb << 8 ) | lsb;
}

//IDX address indirect x
uint16_t IDX(CPU *cpu){
    uint16_t address = (READ_8_BITS(cpu,cpu->program_counter + 1) + cpu->x_register) & 0xFF;
    uint16_t lsb = cpu->nes->read(cpu->nes,address);
    uint16_t msb = cpu->nes->read(cpu->nes,(address + 1) & 0xFF);
    return (msb << 8) | lsb; 
}

//IDY address indirect y
uint16_t IDY(CPU *cpu){

    uint16_t address = READ_8_BITS(cpu,cpu->program_counter + 1);

    uint16_t lsb = cpu->nes->read(cpu->nes,address & 0xFF);
    uint16_t msb = cpu->nes->read(cpu->nes,(address + 1) & 0xFF);

    address = (msb << 8) | lsb;

    bool is_page_crossing = IsCrossingPage(address,cpu->y_register);

    //Dummy Read
    if(is_page_crossing){
        cpu->nes->read(cpu->nes,(address & 0xFF00) | ((address + cpu->y_register) & 0xFF));
    }

    //(ADC,AND,CMP,EOR,LAX,LDA,ORA,SBC) contains 1 more cycle if the page is crossed
    //(DCP,ISC,RLA,RRA,SHA,SLO,SRE,STA) spends fixed cycles
    switch(cpu->opcode){
        case 0x71: case 0x31: case 0xD1: case 0x51: case 0xB3: case 0xB1: case 0x11: case 0xF1:
            if(is_page_crossing){
                cpu->cycles++;
            }
            break;
    }

    return address + cpu->y_register;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Transfer Instructions

//LDA load accumulator
static void LDA(CPU *cpu){
    cpu->accumulator = READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//LDX load X
static void LDX(CPU *cpu){
    cpu->x_register = READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->x_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->x_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//LDY load Y
static void LDY(CPU *cpu){
    cpu->y_register = READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->y_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->y_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//STA store accumulator
static void STA(CPU *cpu){
    WRITE_8_BITS(cpu,cpu->address,cpu->accumulator);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//STX store X
static void STX(CPU *cpu){
    WRITE_8_BITS(cpu,cpu->address,cpu->x_register);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//STY store Y
static void STY(CPU *cpu){
    WRITE_8_BITS(cpu,cpu->address,cpu->y_register);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TAX transfer accumulator to X
static void TAX(CPU *cpu){
    cpu->x_register = cpu->accumulator;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->x_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->x_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TAY transfer accumulator to Y
static void TAY(CPU *cpu){
    cpu->y_register = cpu->accumulator;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->y_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->y_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TSX transfer stack pointer to X
static void TSX(CPU *cpu){
    cpu->x_register = cpu->stack_pointer;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->x_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->x_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TXA transfer X to accumulator
static void TXA(CPU *cpu){
    cpu->accumulator = cpu->x_register;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TXS transfer X to stack pointer
static void TXS(CPU *cpu){
    cpu->stack_pointer = cpu->x_register;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//TYA transfer Y to accumulator
static void TYA(CPU *cpu){
    cpu->accumulator = cpu->y_register;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Stack Instructions

//PHA push accumulator
static void PHA(CPU *cpu){
    push_8_bits(cpu,cpu->accumulator);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//PHP push processor status register (with break flag set)
static void PHP(CPU *cpu){
    push_8_bits(cpu,cpu->status_register | CPU_FLAG_B);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//PLA pull accumulator
static void PLA(CPU *cpu){
    cpu->accumulator = pop_8_bits(cpu);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//PLP pull processor status register
static void PLP(CPU *cpu){
    bool prev = cpu->status_register & CPU_FLAG_I;

    cpu->status_register = (pop_8_bits(cpu) | CPU_FLAG_U) & ~CPU_FLAG_B;

    bool current = cpu->status_register & CPU_FLAG_I;

    if((cpu->irq_source & IRQ_MASK) && prev && !current) cpu->irq_delay = true;

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Decrecment and Increment

//DEC decrement (memory)
static void DEC(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address) - 1;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,value == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,value & 0x80);
    WRITE_8_BITS(cpu,cpu->address,value);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//DEX decrement X
static void DEX(CPU *cpu){
    cpu->x_register--;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->x_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->x_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//DEY decrement Y
static void DEY(CPU *cpu){
    cpu->y_register--;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->y_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->y_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//INC increment (memory)
static void INC(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address) + 1;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,value == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,value & 0x80);
    WRITE_8_BITS(cpu,cpu->address,value);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//INX increment X
static void INX(CPU *cpu){
    cpu->x_register++;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->x_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->x_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//INY increment Y
static void INY(CPU *cpu){
    cpu->y_register++;
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->y_register == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->y_register & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Arithmetic Operations

//ADC add with carry (prepare by CLC)
static void ADC(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    uint8_t c = (cpu->status_register & CPU_FLAG_C) ? 0x01 : 0x00;
    uint32_t result = cpu->accumulator + value + c;

    SetFlag(&cpu->status_register,CPU_FLAG_C,result >> 0x08);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_V,~(cpu->accumulator ^ value) & (cpu->accumulator ^ result) & 0x80);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & 0x80);

    cpu->accumulator = result & 0xFF;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SBC subtract with carry (prepare by SEC)
static void SBC(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    uint8_t c = (cpu->status_register & CPU_FLAG_C) ? 0x00 : 0x01;
    uint32_t result = cpu->accumulator - value - c;

    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_V,(cpu->accumulator ^ value) & (cpu->accumulator ^ result) & 0x80);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & 0x80);

    cpu->accumulator = result & 0xFF;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Logical Operations

//AND and (with accumulator)
static void AND(CPU *cpu){
    cpu->accumulator &= READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//EOR exclusive or (with accumulator)
static void EOR(CPU *cpu){
    cpu->accumulator ^= READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ORA (inclusive) or with accumulator
static void ORA(CPU *cpu){
    cpu->accumulator |= READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Shift and Rotate Instructions

//ASL arithmetic shift left (shifts in a zero bit on the right)
static void ASL(CPU *cpu){
    if(cpu->instruction_table[cpu->opcode].address == ACU){
        uint8_t result = cpu->accumulator << 1;
        SetFlag(&cpu->status_register,CPU_FLAG_C,cpu->accumulator & BIT_7);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        cpu->accumulator = result;
    }
    else{
        uint8_t value = READ_8_BITS(cpu,cpu->address);
        uint8_t result = value << 1;
        SetFlag(&cpu->status_register,CPU_FLAG_C,value & BIT_7);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        WRITE_8_BITS(cpu,cpu->address,result);
    }
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//LSR logical shift right (shifts in a zero bit on the left)
static void LSR(CPU *cpu){
    if(cpu->instruction_table[cpu->opcode].address == ACU){
        uint8_t result = cpu->accumulator >> 1;
        SetFlag(&cpu->status_register,CPU_FLAG_C,cpu->accumulator & BIT_0);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        cpu->accumulator = result;
    }
    else{
        uint8_t value = READ_8_BITS(cpu,cpu->address);
        uint8_t result = value >> 1;
        SetFlag(&cpu->status_register,CPU_FLAG_C,value & BIT_0);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        WRITE_8_BITS(cpu,cpu->address,result);
    }
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ROL rotate left (shifts in carry bit on the right)
static void ROL(CPU *cpu){
    if(cpu->instruction_table[cpu->opcode].address == ACU){
        uint8_t result = cpu->accumulator << 1;
        
        SetFlag(&result,BIT_0,cpu->status_register & CPU_FLAG_C);
        SetFlag(&cpu->status_register,CPU_FLAG_C,cpu->accumulator & BIT_7);

        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        cpu->accumulator = result;
    }
    else{
        uint8_t value = READ_8_BITS(cpu,cpu->address);
        uint8_t result = value << 1;
        SetFlag(&result,BIT_0,cpu->status_register & CPU_FLAG_C);
        SetFlag(&cpu->status_register,CPU_FLAG_C,value & BIT_7);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        WRITE_8_BITS(cpu,cpu->address,result);
    }
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ROR rotate right (shifts in zero bit on the left)
static void ROR(CPU *cpu){
    if(cpu->instruction_table[cpu->opcode].address == ACU){
        uint8_t result = cpu->accumulator >> 1;
        SetFlag(&result,BIT_7,cpu->status_register & CPU_FLAG_C);
        SetFlag(&cpu->status_register,CPU_FLAG_C,cpu->accumulator & BIT_0);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        cpu->accumulator = result;
    }
    else{
        uint8_t value = READ_8_BITS(cpu,cpu->address);
        uint8_t result = value >> 1;
        SetFlag(&result,BIT_7,cpu->status_register & CPU_FLAG_C);
        SetFlag(&cpu->status_register,CPU_FLAG_C,value & BIT_0);
        SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
        SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
        WRITE_8_BITS(cpu,cpu->address,result);
    }
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Flag Instructions

//CLC clear carry
static void CLC(CPU *cpu){
    cpu->status_register &= ~CPU_FLAG_C;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//CLD clear decimal (BCD arithmetics disabled)
static void CLD(CPU *cpu){
    cpu->status_register &= ~CPU_FLAG_D;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//CLI clear interrupt disable
static void CLI(CPU *cpu){
    bool prev = cpu->status_register & CPU_FLAG_I;

    cpu->status_register &= ~CPU_FLAG_I;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;

    if(cpu->irq_source & IRQ_MASK && prev) cpu->irq_delay = true;
}

//CLV clear overflow
static void CLV(CPU *cpu){
    cpu->status_register &= ~CPU_FLAG_V;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SEC set carry
static void SEC(CPU *cpu){
    cpu->status_register |= CPU_FLAG_C;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SED set decimal (BCD arithmetics enabled)
static void SED(CPU *cpu){
    cpu->status_register |= CPU_FLAG_D;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SEI set interrupt disable
static void SEI(CPU *cpu){
    cpu->status_register |= CPU_FLAG_I;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Comparations

//CMP compare with accumulator
static void CMP(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    uint32_t result = cpu->accumulator - value;
    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//CPX compare with X
static void CPX(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    uint32_t result = cpu->x_register - value;
    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//CPY compare with Y
static void CPY(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    uint32_t result = cpu->y_register - value;
    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Conditional Branch Instructions

//BCC branch on carry clear
static void BCC(CPU *cpu){
    branch(cpu,!(cpu->status_register & CPU_FLAG_C));
}

//BCS branch on carry set
static void BCS(CPU *cpu){
    branch(cpu,cpu->status_register & CPU_FLAG_C);
}

//BEQ branch on equal (zero set)
static void BEQ(CPU *cpu){
    branch(cpu,cpu->status_register & CPU_FLAG_Z);
}

//BMI branch on minus (negative set)
static void BMI(CPU *cpu){
    branch(cpu,cpu->status_register & CPU_FLAG_N);
}

//BNE branch on not equal (zero clear)
static void BNE(CPU *cpu){
    branch(cpu,!(cpu->status_register & CPU_FLAG_Z));
}

//BPL branch on plus (negative clear)
static void BPL(CPU *cpu){
    branch(cpu,!(cpu->status_register & CPU_FLAG_N));
}

//BVC branch on overflow clear
static void BVC(CPU *cpu){
    branch(cpu,!(cpu->status_register & CPU_FLAG_V));
}

//BVS branch on overflow set
static void BVS(CPU *cpu){
    branch(cpu,cpu->status_register & CPU_FLAG_V);
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Jumps and Subroutines

//JMP jump to new Location
static void JMP(CPU *cpu){
    cpu->program_counter = cpu->address;
}

//JSR jump subruntime
static void JSR(CPU *cpu){
    push_16_bits(cpu,cpu->program_counter + 2);
    cpu->program_counter = cpu->address;
}

//RTS return from subruntime
static void RTS(CPU *cpu){
    cpu->program_counter = pop_16_bits(cpu) + 1;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Interrupts

//RST
static void RST(CPU *cpu){
    cpu->stack_pointer -= 0x03;

    SetFlag(&cpu->status_register,CPU_FLAG_I,true);

    cpu->program_counter = READ_16_BITS(cpu,RES_ADDRESS);

    cpu->rst_executing = false;
}

//NMI
static void NMI(CPU *cpu){

    push_16_bits(cpu,cpu->program_counter);

    SetFlag(&cpu->status_register,CPU_FLAG_B,false);

    push_8_bits(cpu,cpu->status_register);

    SetFlag(&cpu->status_register,CPU_FLAG_I,true);

    cpu->program_counter = READ_16_BITS(cpu,NMI_ADDRESS);

    cpu->nmi_executing = false;
}

//IRQ
static void IRQ(CPU *cpu){

    push_16_bits(cpu,cpu->program_counter);

    SetFlag(&cpu->status_register,CPU_FLAG_B,false);

    push_8_bits(cpu,cpu->status_register);

    SetFlag(&cpu->status_register,CPU_FLAG_I,true);

    if(cpu->nmi_asserted && cpu->total_cycles - cpu->request_nmi_cycle >= 2){
        cpu->nmi_asserted = false;
        cpu->program_counter = READ_16_BITS(cpu,NMI_ADDRESS);
    }
    else{
        cpu->program_counter = READ_16_BITS(cpu,IRQ_ADDRESS);
        if(cpu->nmi_asserted) cpu->nmi_delay = true;
    }

    cpu->irq_executing = false;
}

//BRK
static void BRK(CPU *cpu){

    push_16_bits(cpu,cpu->program_counter + 2);
    
    push_8_bits(cpu,cpu->status_register | CPU_FLAG_B);

    SetFlag(&cpu->status_register,CPU_FLAG_I,true);

    if(cpu->nmi_asserted && cpu->total_cycles - cpu->request_nmi_cycle >= 2){
        cpu->nmi_asserted = false;
        cpu->program_counter = READ_16_BITS(cpu,NMI_ADDRESS);
    }
    else{
        cpu->program_counter = READ_16_BITS(cpu,IRQ_ADDRESS);
        if(cpu->nmi_asserted) cpu->nmi_delay = true;
    }
}

//RTI return from Interrupt
static void RTI(CPU *cpu){
    cpu->status_register = pop_8_bits(cpu) | CPU_FLAG_U;
    cpu->program_counter = pop_16_bits(cpu);
    cpu->irq_need = false;
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Other

//BIT bit test (accumulator and memory)
static void BIT(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(cpu->accumulator & value) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_V,value & BIT_6);
    SetFlag(&cpu->status_register,CPU_FLAG_N,value & BIT_7);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//NOP no operation
static void NOP(CPU *cpu){
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}


//--------------------------------------------------------------------------------------------------------------------------------------
//Illegal Instructions

//(ALR,ASR) "AND" then Logical Shift Right
static void ALR(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    uint8_t value = cpu->accumulator & memory_value;

    uint8_t result = value >> 1;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_C,value & 0x01);
    SetFlag(&cpu->status_register,CPU_FLAG_N,false);

    cpu->accumulator = result;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ANC "AND" Memory with Accumulator then Move Negative Flag to Carry Flag
static void ANC(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);
    
    cpu->accumulator &= memory_value;
    
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);

    SetFlag(&cpu->status_register,CPU_FLAG_C,cpu->status_register & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//(ANE,XAA) Non-deterministic Operation of Accumulator, cpu->program_counter Register X, Memory and Bus Contents
static void ANE(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);
    
    cpu->accumulator = (cpu->accumulator & 0xFF) & cpu->x_register & memory_value;
    
    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ARR "AND" Accumulator then Rotate Right
static void ARR(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    uint8_t result = memory_value & cpu->accumulator;

    result = ((cpu->status_register & CPU_FLAG_C) << 0x07) | ((result >> 0x01) & 0x7F);

    SetFlag(&cpu->status_register,CPU_FLAG_C,result & 0x40);
    SetFlag(&cpu->status_register,CPU_FLAG_V,((result & 0x40) >> 1) ^ (result & 0x20));
    
    SetFlag(&cpu->status_register,CPU_FLAG_N,result & BIT_7);
    SetFlag(&cpu->status_register,CPU_FLAG_Z,result == 0);

    cpu->accumulator = result;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//DCP Decrement Memory By One then Compare with Accumulator
static void DCP(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address) - 1;

    WRITE_8_BITS(cpu,cpu->address,memory_value);

    uint16_t result = cpu->accumulator - memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result & 0x8000));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//ISC Increment Memory By One then SBC then Subtract Memory from Accumulator with Borrow
static void ISC(CPU *cpu){

    uint8_t value = READ_8_BITS(cpu,cpu->address) + 1;

    WRITE_8_BITS(cpu,cpu->address,value);
    
    uint8_t c = (cpu->status_register & CPU_FLAG_C) ? 0x00 : 0x01;
    uint16_t result = cpu->accumulator - value - c;
    
    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_V,(cpu->accumulator ^ value) & (cpu->accumulator ^ result) & 0x80);

    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & 0x80);
    
    cpu->accumulator = result & 0xFF;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//(LAS,LAR) "AND" Memory with Stack Pointer
static void LAS(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address) & cpu->stack_pointer;

    cpu->accumulator = memory_value;
    cpu->x_register = memory_value;
    cpu->stack_pointer = memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,memory_value == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,memory_value & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//LAX Load Accumulator and cpu->program_counter Register X From Memory
static void LAX(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    cpu->accumulator = memory_value;
    cpu->x_register = memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,memory_value == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,memory_value & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//RLA Rotate Left then "AND" with Accumulator
static void RLA(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    uint8_t bit = cpu->status_register & CPU_FLAG_C;

    SetFlag(&cpu->status_register,CPU_FLAG_C,memory_value & BIT_7);

    memory_value = (memory_value << 0x01) | bit;

    WRITE_8_BITS(cpu,cpu->address,memory_value);

    cpu->accumulator &= memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//RRA Rotate Right and Add Memory to Accumulator
static void RRA(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    uint8_t bit = (cpu->status_register & CPU_FLAG_C) << 0x07;

    SetFlag(&cpu->status_register,CPU_FLAG_C,memory_value & 0x01);

    memory_value = ((memory_value >> 0x01) & 0x7F) | bit;

    WRITE_8_BITS(cpu,cpu->address,memory_value);

    uint8_t c = (cpu->status_register & CPU_FLAG_C) ? 0x01 : 0x00;
    uint16_t result = cpu->accumulator + memory_value + c;

    SetFlag(&cpu->status_register,CPU_FLAG_C,result >> 0x08);
    SetFlag(&cpu->status_register,CPU_FLAG_V,(~(cpu->accumulator ^ memory_value)) & (cpu->accumulator ^ result) & 0x80);

    cpu->accumulator = result & 0xFF;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & 0x80);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//(SAX,AXS,AAX) Store Accumulator "AND" cpu->program_counter Register X in Memory
static void SAX(CPU *cpu){
    WRITE_8_BITS(cpu,cpu->address,cpu->accumulator & cpu->x_register);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SBX Subtract Memory from Accumulator "AND" cpu->program_counter Register X
static void SBX(CPU *cpu){
    uint8_t value = READ_8_BITS(cpu,cpu->address);

    uint16_t result = (cpu->x_register & cpu->accumulator) - value;

    SetFlag(&cpu->status_register,CPU_FLAG_C,!(result >> 0x08));
    SetFlag(&cpu->status_register,CPU_FLAG_Z,(result & 0xFF) == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,(result & 0xFF) & 0x80);
    
    cpu->x_register = result & 0xFF;
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SHA Store Accumulator "AND" cpu->program_counter Register X "AND" Value
static void SHA(CPU *cpu){
    cpu->x_register &= cpu->accumulator;
    cpu->x_register &= 0x07;
    WRITE_8_BITS(cpu,cpu->address,cpu->x_register);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SHX Store cpu->program_counter Register X "AND" Value
static void SHX(CPU *cpu){
    uint8_t high_byte = cpu->address >> 0x08;
    uint8_t result = cpu->x_register & (high_byte + 1);
    uint16_t address = (result << 0x08) | (cpu->address & 0xFF);
    WRITE_8_BITS(cpu,address,result);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SHY Store cpu->program_counter Register Y "AND" Value
static void SHY(CPU *cpu){
    uint8_t high_byte = cpu->address >> 0x08;
    uint8_t result = cpu->y_register & (high_byte + 1);
    uint16_t address = (result << 0x08) | (cpu->address & 0xFF);
    WRITE_8_BITS(cpu,address,result);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SLO Arithmetic Shift Left then "OR" Memory with Accumulator
static void SLO(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    SetFlag(&cpu->status_register,CPU_FLAG_C,memory_value & BIT_7);

    memory_value <<= 0x01;

    WRITE_8_BITS(cpu,cpu->address,memory_value);

    cpu->accumulator |= memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & BIT_7);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//SRE Logical Shift Right then "Exclusive OR" Memory with Accumulator
static void SRE(CPU *cpu){
    uint8_t memory_value = READ_8_BITS(cpu,cpu->address);

    SetFlag(&cpu->status_register,CPU_FLAG_C,memory_value & BIT_0);

    memory_value >>= 0x01;

    WRITE_8_BITS(cpu,cpu->address,memory_value);

    cpu->accumulator ^= memory_value;

    SetFlag(&cpu->status_register,CPU_FLAG_Z,cpu->accumulator == 0x00);
    SetFlag(&cpu->status_register,CPU_FLAG_N,cpu->accumulator & BIT_7);

    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//(TAS,SHS,XAS) Transfer Accumulator "AND" cpu->program_counter Register X to Stack Pointer then Store Stack Pointer "AND" Hi-Byte In Memory
static void TAS(CPU *cpu){
    cpu->status_register = cpu->accumulator & cpu->x_register;
    uint8_t result = cpu->status_register & (((cpu->address >> 0x08) + 0x01) & 0xFF);
    WRITE_8_BITS(cpu,cpu->address,result);
    cpu->program_counter += cpu->instruction_table[cpu->opcode].bytes;
}

//(JAM,KIL,HLT) Reset CPU
static void JAM(CPU *cpu){
    CPU_Reset(cpu,HARDWARE_RESET);
}

//--------------------------------------------------------------------------------------------------------------------------------------
//Check IO

static bool _REL(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == REL);

    return false;
}

static bool _IMP(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == IMP);

    return false;
}

static bool _ACU(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ACU);

    return false;
}

static bool _IME(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == IME);

    return false;
}

static bool _ABS(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ABS);

    if(!(instruction.io & RW)) return false;

    uint16_t effective_address = Debug_CPU_Read16(cpu->nes,cpu->program_counter + 1);

    Range address_range;        
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _ZP0(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ZP0);

    if(!(instruction.io & RW)) return false;

    uint16_t effective_address = Debug_CPU_Read8(cpu->nes,cpu->program_counter + 1);

    Range address_range;
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;
    
    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _ABX(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];
    
    assert(instruction.address == ABX);

    uint16_t address = Debug_CPU_Read16(cpu->nes,cpu->program_counter + 1);

    Range address_range;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    //Dummy read
    if(IsCrossingPage(address,cpu->x_register)){
        address_range.start = address_range.end = (address & 0xFF00) | ((address + cpu->x_register) & 0xFF);

        if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,RD,address_range)){
            return true;
        }
    }

    if(!(instruction.io & RW)) return false;

    uint16_t effective_address = address + cpu->x_register;

    if(instruction.operation == SHY){
        uint8_t high_byte = effective_address >> 0x08;
        effective_address = ((cpu->y_register & (high_byte + 1)) << 0x08) | (effective_address & 0xFF);
    }

    address_range.start = address_range.end = effective_address;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _ABY(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ABY);

    if(!(instruction.io & RW)) return false;

    uint16_t address = Debug_CPU_Read16(cpu->nes,cpu->program_counter + 1);

    uint16_t effective_address = address + cpu->y_register;

    if(instruction.operation == SHX){
        uint8_t high_byte = effective_address >> 0x08;
        effective_address = ((cpu->x_register & (high_byte + 1)) << 0x08) | (effective_address & 0xFF);
    }

    Range address_range;
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _ZPX(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ZPX);

    if(!(instruction.io & RW)) return false;

    uint16_t address = Debug_CPU_Read8(cpu->nes,cpu->program_counter + 1);
    uint16_t effective_address = (address + cpu->x_register) & 0xFF;
    
    Range address_range;
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _ZPY(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == ZPY);

    if(!(instruction.io & RW)) return false;

    uint16_t address = Debug_CPU_Read8(cpu->nes,cpu->program_counter + 1);
    uint16_t effective_address = (address + cpu->y_register) & 0xFF;

    Range address_range;
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _IND(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == IND);

    return false;
}

static bool _IDX(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == IDX);

    if(!(instruction.io & RW)) return false;

    uint16_t address = (Debug_CPU_Read8(cpu->nes,cpu->program_counter + 1) + cpu->x_register) & 0xFF;
            
    uint16_t lsb = Debug_CPU_Read8(cpu->nes,address);
    uint16_t msb = Debug_CPU_Read8(cpu->nes,(address + 1) & 0xFF);

    uint16_t effective_address = (msb << 0x08) | lsb;

    Range address_range;
    address_range.start = address_range.end = effective_address;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

static bool _IDY(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.address == IDY);
        
    uint16_t address = Debug_CPU_Read8(cpu->nes,cpu->program_counter + 1);
    
    uint16_t lsb = Debug_CPU_Read8(cpu->nes,address & 0xFF);
    uint16_t msb = Debug_CPU_Read8(cpu->nes,(address + 1) & 0xFF);
            
    address = (msb << 0x08) | lsb;

    Range address_range;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    //Dummy read
    if(IsCrossingPage(address,cpu->y_register)){
        address_range.start = address_range.end = (address & 0xFF00) | ((address + cpu->y_register) & 0xFF);

        if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,RD,address_range)){
            return true;
        }
    }

    if(!(instruction.io & RW)) return false;

    uint16_t effective_address = address + cpu->y_register;

    address_range.start = address_range.end = effective_address;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Write 1 byte in stack (PHA,PHP)
static bool _PH1(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == WR && (instruction.operation == PHA || instruction.operation == PHP));

    Range address_range;
    address_range.start = address_range.end = 0x100 + cpu->stack_pointer;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Read 1 byte in stack (PLA,PLP)
static bool _PL1(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == RD && (instruction.operation == PLA || instruction.operation == PLP));

    Range address_range;
    address_range.start = address_range.end = 0x100 + cpu->stack_pointer + 1;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Write 2 bytes in stack (JSR)
static bool _JSR(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == WR && instruction.operation == JSR);

    Range address_range;
    address_range.start = 0x100 + cpu->stack_pointer - 1;
    address_range.end = 0x100 + cpu->stack_pointer;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Read 2 bytes in stack (RTS)
static bool _RTS(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == RD && instruction.operation == RTS);

    Range address_range;
    address_range.start = 0x100 + cpu->stack_pointer + 1;
    address_range.end = 0x100 + cpu->stack_pointer + 2;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Write 3 bytes in stack (BRK)
static bool _BRK(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == WR && instruction.operation == BRK);

    Range address_range;
    address_range.start = 0x100 + cpu->stack_pointer - 2;
    address_range.end = 0x100 + cpu->stack_pointer;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

// Read 3 bytes in stack (RTI)
static bool _RTI(CPU *cpu){
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];

    assert(instruction.io == RD && instruction.operation == RTI);

    Range address_range;
    address_range.start = 0x100 + cpu->stack_pointer + 1;
    address_range.end = 0x100 + cpu->stack_pointer + 3;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,instruction.io,address_range)){
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------


static const CPU_Instruction instruction_table[0x100] = {
    //                    0                                       1                                    2                                    3                                     4                                   5                                    6                                    7                                    8                                    9                                    A                                    B                                    C                                    D                                    E                                    F
    /* 0 */ { L"BRK", BRK, IMP, _BRK, 7, 1, WR }, { L"ORA", ORA, IDX, _IDX, 6, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"SLO", SLO, IDX, _IDX, 8, 2, RW },{ L"NOP", NOP, ZP0, _ZP0, 3, 2, NN },{ L"ORA", ORA, ZP0, _ZP0, 3, 2, RD },{ L"ASL", ASL, ZP0, _ZP0, 5, 2, RW },{ L"SLO", SLO, ZP0, _ZP0, 5, 2, RW },{ L"PHP", PHP, IMP, _PH1, 3, 1, WR },{ L"ORA", ORA, IME, _IME, 2, 2, RD },{ L"ASL", ASL, ACU, _ACU, 2, 1, NN },{ L"ANC", ANC, IME, _IME, 2, 2, RD },{ L"NOP", NOP, ABS, _ABS, 4, 3, NN },{ L"ORA", ORA, ABS, _ABS, 4, 3, RD },{ L"ASL", ASL, ABS, _ABS, 6, 3, RW },{ L"SLO", SLO, ABS, _ABS, 6, 3, RW },
	/* 1 */ { L"BPL", BPL, REL, _REL, 2, 2, NN }, { L"ORA", ORA, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"SLO", SLO, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"ORA", ORA, ZPX, _ZPX, 4, 2, RD },{ L"ASL", ASL, ZPX, _ZPX, 6, 2, RW },{ L"SLO", SLO, ZPX, _ZPX, 6, 2, RW },{ L"CLC", CLC, IMP, _IMP, 2, 1, NN },{ L"ORA", ORA, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"SLO", SLO, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"ORA", ORA, ABX, _ABX, 4, 3, RD },{ L"ASL", ASL, ABX, _ABX, 7, 3, RW },{ L"SLO", SLO, ABX, _ABX, 7, 3, RW },
	/* 2 */ { L"JSR", JSR, ABS, _JSR, 6, 3, WR }, { L"AND", AND, IDX, _IDX, 6, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"RLA", RLA, IDX, _IDX, 8, 2, RW },{ L"BIT", BIT, ZP0, _ZP0, 3, 2, RD },{ L"AND", AND, ZP0, _ZP0, 3, 2, RD },{ L"ROL", ROL, ZP0, _ZP0, 5, 2, RW },{ L"RLA", RLA, ZP0, _ZP0, 5, 2, RW },{ L"PLP", PLP, IMP, _PL1, 4, 1, RD },{ L"AND", AND, IME, _IME, 2, 2, RD },{ L"ROL", ROL, ACU, _ACU, 2, 1, NN },{ L"ANC", ANC, IME, _IME, 2, 2, RD },{ L"BIT", BIT, ABS, _ABS, 4, 3, RD },{ L"AND", AND, ABS, _ABS, 4, 3, RD },{ L"ROL", ROL, ABS, _ABS, 6, 3, RW },{ L"RLA", RLA, ABS, _ABS, 6, 3, RW },
	/* 3 */ { L"BMI", BMI, REL, _REL, 2, 2, NN }, { L"AND", AND, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"RLA", RLA, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"AND", AND, ZPX, _ZPX, 4, 2, RD },{ L"ROL", ROL, ZPX, _ZPX, 6, 2, RW },{ L"RLA", RLA, ZPX, _ZPX, 6, 2, RW },{ L"SEC", SEC, IMP, _IMP, 2, 1, NN },{ L"AND", AND, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"RLA", RLA, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"AND", AND, ABX, _ABX, 4, 3, RD },{ L"ROL", ROL, ABX, _ABX, 7, 3, RW },{ L"RLA", RLA, ABX, _ABX, 7, 3, RW },
	/* 4 */ { L"RTI", RTI, IMP, _RTI, 6, 1, RD }, { L"EOR", EOR, IDX, _IDX, 6, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"SRE", SRE, IDX, _IDX, 8, 2, RW },{ L"NOP", NOP, ZP0, _ZP0, 3, 2, NN },{ L"EOR", EOR, ZP0, _ZP0, 3, 2, RD },{ L"LSR", LSR, ZP0, _ZP0, 5, 2, RW },{ L"SRE", SRE, ZP0, _ZP0, 5, 2, RW },{ L"PHA", PHA, IMP, _PH1, 3, 1, WR },{ L"EOR", EOR, IME, _IME, 2, 2, RD },{ L"LSR", LSR, ACU, _ACU, 2, 1, NN },{ L"ALR", ALR, IME, _IME, 2, 2, RD },{ L"JMP", JMP, ABS, _ABS, 3, 3, NN },{ L"EOR", EOR, ABS, _ABS, 4, 3, RD },{ L"LSR", LSR, ABS, _ABS, 6, 3, RW },{ L"SRE", SRE, ABS, _ABS, 6, 3, RW },
	/* 5 */ { L"BVC", BVC, REL, _REL, 2, 2, NN }, { L"EOR", EOR, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"SRE", SRE, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"EOR", EOR, ZPX, _ZPX, 4, 2, RD },{ L"LSR", LSR, ZPX, _ZPX, 6, 2, RW },{ L"SRE", SRE, ZPX, _ZPX, 6, 2, RW },{ L"CLI", CLI, IMP, _IMP, 2, 1, NN },{ L"EOR", EOR, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"SRE", SRE, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"EOR", EOR, ABX, _ABX, 4, 3, RD },{ L"LSR", LSR, ABX, _ABX, 7, 3, RW },{ L"SRE", SRE, ABX, _ABX, 7, 3, RW },
	/* 6 */ { L"RTS", RTS, IMP, _RTS, 6, 1, RD }, { L"ADC", ADC, IDX, _IDX, 6, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"RRA", RRA, IDX, _IDX, 8, 2, RW },{ L"NOP", NOP, ZP0, _ZP0, 3, 2, NN },{ L"ADC", ADC, ZP0, _ZP0, 3, 2, RD },{ L"ROR", ROR, ZP0, _ZP0, 5, 2, RW },{ L"RRA", RRA, ZP0, _ZP0, 5, 2, RW },{ L"PLA", PLA, IMP, _PL1, 4, 1, RD },{ L"ADC", ADC, IME, _IME, 2, 2, RD },{ L"ROR", ROR, ACU, _ACU, 2, 1, NN },{ L"ARR", ARR, IME, _IME, 2, 2, RD },{ L"JMP", JMP, IND, _IND, 5, 3, NN },{ L"ADC", ADC, ABS, _ABS, 4, 3, RD },{ L"ROR", ROR, ABS, _ABS, 6, 3, RW },{ L"RRA", RRA, ABS, _ABS, 6, 3, RW },
	/* 7 */ { L"BVS", BVS, REL, _REL, 2, 2, NN }, { L"ADC", ADC, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"RRA", RRA, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"ADC", ADC, ZPX, _ZPX, 4, 2, RD },{ L"ROR", ROR, ZPX, _ZPX, 6, 2, RW },{ L"RRA", RRA, ZPX, _ZPX, 6, 2, RW },{ L"SEI", SEI, IMP, _IMP, 2, 1, NN },{ L"ADC", ADC, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"RRA", RRA, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"ADC", ADC, ABX, _ABX, 4, 3, RD },{ L"ROR", ROR, ABX, _ABX, 7, 3, RW },{ L"RRA", RRA, ABX, _ABX, 7, 3, RW },
	/* 8 */ { L"NOP", NOP, IME, _IME, 2, 2, NN }, { L"STA", STA, IDX, _IDX, 6, 2, WR },{ L"NOP", NOP, IME, _IME, 2, 2, NN },{ L"SAX", SAX, IDX, _IDX, 6, 2, WR },{ L"STY", STY, ZP0, _ZP0, 3, 2, WR },{ L"STA", STA, ZP0, _ZP0, 3, 2, WR },{ L"STX", STX, ZP0, _ZP0, 3, 2, WR },{ L"SAX", SAX, ZP0, _ZP0, 3, 2, WR },{ L"DEY", DEY, IMP, _IMP, 2, 1, NN },{ L"NOP", NOP, IME, _IME, 2, 2, NN },{ L"TXA", TXA, IMP, _IMP, 2, 1, NN },{ L"ANE", ANE, IME, _IME, 2, 2, RD },{ L"STY", STY, ABS, _ABS, 4, 3, WR },{ L"STA", STA, ABS, _ABS, 4, 3, WR },{ L"STX", STX, ABS, _ABS, 4, 3, WR },{ L"SAX", SAX, ABS, _ABS, 4, 3, WR },
	/* 9 */ { L"BCC", BCC, REL, _REL, 2, 2, NN }, { L"STA", STA, IDY, _IDY, 6, 2, WR },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"SHA", SHA, IDY, _IDY, 6, 2, WR },{ L"STY", STY, ZPX, _ZPX, 4, 2, WR },{ L"STA", STA, ZPX, _ZPX, 4, 2, WR },{ L"STX", STX, ZPY, _ZPY, 4, 2, WR },{ L"SAX", SAX, ZPY, _ZPY, 4, 2, WR },{ L"TYA", TYA, IMP, _IMP, 2, 1, NN },{ L"STA", STA, ABY, _ABY, 5, 3, WR },{ L"TXS", TXS, IMP, _IMP, 2, 1, NN },{ L"TAS", TAS, ABY, _ABY, 5, 3, WR },{ L"SHY", SHY, ABX, _ABX, 5, 3, WR },{ L"STA", STA, ABX, _ABX, 5, 3, WR },{ L"SHX", SHX, ABY, _ABY, 5, 3, WR },{ L"SHA", SHA, ABY, _ABY, 5, 3, WR },
	/* A */ { L"LDY", LDY, IME, _IME, 2, 2, RD }, { L"LDA", LDA, IDX, _IDX, 6, 2, RD },{ L"LDX", LDX, IME, _IME, 2, 2, RD },{ L"LAX", LAX, IDX, _IDX, 6, 2, RD },{ L"LDY", LDY, ZP0, _ZP0, 3, 2, RD },{ L"LDA", LDA, ZP0, _ZP0, 3, 2, RD },{ L"LDX", LDX, ZP0, _ZP0, 3, 2, RD },{ L"LAX", LAX, ZP0, _ZP0, 3, 2, RD },{ L"TAY", TAY, IMP, _IMP, 2, 1, NN },{ L"LDA", LDA, IME, _IME, 2, 2, RD },{ L"TAX", TAX, IMP, _IMP, 2, 1, NN },{ L"LAX", LAX, IME, _IME, 2, 2, RD },{ L"LDY", LDY, ABS, _ABS, 4, 3, RD },{ L"LDA", LDA, ABS, _ABS, 4, 3, RD },{ L"LDX", LDX, ABS, _ABS, 4, 3, RD },{ L"LAX", LAX, ABS, _ABS, 4, 3, RD },
	/* B */ { L"BCS", BCS, REL, _REL, 2, 2, NN }, { L"LDA", LDA, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"LAX", LAX, IDY, _IDY, 5, 2, RD },{ L"LDY", LDY, ZPX, _ZPX, 4, 2, RD },{ L"LDA", LDA, ZPX, _ZPX, 4, 2, RD },{ L"LDX", LDX, ZPY, _ZPY, 4, 2, RD },{ L"LAX", LAX, ZPY, _ZPY, 4, 2, RD },{ L"CLV", CLV, IMP, _IMP, 2, 1, NN },{ L"LDA", LDA, ABY, _ABY, 4, 3, RD },{ L"TSX", TSX, IMP, _IMP, 2, 1, NN },{ L"LAS", LAS, ABY, _ABY, 4, 3, RD },{ L"LDY", LDY, ABX, _ABX, 4, 3, RD },{ L"LDA", LDA, ABX, _ABX, 4, 3, RD },{ L"LDX", LDX, ABY, _ABY, 4, 3, RD },{ L"LAX", LAX, ABY, _ABY, 4, 3, RD },
	/* C */ { L"CPY", CPY, IME, _IME, 2, 2, RD }, { L"CMP", CMP, IDX, _IDX, 6, 2, RD },{ L"NOP", NOP, IME, _IME, 2, 2, NN },{ L"DCP", DCP, IDX, _IDX, 8, 2, RW },{ L"CPY", CPY, ZP0, _ZP0, 3, 2, RD },{ L"CMP", CMP, ZP0, _ZP0, 3, 2, RD },{ L"DEC", DEC, ZP0, _ZP0, 5, 2, RW },{ L"DCP", DCP, ZP0, _ZP0, 5, 2, RW },{ L"INY", INY, IMP, _IMP, 2, 1, NN },{ L"CMP", CMP, IME, _IME, 2, 2, RD },{ L"DEX", DEX, IMP, _IMP, 2, 1, NN },{ L"SBX", SBX, IME, _IME, 2, 2, RD },{ L"CPY", CPY, ABS, _ABS, 4, 3, RD },{ L"CMP", CMP, ABS, _ABS, 4, 3, RD },{ L"DEC", DEC, ABS, _ABS, 6, 3, RW },{ L"DCP", DCP, ABS, _ABS, 6, 3, RW },
	/* D */ { L"BNE", BNE, REL, _REL, 2, 2, NN }, { L"CMP", CMP, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"DCP", DCP, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"CMP", CMP, ZPX, _ZPX, 4, 2, RD },{ L"DEC", DEC, ZPX, _ZPX, 6, 2, RW },{ L"DCP", DCP, ZPX, _ZPX, 6, 2, RW },{ L"CLD", CLD, IMP, _IMP, 2, 1, NN },{ L"CMP", CMP, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"DCP", DCP, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"CMP", CMP, ABX, _ABX, 4, 3, RD },{ L"DEC", DEC, ABX, _ABX, 7, 3, RW },{ L"DCP", DCP, ABX, _ABX, 7, 3, RW },
	/* E */ { L"CPX", CPX, IME, _IME, 2, 2, RD }, { L"SBC", SBC, IDX, _IDX, 6, 2, RD },{ L"NOP", NOP, IME, _IME, 2, 2, NN },{ L"ISC", ISC, IDX, _IDX, 8, 2, RW },{ L"CPX", CPX, ZP0, _ZP0, 3, 2, RD },{ L"SBC", SBC, ZP0, _ZP0, 3, 2, RD },{ L"INC", INC, ZP0, _ZP0, 5, 2, RW },{ L"ISC", ISC, ZP0, _ZP0, 5, 2, RW },{ L"INX", INX, IMP, _IMP, 2, 1, NN },{ L"SBC", SBC, IME, _IME, 2, 2, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"SBC", SBC, IME, _IME, 2, 2, RD },{ L"CPX", CPX, ABS, _ABS, 4, 3, RD },{ L"SBC", SBC, ABS, _ABS, 4, 3, RD },{ L"INC", INC, ABS, _ABS, 6, 3, RW },{ L"ISC", ISC, ABS, _ABS, 6, 3, RW },
	/* F */ { L"BEQ", BEQ, REL, _REL, 2, 2, NN }, { L"SBC", SBC, IDY, _IDY, 5, 2, RD },{ L"JAM", JAM, IMP, _IMP, 0, 1, NN },{ L"ISC", ISC, IDY, _IDY, 8, 2, RW },{ L"NOP", NOP, ZPX, _ZPX, 4, 2, NN },{ L"SBC", SBC, ZPX, _ZPX, 4, 2, RD },{ L"INC", INC, ZPX, _ZPX, 6, 2, RW },{ L"ISC", ISC, ZPX, _ZPX, 6, 2, RW },{ L"SED", SED, IMP, _IMP, 2, 1, NN },{ L"SBC", SBC, ABY, _ABY, 4, 3, RD },{ L"NOP", NOP, IMP, _IMP, 2, 1, NN },{ L"ISC", ISC, ABY, _ABY, 7, 3, RW },{ L"NOP", NOP, ABX, _ABX, 4, 3, NN },{ L"SBC", SBC, ABX, _ABX, 4, 3, RD },{ L"INC", INC, ABX, _ABX, 7, 3, RW },{ L"ISC", ISC, ABX, _ABX, 7, 3, RW },
};



static bool CPU_CheckNextInstruction(CPU *cpu){

    if(cpu->irq_executing || cpu->nmi_executing) return false;

    DebuggerBreakpoints *debugger_breakpoints = &cpu->nes->emulator->debugger->breakpoints;

    Range address_range;
    address_range.start = address_range.end = cpu->program_counter;

    if(BreakpointsData_FindCondition(debugger_breakpoints,MEMORY_CPU,BREAK_EXECUTE,address_range)){
        return true;
    }
    
    CPU_Instruction instruction = cpu->instruction_table[Debug_CPU_Read8(cpu->nes,cpu->program_counter)];
    
    if(instruction.check_io(cpu)) return true;

    return false;
}


void CPU_IRQ(CPU *cpu,uint8_t source){
    cpu->irq_source |= source;
}

void CPU_ClearIRQ(CPU *cpu,uint8_t source){
    cpu->irq_source &= ~source;
    if(!cpu->irq_source) cpu->irq_need = false;
}


void CPU_NMI(CPU *cpu,uint32_t delay){
    cpu->nmi_asserted = true;
    cpu->nmi_delay = delay;
    cpu->request_nmi_cycle = cpu->total_cycles;
}

void CPU_ClearNMI(CPU *cpu){
    cpu->nmi_asserted = false;
}


void CPU_Reset(CPU *cpu,uint8_t type){

    if(type == HARDWARE_RESET){

        cpu->total_cycles = 0x00;

        cpu->bus = 0x00;

        memset(cpu->ram,0,sizeof(uint8_t) * 0x2000);

        cpu->status_register = 0x20;

        cpu->accumulator = 0x00;
        cpu->x_register = 0x00;
        cpu->y_register = 0x00;
        cpu->stack_pointer = 0x00;

        cpu->opcode = 0;
        cpu->address = 0;
        cpu->cycles = 0;
        cpu->operation = NULL;
        
        cpu->request_nmi_cycle = 0x00;

        cpu->irq_source = 0;
        cpu->irq_need = false;
        cpu->irq_delay = false;
        cpu->irq_executing = false;

        cpu->rst_asserted = false;
        cpu->nmi_asserted = false;
        
        cpu->rst_executing = true;
        cpu->nmi_executing = false;

        cpu->oam_dma_byte = 0;
        cpu->oam_dma_start_address = 0;
        cpu->oam_dma_count = 0;
        cpu->oam_dma_delay = 0;
    }
    else if(type == SOFTWARE_RESET){

        cpu->rst_asserted = true;
    }
}


void CPU_OAMDMA(CPU *cpu,uint16_t start_address){
    cpu->oam_dma_start_address = start_address;
    cpu->oam_dma_count = 512;
    cpu->oam_dma_delay = 1;
}


static void CPU_FirstCircle(CPU *cpu){

    if(cpu->rst_executing){
        cpu->opcode = 0;
        cpu->cycles = 7;
        cpu->address = 0;
        cpu->operation = RST;
    }
    else if(cpu->nmi_executing){
        cpu->opcode = 0;
        cpu->cycles = 7;
        cpu->address = 0;
        cpu->operation = NMI;
    }
    else if(cpu->irq_executing){
        cpu->opcode = 0;
        cpu->cycles = 7;
        cpu->address = 0;
        cpu->operation = IRQ;
    }
    else{
        cpu->opcode = READ_8_BITS(cpu,cpu->program_counter);
        cpu->cycles = cpu->instruction_table[cpu->opcode].cycles;
        cpu->address = cpu->instruction_table[cpu->opcode].address(cpu);
        cpu->operation = cpu->instruction_table[cpu->opcode].operation;
    }
}

static void CPU_LastCircle(CPU *cpu){
    
    cpu->operation(cpu);

    if(cpu->rst_asserted){
        cpu->rst_executing = true;
        cpu->rst_asserted = false;
    }
    else if(cpu->nmi_asserted && !cpu->nmi_delay){
        cpu->nmi_executing = true;
        cpu->nmi_asserted = false;
    }
    else if((cpu->irq_need || ((cpu->irq_source & IRQ_MASK) && !(cpu->status_register & CPU_FLAG_I))) && !cpu->irq_delay){
        cpu->irq_executing = true;
        cpu->irq_need = false;
    }

    if(cpu->irq_delay){
        cpu->irq_delay = false;
        cpu->irq_need = true;
    }
    
    cpu->nes->breakpoint_found = CPU_CheckNextInstruction(cpu);
}


void CPU_Init(CPU *cpu,NES *nes){
    cpu->nes = nes;
    cpu->instruction_table = instruction_table;
}

void CPU_Execute(CPU *cpu,uint32_t cycles){

    for(uint32_t i=0; i<cycles; ++i){
        
        if(cpu->oam_dma_count){
            //Delay 512 + 1
            if(cpu->oam_dma_delay){
                READ_8_BITS(cpu,cpu->oam_dma_start_address);
                --cpu->oam_dma_delay;
            }
            //Alignment
            else if(!(cpu->oam_dma_count & 0x01) && !(cpu->total_cycles & 0x01)){
                READ_8_BITS(cpu,cpu->oam_dma_start_address);
            }
            else{
                if(cpu->total_cycles & 0x01){
                    cpu->oam_dma_byte = READ_8_BITS(cpu,cpu->oam_dma_start_address++);
                }
                else{
                    PPU_WriteOAMData(&cpu->nes->ppu,cpu->oam_dma_byte);
                }

                --cpu->oam_dma_count;
            }
        }
        else{
            if(cpu->cycles == 0){
                CPU_FirstCircle(cpu);
            }
            else if(cpu->cycles == 1){
                CPU_LastCircle(cpu);
            }
            
            cpu->cycles--;
        }

        if(cpu->nmi_delay) --cpu->nmi_delay;

        cpu->total_cycles++;
    }
}