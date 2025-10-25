#include "../include/isa.h"
#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/types.h"
#include <stdint.h>

/*-----------------------------Flag Setters-----------------------------*/

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
void set_nzp_flag(word value) {
  CLEAR_ALL_FLAGS;

    int16_t svalue = (int16_t)value;

    if (svalue == 0){
      SET_FLAG(F_ZERO);
    }
    else if (svalue < 0){
      SET_FLAG(F_NEG);
    }
    else
    {SET_FLAG(F_POS);
    }
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
void set_add_flags(word a, word b, word r) {
  CLEAR_ALL_FLAGS;

  // Unsigned carry out of bit 15
  if ((uint32_t)a + (uint32_t)b > 0xFFFF){
    SET_FLAG(F_CARRY);
  }
  // Signed overflow: inputs same sign, result different sign
  sword sa = (sword)a;
  sword sb = (sword)b;
  sword sr = (sword)r;
  if((sa > 0 && sb > 0 && sr < 0) ||
      (sa < 0 && sb < 0 && sr > 0)){
    SET_FLAG(F_OVFLW);
  }
  
  set_nzp_flag(r);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
void set_sub_flags(word a, word b, word r) {
  CLEAR_ALL_FLAGS;

  // Borrow in unsigned
  if((word)a < (word)b)
  {
    SET_FLAG(F_CARRY);
  }

  // Signed overflow: inputs different sign, result sign differs from a
  sword sa = (sword)a;
  sword sb = (sword)b;
  sword sr = (sword)r;
  if((sa >= 0 && sb <  0 && sr <  0) ||
       (sa <  0 && sb >= 0 && sr >= 0))
  {
    SET_FLAG(F_OVFLW);
  }

  set_nzp_flag(r);
}

// Set the zero, carry, and OF :P flags for multiplication
void set_mul_flags(word a, word b, word r)
{
  CLEAR_ALL_FLAGS;

  uint32_t p = (uint32_t)a * (uint32_t)b;
  if(p >> 16 != 0)
  {
    SET_FLAG(F_CARRY);
  }

  int32_t sp = (int16_t)a * (int16_t)b;
  if(sp > INT16_MAX || sp <INT16_MIN)
  {
    SET_FLAG(F_OVFLW);
  }

  set_nzp_flag(r);
}

// Set the flags for division
void set_div_flags(word a, word b, word r)
{
  CLEAR_ALL_FLAGS;

  CLEAR_FLAG(F_CARRY);

  if((sword)a == INT16_MIN && (sword)b == -1){SET_FLAG(F_OVFLW);}

  set_nzp_flag(r);
}

// Set the interrupt flag
//void set_interrupt_flag(bool enabled) {
//    THE_CPU.flags.INTERRUPT = enabled ? 1 : 0;
//}
/*-----------------------------Helpers-----------------------------*/

word sign_extend(word x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/*-----------------------------Codes / Instructions-----------------------------*/

// Adds the value in the cpu's ACC register with the given value
// The sum is stored in ACC and the appropriate flags are set
void add(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] + imm5;
    set_add_flags(THE_CPU.registers[sr1], imm5, THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] + THE_CPU.registers[sr2];
    set_add_flags(THE_CPU.registers[sr1], THE_CPU.registers[sr2], THE_CPU.registers[dr]);
  }

}

// Subtracts the value in the cpu's ACC register with the given value
// The difference is stored in ACC and the appropriate flags are set
void sub(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] - imm5;
    set_sub_flags(THE_CPU.registers[sr1], imm5, THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] - THE_CPU.registers[sr2];
    set_sub_flags(THE_CPU.registers[sr1], THE_CPU.registers[sr2], THE_CPU.registers[dr]);
  }
}

// Multiplies the value in the cpu's ACC register with the value at the given
// memory address. The product is stored in ACC and the appropriate flags are
// set.
void mul(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] * imm5;
    set_mul_flags(THE_CPU.registers[sr1], imm5, THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] * THE_CPU.registers[sr2];
    set_mul_flags(THE_CPU.registers[sr1], THE_CPU.registers[sr2], THE_CPU.registers[dr]);
  }
}

// Divides the value in the cpu's ACC register with the value at the given
// memory address. The quotient is stored in the ACC and the appropriate flags
// are set.
void divide(const word instruction) {
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    if(!imm5)
    {
      printf("Divide by Zero Error");
      THE_CPU.registers[PC] = CPU_HALT;
      return;
    }
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] / imm5;
    set_div_flags(THE_CPU.registers[sr1], imm5, THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    if(!THE_CPU.registers[sr2])
    {
      printf("Divide by Zero Error");
      THE_CPU.registers[PC] = CPU_HALT;
      return;
    }

    THE_CPU.registers[dr] = THE_CPU.registers[sr1] / THE_CPU.registers[sr2];
    set_div_flags(THE_CPU.registers[sr1], THE_CPU.registers[sr2], THE_CPU.registers[dr]);
  }
}

