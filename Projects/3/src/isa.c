#include "../include/isa.h"
#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/types.h"
#include <stdint.h>

/*-----------------------------Flag Setters-----------------------------*/

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
void set_zero_flag(dword value)
{
  CLEAR_FLAG(F_ZERO);

  if(value == 0){SET_FLAG(F_ZERO); return;}
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
void set_add_flags(dword a, dword b, dword r) {
  CLEAR_ALL_FLAGS;

  // Unsigned carry out of bit 15
  if ((uint64_t)a + (uint64_t)b > 0xFFFFFFFF){
    SET_FLAG(F_CARRY);
  }
  // Signed overflow: inputs same sign, result different sign
  sdword sa = (sdword)a;
  sdword sb = (sdword)b;
  sdword sr = (sdword)r;
  if((sa > 0 && sb > 0 && sr < 0) ||
      (sa < 0 && sb < 0 && sr > 0)){
    SET_FLAG(F_OVFLW);
  }
  
  set_zero_flag(r);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
void set_sub_flags(dword a, dword b, dword r) {
  CLEAR_ALL_FLAGS;

  // Borrow in unsigned
  if((dword)a < (dword)b)
  {
    SET_FLAG(F_CARRY);
  }

  // Signed overflow: inputs different sign, result sign differs from a
  sdword sa = (sdword)a;
  sdword sb = (sdword)b;
  sdword sr = (sdword)r;
  if((sa >= 0 && sb <  0 && sr <  0) ||
       (sa <  0 && sb >= 0 && sr >= 0))
  {
    SET_FLAG(F_OVFLW);
  }

  set_zero_flag(r);
}

// Set the zero, carry, and OF :P flags for multiplication
void set_mul_flags(dword a, dword b, dword r)
{
  CLEAR_ALL_FLAGS;

  uint64_t p = (uint64_t)a * (uint64_t)b;
  if(p >> 32 != 0)
  {
    SET_FLAG(F_CARRY);
  }

  int64_t sp = (sdword)a * (sdword)b;
  if(sp > INT32_MAX || sp <INT32_MIN)
  {
    SET_FLAG(F_OVFLW);
  }

  set_zero_flag(r);
}

// Set the flags for division
void set_div_flags(dword a, dword b, dword r)
{
  CLEAR_ALL_FLAGS;

  CLEAR_FLAG(F_CARRY);

  if((sdword)a == INT32_MIN && (sdword)b == -1){SET_FLAG(F_OVFLW);}

  set_zero_flag(r);
}

/*-----------------------------Helpers-----------------------------*/

dword sign_extend(dword x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFFFFFF << bit_count);
    }
    return x;
}

#define ARITHMETIC_INSTRUCTION(instruction, operand, flag_setter, is_dividing) ({\
    /* Destination register */\
    dword dr = (instruction >> DR_SHIFT) & 0xF;\
\
    /* Firsr operand (source register one) */\
    dword sr1 = (instruction >> SR1_SHIFT) & 0xF;\
\
    /* Immediate mode flag */\
    dword imm_flag = (instruction >> MODE_SHIFT) & 0x1;\
\
    if(imm_flag)\
    {\
      sdword imm = sign_extend(instruction & OPERAND_MASK, 12);\
      if(is_dividing && !imm)\
      {\
        printf("Divide by Zero Error");\
        REGISTER(PC) = CPU_HALT;\
        return;\
      }\
      REGISTER(dr)= REGISTER(sr1) operand imm;\
      flag_setter(REGISTER(sr1), imm, REGISTER(dr));\
    }\
    else\
    {\
      /* Not in immediate mode so we must get our value from a second source register */\
      dword sr2 = instruction & 0xF;\
      if(is_dividing && !REGISTER(sr2))\
      {\
        printf("Divide by Zero Error");\
        REGISTER(PC)= CPU_HALT;\
        return;\
      }\
      REGISTER(dr) = REGISTER(sr1) operand REGISTER(sr2);\
      flag_setter(REGISTER(sr1), REGISTER(sr2), REGISTER(dr));\
    }\
})

#define BITWISE_INSTRUCTION(instruction, operand) ({\
    /* Destination register */\
    dword dr = (instruction >> DR_SHIFT) & 0xF;\
\
    /* First operand (source register one) */\
    dword sr1 = (instruction >> SR1_SHIFT) & 0xF;\
\
    /* Immediate mode flag */\
    dword imm_flag = (instruction >> MODE_SHIFT) & 0x1;\
