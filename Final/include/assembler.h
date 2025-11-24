#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t entry_point; // The memory adress of the :main label, or _start
  uint32_t text_start; // The start adress of the .text segment, where code is
  uint32_t text_size; // The size of the .text segment
  uint32_t data_start; // The start adress of the .data segment, where variables will be kept
  uint32_t data_size; // The size of the .data degment
  uint32_t stack_ptr; // Initial value of the stack pointer ($sp or $29 register)
  uint32_t globl_ptr; // Initial value of the global pointer ($gp or $28 register)
  char program_name[256];
} AssembledProgram;

/* ========= THE FOLLOWING DATATYPES AND FUNCTIONS ARE TO BE USED FOR DEBUGGING PURPOSES ONLY ========= */
/* ---------------------------------------------------------------------------------------------------- */
/* ====================================== END OF DUBUGGING TYPES ====================================== */

// The following is a clone of the internal representation of a Symbol
typedef struct {
  char name[64];
  uint32_t address;
  bool is_global;
  bool is_procedure;
} SymbolInfo;

// The result of the assembly of the program, along with helpful debugging info
typedef struct {
  AssembledProgram* program;
  SymbolInfo *symbols;
  int symbol_count;
  bool success;
  char error_message[512];
} AssemblyResult;

// Print the result of assembly
void print_assembly_result(const AssemblyResult *result);

// Get the adress of a symbol by it's name
int get_symbol_by_name(const AssemblyResult *result, const char *name);

/* ---------------------------------------------------------------------------------------------------- */
/* ====================================== END OF DUBUGGING TYPES ====================================== */

/*
 * Assemble a program, and write it to memory
 *
 * Parameters:
 *  filename: Path to the .asm(or .fsp) file
 *  memory: Pointer to the memory space to write to
 *  memory_size: Total size of the memory space
 *  process_id: Unique ID for this process (for isolation)
*/

AssemblyResult assemble(const char *filename,int process_id);

/*
 * Assemble multiple programs, and write them each to memory
 * Each program gets isolated memory regions
 * Returns the number of succesfully assembled programs
 * 
 * Parameters:
 *  filenames: Array of .asm(or .fsp) file paths
 *  file_count: Number of files to assemble
 *  memory: Shared memory space
 *  memory_size: Total memory size
 *  results: Output array for assembly results (must be allocated)
 */
int assemble_programs(const char **filenames,
                            int file_count,
                            AssemblyResult *results);


// Free all resources allocated by a program
void free_program(AssemblyResult *result);
#endif