// Performs bitwise and on the cpu's ACC rgister and the value at the given
// memory address.
void bit_and(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] & imm5;
    set_nzp_flag(THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] & THE_CPU.registers[sr2];
    set_nzp_flag(THE_CPU.registers[dr]);
  } 
}

// Performs bitwise or on the cpu's ACC rgister and the value at the given
// memory address.
void bit_or(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Firsr operand (source register one)
  word sr1 = (instruction >> 6) & 0x7;

  // Immediate mode flag
  word imm_flag = (instruction >> 5) & 0x1;

  if(imm_flag)
  {
    word imm5 = sign_extend(instruction & 0x1F, 5);
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] | imm5;
    set_nzp_flag(THE_CPU.registers[dr]);
  }
  else
  {
    // Not in immediate mode so we must get our value from a second source register
    word sr2 = instruction & 0x7;
    THE_CPU.registers[dr] = THE_CPU.registers[sr1] | THE_CPU.registers[sr2];
    set_nzp_flag(THE_CPU.registers[dr]);
  } 
}

// Performs bitwise not on the source register and loads it into destination register
// sets zero flag accordingly
void bit_not(const word instruction)
{
  // Destination register
  word dr = (instruction >> 9) & 0x7;

  // Source register
  word sr1 = (instruction >> 6) & 0x7;

  THE_CPU.registers[dr] = ~THE_CPU.registers[sr1];

  set_nzp_flag(THE_CPU.registers[dr]);
}

// Conditionally branch the CPU depending on the status of the zero, overflow
// and carry flags. if no flags are set, no branching will happen
void branch(const word instruction)
{
  word offset = sign_extend(instruction & 0x1FF, 9);
  word cond = (instruction >> 9) & 0x7;

  // Bit 11 is the zero flag
  if (cond & THE_CPU.registers[FLAG])
  {
    THE_CPU.registers[PC] += offset;
  }
}

// Begins execution at the given memory address by setting the cpu's program
// counter.
void jump(const word instruction)
{
  word dest = (instruction >> 6) & 0x7;
  THE_CPU.registers[PC] = THE_CPU.registers[dest];
}

// Save the incremented PC into EX, then jump to first instruction of the
// subroutine. the location of the subroutine is stored in the base register
// or calculated by sign extending the offset
 void jump_register(const word instruction)
{
  //save the pc in EX
  THE_CPU.registers[EX] = THE_CPU.registers[PC];

  // bit 11 will signify wether the location is in a register
  word flag = (instruction >> 11) & 0x1;

  if(flag)
  {
    word offset = sign_extend(instruction & 0x7FF, 11);
    THE_CPU.registers[PC] += offset;
  }
  else
  {
    word br = (instruction >> 6) & 0x7;
    THE_CPU.registers[PC] = THE_CPU.registers[br];
  }
}

// Begins execution at the given memory address if the zero flag is set.
void jump_zero(const word instruction)
{
  if((THE_CPU.registers[FLAG] >> 0) & 1)
  {
    word flag = (instruction >> 11) & 0x1;
    if(flag)
    {
      word offset = sign_extend(instruction & 0x7FF, 11);
      THE_CPU.registers[PC] += offset;
    }
    else
    {
      word br = (instruction >> 6) & 0x7;
      THE_CPU.registers[PC] = THE_CPU.registers[br]; 
    }
  }
}

// Stores the data in cpu's ACC register at the given memory address
void store(const word instruction)
{
  word sr = (instruction >> 9) & 0x7;
  word offset = sign_extend(instruction & 0x1FF, 9);
  write_mem(THE_CPU.registers[PC] + offset, THE_CPU.registers[sr]);
}

// The contents of the register specified by SR are stored in the memory location
// whose address is computed by sign-extending bits [5:0] to 16 bits and adding this
// value to the contents of the register specified by bits [8:6]
void store_register(const word instruction)
{
  word sr = (instruction >> 9) & 0x7;
  word br = (instruction >> 6) & 0x7;
  word offset = sign_extend(instruction & 0x3F, 6);

  write_mem(THE_CPU.registers[br] + offset, THE_CPU.registers[sr]);
}

// The contents of the register specified by SR are stored in the memory location
// whose address is obtained as follows: Bits [8:0] are sign-extended to 16 bits and
// added to the incremented PC. What is in memory at this address is the address of
// the location to which the data in SR is stored.
void store_indirect(const word instruction)
{
  word sr = (instruction >> 9) & 0x7;
  word offset = sign_extend(instruction & 0x1FF, 9);
  write_mem(read_mem(THE_CPU.registers[PC] + offset), THE_CPU.registers[sr]);
}

// Loads the data at the given memory address into cpu's ACC register
void load(const word instruction)
{
  word dr = (instruction >> 9) & 0x7;
  word offset = sign_extend(instruction & 0x1FF, 9);

  THE_CPU.registers[dr] = read_mem(THE_CPU.registers[PC] + offset);
  set_nzp_flag(THE_CPU.registers[dr]);
}