\
    if(imm_flag)\
    {\
      sdword imm = sign_extend(instruction & OPERAND_MASK, 12);\
      REGISTER(dr) = REGISTER(sr1) operand imm;\
    }\
    else\
    {\
      /* Not in immediate mode so we must get our value from a second source register */\
      dword sr2 = instruction & 0xF;\
      REGISTER(dr) = REGISTER(sr1) operand REGISTER(sr2);\
    }\
    set_zero_flag(REGISTER(dr));\
})


/*-----------------------------Codes / Instructions-----------------------------*/

// Adds the value in the cpu's ACC register with the given value
// The sum is stored in ACC and the appropriate flags are set
void add(const dword instruction)
{
  ARITHMETIC_INSTRUCTION(instruction, +, set_add_flags, 0);
}

// Subtracts the value in the cpu's ACC register with the given value
// The difference is stored in ACC and the appropriate flags are set
void sub(const dword instruction)
{
  ARITHMETIC_INSTRUCTION(instruction, -, set_sub_flags, 0);
}

// Multiplies the value in the cpu's ACC register with the value at the given
// memory address. The product is stored in ACC and the appropriate flags are
// set.
void mul(const dword instruction)
{
  ARITHMETIC_INSTRUCTION(instruction, *, set_mul_flags, 0);
}

// Divides the value in the cpu's ACC register with the value at the given
// memory address. The quotient is stored in the ACC and the appropriate flags
// are set.
void divide(const dword instruction)
{
  ARITHMETIC_INSTRUCTION(instruction, /, set_div_flags, 1);
}

// Performs bitwise and on the cpu's ACC rgister and the value at the given
// memory address.
void bit_and(const dword instruction)
{
  BITWISE_INSTRUCTION(instruction, &);
}

// Performs bitwise or on the cpu's ACC rgister and the value at the given
// memory address.
void bit_or(const dword instruction)
{
  BITWISE_INSTRUCTION(instruction, |);
}

// Perform bitwise xor on the source register and loads it into destination register
// sets zero flag accordingly
void bit_xor(dword instruction)
{
  BITWISE_INSTRUCTION(instruction, ^);
}

// Performs bitwise not on the source register and loads it into destination register
// sets zero flag accordingly
void bit_not(const dword instruction)
{
  // Destination register
  dword dr  = (instruction >> DR_SHIFT) & 0xF;

  // Source register
  dword sr1 = (instruction >> SR1_SHIFT) & 0xF;

  REGISTER(dr) = ~REGISTER(sr1);

  set_zero_flag(REGISTER(dr));
}

// Conditionally branch the CPU depending on the status of the zero, overflow
// and carry flags. if no flags are set, no branching will happen
void branch(const dword instruction)
{
  sdword offset = sign_extend(instruction & 0x000FFFFF, 20);
  dword cond = (instruction >> DR_SHIFT) & 0x7;

  // Bit 11 is the zero flag
  if (cond & REGISTER(FLAG))
  {
    REGISTER(PC) += offset;
  }
}

// Begins execution at the given memory address by setting the cpu's program
// counter.
void jump(const dword instruction)
{
  dword dest = (instruction >> SR1_SHIFT) & 0xF;
  REGISTER(PC) = REGISTER(dest);
}

// Save the incremented PC into AX, then jump to first instruction of the
// subroutine. the location of the subroutine is stored in the base register
// or calculated by sign extending the offset
 void jump_register(const dword instruction)
{
  //save the pc in AX
  REGISTER(AX) = REGISTER(PC);

  // DR_SHIFT will hold the flag instead of a register
  dword flag = (instruction >> DR_SHIFT) & 0x1;

  if(flag)
  {
    sdword offset = sign_extend(instruction & REG_MASK, 16);
    REGISTER(PC) += offset;
  }
  else
  {
    dword br = (instruction >> SR1_SHIFT) & 0xF;
    REGISTER(PC) = REGISTER(br);
  }
}

// Begins execution at the given memory address if the zero flag is set.
void jump_zero(const dword instruction)
{
  if((REGISTER(FLAG) >> 0) & 1)
  {
    dword flag = (instruction >> DR_SHIFT) & 0x1;
    if(flag)
    {
      sdword offset = sign_extend(instruction & REG_MASK, 16);
      REGISTER(PC) += offset;
    }
    else
    {
      dword br = (instruction >> SR1_SHIFT) & 0xF;
      REGISTER(PC) = REGISTER(br); 
    }
  }
}

// Stores the data in cpu's ACC register at the given memory address
void store(const dword instruction)
{
  dword sr = (instruction >> DR_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & IMM_MASK, 20);
  write_mem(REGISTER(PC) + offset, REGISTER(sr));
}

