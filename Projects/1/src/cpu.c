#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"

//CPU to control execution
Cpu CPU;

/* --- helpers ------------------------------------------------------------- */

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
void set_zero_flag(word value) {
  CPU.flags.ZERO = (value == 0);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
void set_add_flags(word a, word b, word r) {
  // Unsigned carry out of bit 15
  CPU.flags.CARRY =
      ((uint32_t)(uword)a + (uint32_t)(uword)b) > 0xFFFFu;

  // Signed overflow: inputs same sign, result different sign
  CPU.flags.OVERFLOW =
      ((a >= 0 && b >= 0 && r <  0) ||
       (a <  0 && b <  0 && r >= 0));

  set_zero_flag(r);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
void set_sub_flags(word a, word b, word r) {
  // Borrow in unsigned
  CPU.flags.CARRY = ((uword)a < (uword)b);

  // Signed overflow: inputs different sign, result sign differs from a
  CPU.flags.OVERFLOW =
      ((a >= 0 && b <  0 && r <  0) ||
       (a <  0 && b >= 0 && r >= 0));

  set_zero_flag(r);
}

/* --- core ---------------------------------------------------------------- */

void init_cpu(Cpu* cpu)
{
  //initialize the flags of the given cpu
  void init_flags(Flags flags)
  {
    flags.ZERO     = UNSET_FLAG;
    flags.CARRY    = UNSET_FLAG;
    flags.OVERFLOW = UNSET_FLAG;
  }
  
  cpu->EAX = EMPTY_REG;
  cpu->EBX = EMPTY_REG;
  cpu->ECX = EMPTY_REG;
  cpu->EDX = EMPTY_REG;
  cpu->PC  = 0;
  cpu->IR  = EMPTY_REG;
  cpu->ACC = 0;
  init_flags(cpu->flags);
  printf("Initialized the cpu!\n");
  cpu_print_state();
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  CPU.IR = read_mem(CPU.PC);
  CPU.PC++;
}

// Decodes the given instruction into its operator and operand
Decoded decode (word instruction) {
  Decoded d;
  d.op   = (OP)((instruction & 0xF000u) >> 12);
  d.addr = (mem_addr)(instruction & 0x0FFFu);
  return d;
}

// Executes the instruction in the given cpu's IR with the given RAM
void execute() {
  word instruction = CPU.IR;
  Decoded d = decode(instruction);
  OP opcode = d.op;
  mem_addr operand = d.addr;
  printf("opcode == %X\n", d.op);
  printf("operand == %X\n", d.addr);

  execute_instruction(opcode, operand);
}

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(int program_size, word* mem) {
  for (int i = 0; i < program_size && CPU.PC != CPU_HALT; i++) {
    printf("=== Cycle %d ===\n", i + 1);

    if (CPU.PC == CPU_HALT) {
      printf("CPU Halted!\n");
      break;
    }
    fetch();
    execute();

    cpu_print_state();
  }
}

// Prints the state of the given CPU
void cpu_print_state() {
  printf("CPU STATE\n");
  printf("PC:  %X\n", CPU.PC);
  printf("ACC: %X\n", CPU.ACC);
  printf("IR:  %X\n", CPU.IR);
  printf("FLAGS:\n");
  printf("  ZERO:     %1d\n", CPU.flags.ZERO);
  printf("  CARRY:    %1d\n", CPU.flags.CARRY);
  printf("  OVERFLOW: %1d\n\n\n", CPU.flags.OVERFLOW);
}

/*
int main() {
  struct flags flags = {0, 0, 0};
  struct cpu cpu = {0x300, 0, 0, flags}; 
  
  //word memory[0x1000];
  //memory[0x300] = 0x1940;
  //memory[0x301] = 0x5941;
  //memory[0x302] = 0x2941;
  //memory[0x303] = 0x6942;
  //memory[0x304] = 0x2942;
  //memory[0x940] = 0x0003;
  //memory[0x941] = 0x0002;
  //memory[0x942] = 0x0001;
  
  init_cache(&L1, L1CACHE_SIZE);
  init_cache(&L2, L2CACHE_SIZE);
  init_ram(RAM_SIZE);

  
  //cpu_run(&cpu, 5, memory);
  
  //printf("0x941: %X\n", memory[0x941]);
  //printf("0x942: %X\n", memory[0x942]);
  

  write_mem(0x300, 0x1940);
  printf("read adress 0x300 <-> 0x%X\n", read_mem(0x300));
  cpu_run(&cpu, 5, RAM);
  print_cache_stats();
}
*/
