#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"
#include "../include/interrupts.h"

// Decodes the given instruction into its operator and operand
static dword decode(dword instruction);
// Prints the state of the given CPU
static void cpu_print_state(void);

//CPU to control execution
Cpu THE_CPU;

/* --- core ---------------------------------------------------------------- */

void init_cpu()
{
  REGISTER(PC) = MEM_START;
  REGISTER(IR) = EMPTY_REG;
  REGISTER(FLAG) = F_ZERO;
  REGISTER(ACC) = 0;
  // init_flags(&cpu->flags);
  printf("Initialized the cpu!\n");
  cpu_print_state();
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  REGISTER(IR) = read_mem(REGISTER(PC));
  REGISTER(PC)++;
}

static dword decode(dword instruction)
{
  dword op = (instruction >> OPCODE_SHIFT);
  return op;
}

// Executes the instruction in the given cpu's IR with the given RAM
void execute() {
  dword instruction = REGISTER(IR);
  dword op = decode(instruction);

  execute_instruction(op, instruction);
}

void cpu_run()
{
  int i = 0;
  while(REGISTER(PC) != CPU_HALT)
  {
    printf("=== Cycle %d ===\n", i + 1);
    fetch();
    execute();
    check_for_interrupt();
    cpu_print_state();
    i++;
  }
}
// Prints the state of the given CPU
static void cpu_print_state() {
  printf("CPU STATE\n");
  printf("  AX: 0x%X\n", THE_CPU.registers[AX]);
  printf("  BX: 0x%X\n", THE_CPU.registers[BX]);
  printf("  CX: 0x%X\n", THE_CPU.registers[CX]);
  printf("  DX: 0x%X\n", THE_CPU.registers[DX]);
  printf("PC:  %X\n", THE_CPU.registers[PC]);
  printf("ACC: %X\n", THE_CPU.registers[ACC]);
  printf("IR:  %X\n", THE_CPU.registers[IR]);
  printf("FLAGS:\n");
  printf("  ZERO:      %1d\n", (THE_CPU.registers[FLAG] >> 0) & 1);
  printf("  OVERFLOW:  %1d\n", (THE_CPU.registers[FLAG] >> 1) & 1);
  printf("  CARRY:     %1d\n\n\n", (THE_CPU.registers[FLAG] >> 2) & 1);
  //printf("  INTERRUPT: %1d\n\n\n", THE_CPU.registers[FLAG] >> 0) & 1);
}
