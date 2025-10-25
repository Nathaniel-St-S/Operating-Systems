#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"
#include "../include/interrupts.h"

//CPU to control execution
Cpu THE_CPU;

/* --- core ---------------------------------------------------------------- */

void init_cpu(Cpu* cpu)
{
  cpu->registers[PC]  = MEM_START;
  cpu->registers[IR]  = EMPTY_REG;
  cpu->registers[FLAG] = F_ZERO;
  cpu->registers[ACC] = 0;
  // init_flags(&cpu->flags);
  printf("Initialized the cpu!\n");
  cpu_print_state();
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  THE_CPU.registers[IR] = read_mem(THE_CPU.registers[PC]);
  THE_CPU.registers[PC]++;
}

word decode(word instruction)
{
  word op = instruction >> 12;
  return op;
}
// Executes the instruction in the given cpu's IR with the given RAM
void execute() {
  word instruction = THE_CPU.registers[IR];
  //Decoded d = decode(instruction);
  //OP opcode = d.op;
  //mem_addr operand = d.addr;
  word op = decode(instruction);

  execute_instruction(op, instruction);
}

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(const int program_size, word* mem) {
  (void)mem; // unused here

  int i = 0;

  start:
    if (!(i < program_size && THE_CPU.registers[PC] != CPU_HALT))
      goto end;

    printf("=== Cycle %d ===\n", i + 1);

    if (THE_CPU.registers[PC] == CPU_HALT) {
      printf("CPU Halted!\n");
      goto end;
    }

    fetch();
    execute();
    check_for_interrupt();

    cpu_print_state();

    i++;
    goto start;

  end:
    ;
}
// Prints the state of the given CPU
void cpu_print_state() {
  printf("CPU STATE\n");
  printf("PC:  %X\n", THE_CPU.registers[PC]);
  printf("ACC: %X\n", THE_CPU.registers[ACC]);
  printf("IR:  %X\n", THE_CPU.registers[IR]);
  printf("FLAGS:\n");
  printf("  ZERO:      %1d\n", (THE_CPU.registers[FLAG] >> 0) & 1);
  printf("  OVERFLOW:  %1d\n", (THE_CPU.registers[FLAG] >> 1) & 1);
  printf("  CARRY:     %1d\n\n\n", (THE_CPU.registers[FLAG] >> 2) & 1);
  //printf("  INTERRUPT: %1d\n\n\n", THE_CPU.registers[FLAG] >> 0) & 1);
}
