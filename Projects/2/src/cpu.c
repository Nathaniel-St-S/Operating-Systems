#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"
#include "../include/interrupts.h"

//CPU to control execution
Cpu CPU;

/* --- core ---------------------------------------------------------------- */

//initialize the flags of the given cpu
  void init_flags(Flags* flags) {
    flags->ZERO      = UNSET_FLAG;
    flags->CARRY     = UNSET_FLAG;
    flags->OVERFLOW  = UNSET_FLAG;
    flags->INTERRUPT = UNSET_FLAG;
  
  }

void init_cpu(Cpu* cpu)
{
  cpu->registers[PC]  = MEM_START;
  cpu->registers[IR]  = EMPTY_REG;
  cpu->registers[ACC] = 0;
  init_flags(&cpu->flags);
  printf("Initialized the cpu!\n");
  cpu_print_state();
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  CPU.registers[IR] = read_mem(CPU.registers[PC]);
  CPU.registers[PC]++;
}

/*
// Decodes the given instruction into its operator and operand
Decoded decode (word instruction) {
  Decoded d;
  d.op   = (OP)((instruction & 0xF000u) >> 12);
  d.addr = (mem_addr)(instruction & 0x0FFFu);
  return d;
}
*/
word decode(word instruction)
{
  word op = instruction >> 12;
  return op;
}
// Executes the instruction in the given cpu's IR with the given RAM
void execute() {
  word instruction = CPU.registers[IR];
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
    if (!(i < program_size && CPU.registers[PC] != CPU_HALT))
      goto end;

    printf("=== Cycle %d ===\n", i + 1);

    if (CPU.registers[PC] == CPU_HALT) {
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
  printf("PC:  %X\n", CPU.registers[PC]);
  printf("ACC: %X\n", CPU.registers[PC]);
  printf("IR:  %X\n", CPU.registers[IR]);
  printf("FLAGS:\n");
  printf("  ZERO:      %1d\n", CPU.flags.ZERO);
  printf("  CARRY:     %1d\n", CPU.flags.CARRY);
  printf("  OVERFLOW:  %1d\n", CPU.flags.OVERFLOW);
  printf("  INTERRUPT: %1d\n\n\n", CPU.flags.INTERRUPT);
}
