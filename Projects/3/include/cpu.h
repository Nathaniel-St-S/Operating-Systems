#ifndef CPU_H
#define CPU_H

#include "isa.h"

#define EMPTY_REG ((dword) - 1)
#define UNSET_FLAG 0
#define MEM_START 0

#define REGISTER(register) (THE_CPU.registers[register])

// Interupt codes
enum
{
  INT_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
  INT_OUT = 0x21,   /* output a character */
  INT_PUTS = 0x22,  /* output a word string */
  INT_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
  INT_PUTSP = 0x24, /* output a byte string */
  INT_HALT = 0x25   /* halt the program */
};

// CPU flags
enum
{
  F_ZERO  = 1 << 0, /* zero flag */
  F_OVFLW = 1 << 1, /* overflow flag */
  F_CARRY = 1 << 2, /* carry flag */
  //F_POS   = 1 << 3, /* positive flag */
  //F_NEG   = 1 << 4  /* negative flag */
};

// Registers
enum
{
  AX = 0,
  BX,
  CX,
  DX,
  PC,
  IR,
  ACC,
  FLAG,
  COUNT // Not an actual register, here to keep count of how many registers there are.
};

typedef struct Cpu { dword registers[COUNT]; } Cpu;

//CPU to control execution
extern Cpu THE_CPU;

//initialize a CPU to fetch, decode, and execute instructions
void init_cpu(void);

// Fetch the next instruction from the given memory and cpu
// and increments the program counter
void fetch(void);

// Executes the instruction in the given cpu's IR with the given RAM
void execute(void);

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(void);

#endif
