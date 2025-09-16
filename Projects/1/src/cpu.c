#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"

//CPU to control execution
Cpu CPU;

/* --- helpers ------------------------------------------------------------- */

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
static inline void set_zero_flag(CPU *cpu, word value) {
  cpu->flags.ZERO = (value == 0);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
static inline void set_add_flags(CPU *cpu, word a, word b, word r) {
  // Unsigned carry out of bit 15
  cpu->flags.CARRY =
      ((uint32_t)(uword)a + (uint32_t)(uword)b) > 0xFFFFu;

  // Signed overflow: inputs same sign, result different sign
  cpu->flags.OVERFLOW =
      ((a >= 0 && b >= 0 && r <  0) ||
       (a <  0 && b <  0 && r >= 0));

  set_zero_flag(cpu, r);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
static inline void set_sub_flags(CPU *cpu, word a, word b, word r) {
  // Borrow in unsigned
  cpu->flags.CARRY = ((uword)a < (uword)b);

  // Signed overflow: inputs different sign, result sign differs from a
  cpu->flags.OVERFLOW =
      ((a >= 0 && b <  0 && r <  0) ||
       (a <  0 && b >= 0 && r >= 0));

  set_zero_flag(cpu, r);
}

/* --- core ---------------------------------------------------------------- */

void init_cpu(Cpu* cpu)
{
  //initialize the flags of the given cpu
  void init_flags(Flags* flags)
  {
    flags->ZERO     = UNSET_FLAG;
    flags->CARRY    = UNSET_FLAG;
    flags->OVERFLOW = UNSET_FLAG;
  }
  
  cpu->EAX = EMPTY_REG;
  cpu->EBX = EMPTY_REG;
  cpu->ECX = EMPTY_REG;
  cpu->EDX = EMPTY_REG;
  cpu->PC  = 0;
  cpu->IR  = EMPTY_REG;
  cpu->ACC = EMPTY_REG;
  init_flags(cpu->flags);
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  mem_addr next_instruction_addr = CPU.pc;
  //cpu->IR = (instr)data_mem[next_instruction_addr];
  CPU.IR = read_mem(next_instruction_addr);
  CPU.PC++;
}

// Decodes the given instruction into its operator and operand
Decoded decode(instr instruction) {
  Decoded d;
  d.op   = (OP)((instruction & 0xF000u) >> 12);
  d.addr = (mem_addr)(instruction & 0x0FFFu);
  return d;
}

// Executes the instruction in the given cpu's IR with the given RAM
void execute(CPU* cpu, word* data_mem) {
  instr instruction = cpu->IR;
  Decoded d = decode(instruction);
  OP opcode = d.op;
  mem_addr operand = d.addr;

  switch (opcode) {
    case OP_LOAD:  load(cpu, data_mem, operand);  break;
    case OP_STORE: store(cpu, data_mem, operand); break;
    case OP_ADD:   add(cpu, data_mem, operand);   break;
    case OP_SUB:   sub(cpu, data_mem, operand);   break;
    case OP_HALT:  halt(cpu);                     break;
    default:
      printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode, (unsigned)instruction);
      cpu->PC = CPU_HALT;
  }
}

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(CPU* cpu, int program_size, word* mem) {
  for (int i = 0; i < program_size && cpu->PC != CPU_HALT; i++) {
    printf("=== Cycle %d ===\n", i + 1);
    cpu_print_state(cpu);

    fetch(cpu, mem);
    execute(cpu, mem);

    if (cpu->PC == CPU_HALT) {
      printf("CPU Halted!\n");
      break;
    }
  }
}

// Prints the state of the given CPU
void cpu_print_state(const CPU* cpu) {
  printf("CPU STATE\n");
  printf("PC: %X\n", cpu->PC);
  printf("ACC: %X\n", cpu->ACC);
  printf("IR: %X\n", cpu->IR);
  printf("FLAGS:\n");
  printf("  ZERO: %1d\n", cpu->flags.ZERO);
  printf("  CARRY: %1d\n", cpu->flags.CARRY);
  printf("  OVERFLOW: %1d\n\n\n", cpu->flags.OVERFLOW);
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
