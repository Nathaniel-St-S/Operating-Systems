#ifndef CPU_H
#define CPU_H

#include "types.h"

typedef enum GP_Register {
  REG_ZERO, // Constant zero, reads as zero, writes ignored
  REG_AT,   // Assembler Temporary
  REG_VO,   // Function Return Values
  REG_V1,
  REG_A0,   // Function Argument
  REG_A1,
  REG_A2,
  REG_A3,
  REG_T0,   // Temporaries (caller-saved)
  REG_T1,
  REG_T2,
  REG_T3,
  REG_T4,
  REG_T5,
  REG_T6,
  REG_T7,
  REG_S0,   // Saved registers (callee-saved)
  REG_S1,
  REG_S2,
  REG_S3,
  REG_S4,
  REG_S5,
  REG_S6,
  REG_S7,
  REG_T8,   // Temporaries
  REG_T9,
  REG_K0,   // Reserved for kernel / OS use
  REG_K1,
  REG_GP,   // Global pointer
  REG_SP,   // Stack pointer
  REG_FP,   // Frame pointer
  REG_RA,   // Return address for calls
  GP_REG_COUNT,
} GP_Register;

typedef enum HW_Register {
  PC,    // Program Counter
  IR,    // Instruction Register
  MAR,   // Memory Address Register
  MBR,   // Memory Buffer Register
  IO_AR, // IO Address Register
  IO_BR, // IO Buffer Register
  FLAGS, // The CPU Flags
  HI,    // Holds the high 32 bits of a multiplication or division
  LO,    // Holds the low 32 bits of a multiplication or division
  HW_REG_COUNT,
} HW_Register;

typedef enum FLAG {
  F_ZERO     = 1 << 0,
  F_OVERFLOW = 1 << 1,
  F_CARRY    = 1 << 2,
} FLAG;

typedef struct Cpu { 
  word gp_registers[GP_REG_COUNT]; 
  word hw_registers[HW_REG_COUNT];
} Cpu;

extern Cpu THE_CPU;


#endif // !CPU_H