// An address is computed by sign-extending bits [8:0] to 16 bits and adding this
// value to the incremented PC. This address is loaded into DR. The condition
// codes are set, based on whether the value loaded is negative, zero, or positive.
void load_effective_address(const word instruction)
{
  word dr = (instruction >> 9) & 0x7;
  word offset= sign_extend(instruction & 0x1FF, 9);

  THE_CPU.registers[dr] = THE_CPU.registers[PC] + offset;
  set_nzp_flag(THE_CPU.registers[dr]);
}

// An address is computed by sign-extending bits [5:0] to 16 bits and adding this
// value to the contents of the register specified by bits [8:6]. The contents of memory
// at this address are loaded into DR. The condition codes are set, based on whether
// the value loaded is negative, zero, or positive
void load_register(const word instruction)
{
  word dr = (instruction >> 9) & 0x7;
  word br = (instruction >> 6) & 0x7;
  word offset = sign_extend(instruction & 0x3F, 6);

  THE_CPU.registers[dr] = read_mem(THE_CPU.registers[br] + offset);
  set_nzp_flag(THE_CPU.registers[dr]); 
}

// An address is computed by sign-extending bits [8:0] to 16 bits and adding this
// value to the incremented PC. What is stored in memory at this address is the
// address of the data to be loaded into DR. The condition codes are set, based on
// whether the value loaded is negative, zero, or positive
void load_indirect(const word instruction)
{
  word dr = (instruction >> 9) & 0x7;
  word offset = sign_extend(instruction & 0x1FF, 9);

  THE_CPU.registers[dr] = read_mem(read_mem(THE_CPU.registers[PC] + offset));

  set_nzp_flag(THE_CPU.registers[dr]);
}

// get a single ascii charachter from BX
void get_char()
{
  THE_CPU.registers[BX] = (word)getchar();
}

// flush a single characher to stdout
void out()
{
  putc((char)THE_CPU.registers[BX], stdout);
  fflush(stdout);
}

// grab a single charachter from stdin
void in()
{
  printf("Enter a charachter: ");
  char c = getchar();
  putc(c, stdout);
  fflush(stdout);
  THE_CPU.registers[BX] = (word)c;
}

// put an entire null terminated string into stdout
void put_str()
{
  //one char per word
  uint16_t* c = RAM + THE_CPU.registers[BX];
  while (*c)
  {
      putc((char)*c, stdout);
      ++c;
  }
  fflush(stdout);
}

// Put up to 2 8 bit charachters into stdout
void put_sp()
{
  //one char per byte (two bytes per word)
  uint16_t* c = RAM + THE_CPU.registers[BX];
  while (*c)
  {
      char char1 = (*c) & 0xFF;
      putc(char1, stdout);
      char char2 = (*c) >> 8;
      if (char2) putc(char2, stdout);
      ++c;
  }
  fflush(stdout);
}

// END execution of the cpu
void halt()
{
  puts("HALT");
  fflush(stdout);
  THE_CPU.registers[PC] = CPU_HALT;
}

void report_invalid_instruction(const word instruction)
{
printf("ERROR: Invalid instruction %u \n", (unsigned)instruction);
   THE_CPU.registers[PC] = CPU_HALT;
}

// Reports an error for invalid opcodes and halts the THE_CPU.
static inline void report_invalid_opcode(const word opcode, const word instruction) {
  printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode,
          (unsigned)instruction);
   THE_CPU.registers[PC] = CPU_HALT;
}

// Initiates and handles CPU interrupts
static inline void interrupt(const word instruction)
{
  // Save the register for restoration later
  THE_CPU.registers[AX] = THE_CPU.registers[PC];

  switch(instruction & 0xFF)
  {
    case INT_GETC: get_char(); break;
    case INT_OUT: out(); break;
    case INT_IN: in(); break;
    case INT_PUTS: put_str(); break;
    case INT_PUTSP: put_sp(); break;
    case INT_HALT: halt(); break;
    default: report_invalid_instruction(instruction);
  }

  // go back to the og instruction
  THE_CPU.registers[PC] = THE_CPU.registers[AX];
}

/*------------------------------------External Functions / API------------------------------------*/

// execute the instruction based off of the opcode
void execute_instruction(const word op, const word instruction) {
  switch (op) {
    case ADD: add(instruction); break;
    case SUB: sub(instruction); break;
    case MUL: mul(instruction); break;
    case DIV: divide(instruction); break;
    case AND: bit_and(instruction); break;
    case OR: bit_or(instruction); break;
    case NOT: bit_not(instruction); break;
    case BRANCH: branch(instruction); break;
    case JUMP: jump(instruction); break;
    case JUMPR: jump_register(instruction); break;
    case JUMPZ: jump_zero(instruction); break;
    case STORE: store(instruction); break;
    case STRR: store_register(instruction); break;
    case STRI: store_indirect(instruction); break;
    case LOAD: load(instruction); break;
    case LEA: load_effective_address(instruction); break;
    case LDR: load_register(instruction); break;
    case LDI: load_indirect(instruction); break;
    case INTR: interrupt(instruction); break;
    //case ENDINT: end_interrupt(instruction); break;
    //case HALT: halt(); break;
    default: report_invalid_opcode(op, instruction);
  }
}