// The contents of the register specified by SR are stored in the memory location
// whose address is computed by sign-extending bits [5:0] to 16 bits and adding this
// value to the contents of the register specified by bits [8:6]
void store_register(const dword instruction)
{
  dword sr = (instruction >> DR_SHIFT) & 0xF;
  dword br = (instruction >> SR1_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & REG_MASK, 16);

  write_mem(REGISTER(br) + offset, REGISTER(sr));
}

// The contents of the register specified by SR are stored in the memory location
// whose address is obtained as follows: Bits [8:0] are sign-extended to 16 bits and
// added to the incremented PC. What is in memory at this address is the address of
// the location to which the data in SR is stored.
void store_indirect(const dword instruction)
{
  dword sr = (instruction >> DR_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & IMM_MASK, 20);
  write_mem(read_mem(REGISTER(PC) + offset), REGISTER(sr));
}

// Loads the data at the given memory address into cpu's ACC register
void load(const dword instruction)
{
  dword dr = (instruction >> DR_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & IMM_MASK, 20);

  REGISTER(dr) = read_mem(REGISTER(PC) + offset);
  set_zero_flag(REGISTER(dr));
}

// An address is computed by sign-extending bits [8:0] to 16 bits and adding this
// value to the incremented PC. This address is loaded into DR. The condition
// codes are set, based on whether the value loaded is negative, zero, or positive.
void load_effective_address(const dword instruction)
{
  dword dr = (instruction >> DR_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & IMM_MASK, 20);

  REGISTER(dr) = REGISTER(PC) + offset;
  set_zero_flag(REGISTER(dr));
}

// An address is computed by sign-extending bits [5:0] to 16 bits and adding this
// value to the contents of the register specified by bits [8:6]. The contents of memory
// at this address are loaded into DR. The condition codes are set, based on whether
// the value loaded is negative, zero, or positive
void load_register(const dword instruction)
{
  dword dr = (instruction >> DR_SHIFT) & 0xF;
  dword br = (instruction >> SR1_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & REG_MASK, 16);

  REGISTER(dr) = read_mem(REGISTER(br) + offset);
  set_zero_flag(REGISTER(dr)); 
}

// An address is computed by sign-extending bits [8:0] to 16 bits and adding this
// value to the incremented PC. What is stored in memory at this address is the
// address of the data to be loaded into DR. The condition codes are set, based on
// whether the value loaded is negative, zero, or positive
void load_indirect(const dword instruction)
{
  dword dr = (instruction >> DR_SHIFT) & 0xF;
  sdword offset = sign_extend(instruction & IMM_MASK, 20);

  REGISTER(dr) = read_mem(read_mem(REGISTER(PC) + offset));

  set_zero_flag(REGISTER(dr));
}

// get a single ascii charachter from BX
void get_char()
{
  REGISTER(BX) = (dword)getchar();
}

// flush a single characher to stdout
void out()
{
  putc((char)REGISTER(BX), stdout);
  fflush(stdout);
}

// grab a single charachter from stdin
void in()
{
  printf("Enter a charachter: ");
  char c = getchar();
  putc(c, stdout);
  fflush(stdout);
  REGISTER(BX) = (dword)c;
}

// put an entire null terminated string into stdout
void put_str()
{
  //one char per dword
  dword* c = RAM + REGISTER(BX);
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
  //one char per byte (two bytes per dword)
  dword* c = RAM + REGISTER(BX);
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
  REGISTER(PC) = CPU_HALT;
}

void report_invalid_instruction(const dword instruction)
{
printf("ERROR: Invalid instruction %u \n", (unsigned)instruction);
   REGISTER(PC) = CPU_HALT;
}

// Reports an error for invalid opcodes and halts the THE_CPU.
static inline void report_invalid_opcode(const dword opcode, const dword instruction) {
  printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode,
          (unsigned)instruction);
   REGISTER(PC) = CPU_HALT;
}

// Initiates and handles CPU interrupts
static inline void interrupt(const dword instruction)
{
  // Save the register for restoration later
  REGISTER(AX) = REGISTER(PC);

  switch(instruction & 0xFFFFFF)
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
  REGISTER(PC) = REGISTER(AX) + 1;
}

/*------------------------------------External Functions / API------------------------------------*/

// execute the instruction based off of the opcode
void execute_instruction(const dword op, const dword instruction) {
  switch (op) {
    case ADD: add(instruction); break;
    case SUB: sub(instruction); break;
    case MUL: mul(instruction); break;
    case DIV: divide(instruction); break;
    case AND: bit_and(instruction); break;
    case OR: bit_or(instruction); break;
    case XOR: bit_xor(instruction); break;
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
