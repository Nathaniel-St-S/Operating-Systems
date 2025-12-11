// MIPS-1 Assembler Implementation
#include "../include/assembler.h"
#include "../include/memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 512
#define MAX_SYMBOLS 1024
#define MAX_INSTRUCTIONS 4096
#define MAX_DATA 8192
#define MAX_MACROS 256
#define MAX_MACRO_LINES 128
#define MAX_MACRO_PARAMS 16

/* ---------------------------------------------------------------------------------------------------- */
/* ========================================= INTERNAL STRUCTS ========================================= */

typedef struct {
  char name[64];
  uint32_t address;
  bool is_global;
  bool is_procedure;
} Symbol;

typedef struct {
  char name[64];
  char params[MAX_MACRO_PARAMS][32];
  uint8_t param_count;
  char* lines[MAX_MACRO_LINES];
  uint8_t line_count;
} Macro;

typedef struct {
  uint32_t address;
  uint8_t data[MAX_DATA];
  uint32_t size;
} DataSegment;

typedef struct {
  uint32_t address;
  char* line;
  int line_number;
} TextEntry;

// Assembly context (keeps state during assembly)
typedef struct {
  Symbol symbols[MAX_SYMBOLS];
  uint16_t symbol_count;

  Macro macros[MAX_MACROS];
  uint8_t macro_count;

  DataSegment data_segment;
  TextEntry text_segment[MAX_INSTRUCTIONS];
  uint16_t text_count;

  uint32_t current_address;
  uint16_t source_line;

  bool in_data_section;
  bool align_mode;

  uint32_t text_base;
  uint32_t data_base;

  // Keeps track of the physical memory 
  // addresses given by the system
  // when malocate is called
  uint32_t allocated_text_addr;
  uint32_t allocated_data_addr;

  int process_id;
} AssemblyContext;

/* ---------------------------------------------------------------------------------------------------- */
/* ========================================== UTILITY FUNCTS ========================================== */

// Trim the leading and trailing white space from a string.
static char* trim(char* str){
  // Trim leading white spaces
  while(isspace(*str)) {str++;}
  if (*str == 0){return str;}

  char* end = str + strlen(str) -1;

  // Trim trailing blank spaces
  while (end > str && isspace(*end)) {end--;}

  // Add back null terminator
  *(end + 1) = 0;
  return str;
}

// Convert a string into it's numeric value
static int32_t parse_num (const char* str){
  if (!str) {return 0;}

  // Binary number
  if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B'))
    return (int32_t)strtol(str + 2, NULL, 2);

  // Hex number
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    return (int32_t)strtol(str, NULL, 16);

  // Octal number
  if (str[0] == '0' && isdigit(str[1]))
    return (int32_t)strtol(str, NULL, 8);

  // Decimal Number
  return (int32_t)strtol(str, NULL, 10); 
}

static int get_register(const char* reg){
  if (!reg || reg[0] != '$') {return -1;}
  if (isdigit(reg[1])){
    int num = atoi(reg +1);
    return (num >= 0 && num <=31 ) ? num : -1;
  }

  const char* names[] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
  };

  for (int i = 0; i < 32; i++){
    if (strcmp(reg, names[i]) == 0) {return i;}
  }
  return -1;
}

// At some point maybe i'll hash this so it can be faster
static void add_symbol(AssemblyContext *ctx, const char *name, uint32_t address, int is_global, int is_proc) {
  // Update symbol if it already exists
  for (int i = 0; i < ctx->symbol_count; i++) {
    if (strcmp(ctx->symbols[i].name, name) == 0) {
      ctx->symbols[i].address = address;

      if (is_global) { ctx->symbols[i].is_global = 1;}
      if (is_proc) {ctx->symbols[i].is_procedure = 1;}

      return;
    }
  }

  // Add in the new symbol
  if (ctx->symbol_count < MAX_SYMBOLS) {
    strncpy(ctx->symbols[ctx->symbol_count].name, name, 63);
    ctx->symbols[ctx->symbol_count].name[63] = '\0';
    ctx->symbols[ctx->symbol_count].address = address;
    ctx->symbols[ctx->symbol_count].is_global = is_global ? 1 : 0;
    ctx->symbols[ctx->symbol_count].is_procedure = is_proc ? 1 : 0;
    ctx->symbol_count++;
  }
}

// Same, hash this as well
static int get_symbol_address(AssemblyContext *ctx, const char *name) {
  for (int i = 0; i < ctx->symbol_count; i++) {
    if (strcmp(ctx->symbols[i].name, name) == 0) {
      return ctx->symbols[i].address;
    }
  }
  return -1;
}

// Evil ass wizardry
// Align the data in .data to 2^boundary bytes
// disable with .align 0
static void align_data(AssemblyContext *ctx, int boundary) {
  if (ctx->align_mode && boundary > 0) {
    int mask = (1 << boundary) - 1;
    if (ctx->data_segment.size & mask) {
      ctx->data_segment.size = (ctx->data_segment.size + mask) & ~mask;
    }
  }
}

static void handle_word(AssemblyContext *ctx, char *values) {
  // Words need to be aligned to 4 bytes
  align_data(ctx, 2);
  char *token = strtok(values, ",");
  while (token && ctx->data_segment.size + 4 <= MAX_DATA) {
    token = trim(token);
    if (!token || token[0] == '\0') {
      token = strtok(NULL, ",");
      continue;
    }
    int32_t value = parse_num(token);
    ctx->data_segment.data[ctx->data_segment.size++] = value & 0xFF;
    ctx->data_segment.data[ctx->data_segment.size++] = (value >> 8) & 0xFF;
    ctx->data_segment.data[ctx->data_segment.size++] = (value >> 16) & 0xFF;
    ctx->data_segment.data[ctx->data_segment.size++] = (value >> 24) & 0xFF;
    token = strtok(NULL, ",");
  }
}

static void handle_half(AssemblyContext *ctx, char *values) {
  // halfwords need to be aligned to 2 bytes
  align_data(ctx, 1);
  char *token = strtok(values, ",");
  while (token && ctx->data_segment.size + 2 <= MAX_DATA) {
    token = trim(token);
    if (!token || token[0] == '\0') {
      token = strtok(NULL, ",");
      continue;
    }
    int16_t value = (int16_t)parse_num(token);
    ctx->data_segment.data[ctx->data_segment.size++] = value & 0xFF;
    ctx->data_segment.data[ctx->data_segment.size++] = (value >> 8) & 0xFF;
    token = strtok(NULL, ",");
  }
}

static void handle_byte(AssemblyContext *ctx, char *values) {
  char *token = strtok(values, ",");
  while (token && ctx->data_segment.size < MAX_DATA) {
    token = trim(token);
    if (!token || token[0] == '\0') {
      token = strtok(NULL, ",");
      continue;
    }
    if (token[0] == '\'' && token[2] == '\'') {
      ctx->data_segment.data[ctx->data_segment.size++] = token[1];
    } else {
      int8_t value = (int8_t)parse_num(token);
      ctx->data_segment.data[ctx->data_segment.size++] = value;
    }
    token = strtok(NULL, ",");
  }
}

static void handle_ascii(AssemblyContext *ctx, char *str, int null_terminate) {
  int in_string = 0;
  for (int i = 0; str[i] && ctx->data_segment.size < MAX_DATA; i++) {
    if (str[i] == '"') {
      in_string = !in_string;
    } else if (in_string) {
      if (str[i] == '\\' && str[i+1]) {
        i++;
        switch (str[i]) {
          case 'n': ctx->data_segment.data[ctx->data_segment.size++] = '\n'; break;
          case 't': ctx->data_segment.data[ctx->data_segment.size++] = '\t'; break;
          case 'r': ctx->data_segment.data[ctx->data_segment.size++] = '\r'; break;
          case '0': ctx->data_segment.data[ctx->data_segment.size++] = '\0'; break;
          case '\\': ctx->data_segment.data[ctx->data_segment.size++] = '\\'; break;
          case '"': ctx->data_segment.data[ctx->data_segment.size++] = '"'; break;
          default: ctx->data_segment.data[ctx->data_segment.size++] = str[i]; break;
        }
      } else {
        ctx->data_segment.data[ctx->data_segment.size++] = str[i];
      }
    }
  }
  if (null_terminate && ctx->data_segment.size < MAX_DATA) {
    ctx->data_segment.data[ctx->data_segment.size++] = '\0';
  }
}

static void handle_space(AssemblyContext *ctx, int bytes) {
  // Align to 4 bytes for .space directives
  align_data(ctx, 2);  // Add this line
  for (int i = 0; i < bytes && ctx->data_segment.size < MAX_DATA; i++) {
    ctx->data_segment.data[ctx->data_segment.size++] = 0;
  }
}

/* ---------------------------------------------------------------------------------------------------- */
/* ============================================= ASSEMBLY ============================================= */

static void init_context(AssemblyContext *ctx, int process_id) {
  memset(ctx, 0, sizeof(AssemblyContext));
  ctx->process_id = process_id;
  ctx->text_base = TEXT_BASE + (process_id * MAX_PROCESS_SIZE);  // 1MB per process
  ctx->data_base = DATA_BASE + (process_id * MAX_PROCESS_SIZE);
  ctx->current_address = ctx->text_base;
  ctx->data_segment.address = ctx->data_base;
  ctx->align_mode = 1;
}

// Stupid ass language can't match with strings so BEHOLD
static uint32_t assemble_r_type(const char *op, int rd, int rs, int rt, int shamt) {
  uint32_t funct = 0;

  if (strcmp(op, "add") == 0) funct = 0x20;
  else if (strcmp(op, "addu") == 0) funct = 0x21;
  else if (strcmp(op, "sub") == 0) funct = 0x22;
  else if (strcmp(op, "subu") == 0) funct = 0x23;
  else if (strcmp(op, "and") == 0) funct = 0x24;
  else if (strcmp(op, "or") == 0) funct = 0x25;
  else if (strcmp(op, "xor") == 0) funct = 0x26;
  else if (strcmp(op, "nor") == 0) funct = 0x27;
  else if (strcmp(op, "slt") == 0) funct = 0x2a;
  else if (strcmp(op, "sltu") == 0) funct = 0x2b;
  else if (strcmp(op, "sll") == 0) funct = 0x00;
  else if (strcmp(op, "srl") == 0) funct = 0x02;
  else if (strcmp(op, "sra") == 0) funct = 0x03;
  else if (strcmp(op, "sllv") == 0) funct = 0x04;
  else if (strcmp(op, "srlv") == 0) funct = 0x06;
  else if (strcmp(op, "srav") == 0) funct = 0x07;
  else if (strcmp(op, "jr") == 0) funct = 0x08;
  else if (strcmp(op, "jalr") == 0) funct = 0x09;
  else if (strcmp(op, "syscall") == 0) funct = 0x0c;
  else if (strcmp(op, "break") == 0) funct = 0x0d;
  else if (strcmp(op, "mfhi") == 0) funct = 0x10;
  else if (strcmp(op, "mthi") == 0) funct = 0x11;
  else if (strcmp(op, "mflo") == 0) funct = 0x12;
  else if (strcmp(op, "mtlo") == 0) funct = 0x13;
  else if (strcmp(op, "mult") == 0) funct = 0x18;
  else if (strcmp(op, "multu") == 0) funct = 0x19;
  else if (strcmp(op, "div") == 0) funct = 0x1a;
  else if (strcmp(op, "divu") == 0) funct = 0x1b;

  return ((uint32_t)rs << 21) | ((uint32_t)rt << 16) | ((uint32_t)rd << 11) | ((uint32_t)shamt << 6) | funct;
}

static uint32_t assemble_i_type(const char *op, int rt, int rs, int16_t imm) {
  uint32_t opcode = 0;

  if (strcmp(op, "addi") == 0) opcode = 0x08;
  else if (strcmp(op, "addiu") == 0) opcode = 0x09;
  else if (strcmp(op, "andi") == 0) opcode = 0x0c;
  else if (strcmp(op, "ori") == 0) opcode = 0x0d;
  else if (strcmp(op, "xori") == 0) opcode = 0x0e;
  else if (strcmp(op, "lui") == 0) opcode = 0x0f;
  else if (strcmp(op, "slti") == 0) opcode = 0x0a;
  else if (strcmp(op, "sltiu") == 0) opcode = 0x0b;
  else if (strcmp(op, "beq") == 0) opcode = 0x04;
  else if (strcmp(op, "bne") == 0) opcode = 0x05;
  else if (strcmp(op, "blez") == 0) opcode = 0x06;
  else if (strcmp(op, "bgtz") == 0) opcode = 0x07;
  else if (strcmp(op, "lb") == 0) opcode = 0x20;
  else if (strcmp(op, "lh") == 0) opcode = 0x21;
  else if (strcmp(op, "lw") == 0) opcode = 0x23;
  else if (strcmp(op, "lbu") == 0) opcode = 0x24;
  else if (strcmp(op, "lhu") == 0) opcode = 0x25;
  else if (strcmp(op, "sb") == 0) opcode = 0x28;
  else if (strcmp(op, "sh") == 0) opcode = 0x29;
  else if (strcmp(op, "sw") == 0) opcode = 0x2b;

  return (opcode << 26) | ((uint32_t)rs << 21) | ((uint32_t)rt << 16) | (imm & 0xFFFF);
}

static uint32_t assemble_j_type(const char *op, uint32_t addr) {
  uint32_t opcode = (strcmp(op, "j") == 0) ? 0x02 : 0x03;
  return (opcode << 26) | ((addr >> 2) & 0x3FFFFFF);
}

static int validate_register_num(int reg, const char *name, const char *op, uint32_t pc) {
  if (reg < 0 || reg > 31) {
    fprintf(stderr, "Error: Invalid register '%s' for %s at PC 0x%08x\n",
            name ? name : "<null>", op, pc);
    return 0;
  }
  return 1;
}

// Assemble MIPS-1 psuedo instructions
static int expand_pseudo(AssemblyContext *ctx, const char *op, char *operands, 
    char output[][MAX_LINE], uint32_t pc) {
  (void)pc;
  char args[4][64];
  int argc = 0;
  char *saveptr = NULL;
  char *token = strtok_r(operands, " ,\t()", &saveptr);
  while (token && argc < 4) {
    strcpy(args[argc++], trim(token));
    token = strtok_r(NULL, " ,\t()", &saveptr);
  }

  if (strcmp(op, "li") == 0 && argc == 2) {
    int32_t imm = parse_num(args[1]);
    if (imm >= -32768 && imm <= 32767) {
      sprintf(output[0], "addiu %s, $zero, %d", args[0], imm);
      return 1;
    } else {
      sprintf(output[0], "lui %s, %d", args[0], (imm >> 16) & 0xFFFF);
      sprintf(output[1], "ori %s, %s, %d", args[0], args[0], imm & 0xFFFF);
      return 2;
    }
  }

  if (strcmp(op, "la") == 0 && argc == 2) {
    int addr = get_symbol_address(ctx, args[1]);
    if (addr == -1) {
      fprintf(stderr, "Warning: Undefined label '%s' for la, using 0\n", args[1]);
      addr = 0;
    }
    if (addr >= -32768 && addr <= 32767) {
      sprintf(output[0], "addiu %s, $zero, %d", args[0], addr);
      return 1;
    } else {
      sprintf(output[0], "lui %s, %d", args[0], (addr >> 16) & 0xFFFF);
      sprintf(output[1], "ori %s, %s, %d", args[0], args[0], addr & 0xFFFF);
      return 2;
    }
  }

  if (strcmp(op, "move") == 0 && argc == 2) {
    sprintf(output[0], "addu %s, %s, $zero", args[0], args[1]);
    return 1;
  }

  if (strcmp(op, "nop") == 0) {
    sprintf(output[0], "sll $zero, $zero, 0");
    return 1;
  }

  if ((strcmp(op, "lw") == 0 || strcmp(op, "sw") == 0 ||
        strcmp(op, "lb") == 0 || strcmp(op, "sb") == 0 ||
        strcmp(op, "lh") == 0 || strcmp(op, "sh") == 0 ||
        strcmp(op, "lbu") == 0 || strcmp(op, "lhu") == 0) && argc == 2) {

    // Check if second arg has no parentheses (it's a label)
    if (strchr(args[1], '(') == NULL) {
      int label_addr = get_symbol_address(ctx, args[1]);
      if (label_addr == -1) {
        fprintf(stderr, "Warning: Undefined label '%s', using 0\n", args[1]);
        label_addr = 0;
      }

      if (label_addr >= -32768 && label_addr <= 32767) {
        sprintf(output[0], "%s %s, %d($zero)", op, args[0], label_addr);
        return 1;
      } else {
        sprintf(output[0], "lui $at, %d", (label_addr >> 16) & 0xFFFF);
        sprintf(output[1], "ori $at, $at, %d", label_addr & 0xFFFF);
        sprintf(output[2], "%s %s, 0($at)", op, args[0]);
        return 3;
      }
    }
  }

  return 0;
}

static uint32_t assemble_line(AssemblyContext *ctx, const char *line, uint32_t pc) {
  char buffer[MAX_LINE];
  strncpy(buffer, line, MAX_LINE - 1);
  buffer[MAX_LINE - 1] = '\0';

  char *saveptr = NULL;
  char *op = strtok_r(buffer, " \t,()", &saveptr);
  if (!op) return 0;

  // Check pseudo-instructions
  char expanded[4][MAX_LINE];
  char rest[MAX_LINE] = "";
  const char *operand_str = line;
  while (*operand_str && !isspace((unsigned char)*operand_str)) {
    operand_str++;
  }
  while (*operand_str && isspace((unsigned char)*operand_str)) {
    operand_str++;
  }
  if (*operand_str) {
    strncpy(rest, operand_str, MAX_LINE - 1);
    rest[MAX_LINE - 1] = '\0';
  }

  int exp_count = expand_pseudo(ctx, op, rest, expanded, pc);
  if (exp_count > 0) {
    return assemble_line(ctx, expanded[0], pc);
  }

  // R-type
  if (strcmp(op, "add") == 0 || strcmp(op, "addu") == 0 ||
      strcmp(op, "sub") == 0 || strcmp(op, "subu") == 0 ||
      strcmp(op, "and") == 0 || strcmp(op, "or") == 0 ||
      strcmp(op, "xor") == 0 || strcmp(op, "nor") == 0 ||
      strcmp(op, "slt") == 0 || strcmp(op, "sltu") == 0) {
    char *rd = strtok_r(NULL, " \t,()", &saveptr);
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    int rd_num = get_register(rd);
    int rs_num = get_register(rs);
    int rt_num = get_register(rt);
    if (!validate_register_num(rd_num, rd, op, pc) ||
        !validate_register_num(rs_num, rs, op, pc) ||
        !validate_register_num(rt_num, rt, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, rd_num, rs_num, rt_num, 0);
  }

  if (strcmp(op, "sll") == 0 || strcmp(op, "srl") == 0 || strcmp(op, "sra") == 0) {
    char *rd = strtok_r(NULL, " \t,()", &saveptr);
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    char *shamt = strtok_r(NULL, " \t,()", &saveptr);
    int rd_num = get_register(rd);
    int rt_num = get_register(rt);
    if (!validate_register_num(rd_num, rd, op, pc) ||
        !validate_register_num(rt_num, rt, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, rd_num, 0, rt_num, parse_num(shamt));
  }

  if (strcmp(op, "mult") == 0 || strcmp(op, "multu") == 0 ||
      strcmp(op, "div") == 0  || strcmp(op, "divu") == 0) {
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    int rs_num = get_register(rs);
    int rt_num = get_register(rt);
    if (!validate_register_num(rs_num, rs, op, pc) ||
        !validate_register_num(rt_num, rt, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, 0, rs_num, rt_num, 0);
  }

  if (strcmp(op, "mfhi") == 0 || strcmp(op, "mflo") == 0) {
    char *rd = strtok_r(NULL, " \t,()", &saveptr);
    int rd_num = get_register(rd);
    if (!validate_register_num(rd_num, rd, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, rd_num, 0, 0, 0);
  }

  if (strcmp(op, "mthi") == 0 || strcmp(op, "mtlo") == 0) {
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    int rs_num = get_register(rs);
    if (!validate_register_num(rs_num, rs, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, 0, rs_num, 0, 0);
  }

  // I-type
  if (strcmp(op, "addi") == 0 || strcmp(op, "addiu") == 0 ||
      strcmp(op, "andi") == 0 || strcmp(op, "ori") == 0 ||
      strcmp(op, "xori") == 0 || strcmp(op, "slti") == 0 ||
      strcmp(op, "sltiu") == 0) {
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    char *imm = strtok_r(NULL, " \t,()", &saveptr);
    int rt_num = get_register(rt);
    int rs_num = get_register(rs);
    if (!validate_register_num(rt_num, rt, op, pc) ||
        !validate_register_num(rs_num, rs, op, pc)) {
      return 0;
    }
    return assemble_i_type(op, rt_num, rs_num, (int16_t)parse_num(imm));
  }

  if (strcmp(op, "lui") == 0) {
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    char *imm = strtok_r(NULL, " \t,()", &saveptr);
    int rt_num = get_register(rt);
    if (!validate_register_num(rt_num, rt, op, pc)) {
      return 0;
    }
    return assemble_i_type(op, rt_num, 0, (int16_t)parse_num(imm));
  }

  // Load/Store
  if (strcmp(op, "lw") == 0 || strcmp(op, "sw") == 0 ||
      strcmp(op, "lb") == 0 || strcmp(op, "sb") == 0 ||
      strcmp(op, "lh") == 0 || strcmp(op, "sh") == 0 ||
      strcmp(op, "lbu") == 0 || strcmp(op, "lhu") == 0) {

    char *rt = strtok_r(NULL, " \t,", &saveptr);
    if (!rt) {
      fprintf(stderr, "Error: Missing rt for %s at PC 0x%08x\n", op, pc);
      return 0;
    }

    // Get offset($rs) part
    char *offset_part = strtok_r(NULL, "", &saveptr);
    if (!offset_part) {
      fprintf(stderr, "Error: Missing offset($rs) for %s at PC 0x%08x\n", op, pc);
      return 0;
    }

    // Trim and find parentheses
    while (*offset_part && isspace((unsigned char)*offset_part)) offset_part++;

    char *open_paren = strchr(offset_part, '(');
    char *close_paren = strchr(offset_part, ')');

    if (!open_paren || !close_paren) {
      fprintf(stderr, "Error: Bad format for %s at PC 0x%08x (expected offset($rs))\n", op, pc);
      return 0;
    }

    *open_paren = '\0';
    *close_paren = '\0';

    char *offset_str = offset_part;
    char *rs_str = open_paren + 1;

    // Trim
    while (*offset_str && isspace(*offset_str)) offset_str++;
    while (*rs_str && isspace(*rs_str)) rs_str++;

    int rt_num = get_register(rt);
    int rs_num = get_register(rs_str);
    int16_t offset = (offset_str && *offset_str) ? (int16_t)parse_num(offset_str) : 0;

    if (!validate_register_num(rt_num, rt, op, pc) ||
        !validate_register_num(rs_num, rs_str, op, pc)) {
      return 0;
    }

    return assemble_i_type(op, rt_num, rs_num, offset);
  }

  // Branches
  if (strcmp(op, "beq") == 0 || strcmp(op, "bne") == 0) {
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    char *rt = strtok_r(NULL, " \t,()", &saveptr);
    char *label = strtok_r(NULL, " \t,()", &saveptr);

    if (!label) {
      fprintf(stderr, "Error: Missing label for %s at PC 0x%08x\n", op, pc);
      return 0;
    }

    int rs_num = get_register(rs);
    int rt_num = get_register(rt);
    if (!validate_register_num(rs_num, rs, op, pc) ||
        !validate_register_num(rt_num, rt, op, pc)) {
      return 0;
    }

    int target = get_symbol_address(ctx, label);
    if (target == -1) {
      fprintf(stderr, "Error: Undefined label '%s' at PC 0x%08x\n", label, pc);
      return 0;
    }
    int16_t offset = (target - (pc + 4)) / 4;
    return assemble_i_type(op, rt_num, rs_num, offset);
  }

  // Jumps
  if (strcmp(op, "j") == 0 || strcmp(op, "jal") == 0) {
    char *label = strtok_r(NULL, " \t,()", &saveptr);
    if (!label) {
      fprintf(stderr, "Error: Missing label for %s at PC 0x%08x\n", op, pc);
      return 0;
    }
    int target = get_symbol_address(ctx, label);
    if (target == -1) {
      fprintf(stderr, "Error: Undefined label '%s' for %s at PC 0x%08x\n", 
          label, op, pc);
      return 0;
    }
    return assemble_j_type(op, target);
  }

  if (strcmp(op, "jr") == 0) {
    char *rs = strtok_r(NULL, " \t,()", &saveptr);
    int rs_num = get_register(rs);
    if (!validate_register_num(rs_num, rs, op, pc)) {
      return 0;
    }
    return assemble_r_type(op, 0, rs_num, 0, 0);
  }

  if (strcmp(op, "syscall") == 0 || strcmp(op, "break") == 0) {
    return assemble_r_type(op, 0, 0, 0, 0);
  }

  if (strcmp(op, "eret") == 0) {
    return (0x10 << 26) | (1 << 25) | 0x18;
  }

  return 0;
}

// Mallocate now uses mallocate instead of just rawdogging it
// like they did in the 70's. so now the isntructions can be 
// properly loaded in to memory, and freed with just the pid
static int write_to_memory(AssemblyContext *ctx) {
  // Set current process for memory access control
  set_current_process(ctx->process_id);

  // Mallocates memory for text segment, cause i lowkey just
  // realized we don't have to allocate one big blob at once
  uint32_t text_size = ctx->text_count * 4;
  if (text_size > 0) { // No allocation if corrupted program
    ctx->allocated_text_addr = mallocate(ctx->process_id, text_size);
    if (ctx->allocated_text_addr == UINT32_MAX) {
      fprintf(stderr, "Failed to allocate %u bytes for text segment (PID %d)\n", 
              text_size, ctx->process_id);
      set_current_process(SYSTEM_PROCESS_ID);
      return 0;
    }
    printf("Allocated text: 0x%08x (%u bytes)\n", 
           ctx->allocated_text_addr, text_size);
  }

  // And now just allocate the text segment cause it's so much
  // simpler than tryin to manually split up the memory :]
  // don't ask what i was doing before this
  if (ctx->data_segment.size > 0) {
    ctx->allocated_data_addr = mallocate(ctx->process_id, ctx->data_segment.size);
    if (ctx->allocated_data_addr == UINT32_MAX) {
      fprintf(stderr, "Failed to allocate %u bytes for data segment (PID %d)\n", 
              ctx->data_segment.size, ctx->process_id);
      // Clean up text allocation if data fails
      if (ctx->allocated_text_addr != UINT32_MAX) {
        liberate(ctx->process_id);
      }
      set_current_process(SYSTEM_PROCESS_ID);
      return 0;
    }
    printf("  ✓ Allocated data: 0x%08x (%u bytes)\n", 
           ctx->allocated_data_addr, ctx->data_segment.size);
  }

  // Rebase symbol addresses to the allocated physical addresses so that
  // any references (e.g., via `la`) point at real memory we control.
  for (int i = 0; i < ctx->symbol_count; i++) {
    if (ctx->symbols[i].address >= ctx->text_base &&
        ctx->symbols[i].address < ctx->text_base + ctx->text_count * 4) {
      uint32_t off = ctx->symbols[i].address - ctx->text_base;
      ctx->symbols[i].address = ctx->allocated_text_addr + off;
    } else if (ctx->symbols[i].address >= ctx->data_base &&
               ctx->symbols[i].address < ctx->data_base + ctx->data_segment.size) {
      uint32_t off = ctx->symbols[i].address - ctx->data_base;
      ctx->symbols[i].address = ctx->allocated_data_addr + off;
    }
  }

  // Write text segment
  for (int i = 0; i < ctx->text_count; i++) {
    uint32_t offset = i * 4;
    uint32_t addr = ctx->allocated_text_addr + offset;
    uint32_t code = assemble_line(ctx, ctx->text_segment[i].line,
                                  ctx->allocated_text_addr + offset);
    
    write_word(addr, code);
    
    free(ctx->text_segment[i].line);
  }

  // Write data segment
  for (uint32_t i = 0; i < ctx->data_segment.size; i++) {
    write_byte(ctx->allocated_data_addr + i, ctx->data_segment.data[i]);
  }
  
  // Reset to system mode after assembly
  set_current_process(SYSTEM_PROCESS_ID);

  return 1;
}
/* ---------------------------------------------------------------------------------------------------- */
/* ============================================== PARSER ============================================== */

static int parse_file(AssemblyContext *ctx, const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) return 0;

  char line[MAX_LINE];

  // Pass 1: Collect symbols and directives
  while (fgets(line, MAX_LINE, fp)) {
    ctx->source_line++;
    char *trimmed = trim(line);
    if (trimmed[0] == '\0' || trimmed[0] == '#') continue;

    char buffer[MAX_LINE];
    strcpy(buffer, trimmed);

    // Labels
    char *colon = strchr(buffer, ':');
    if (colon) {
      *colon = '\0';
      char *label = trim(buffer);
      if (ctx->in_data_section) {
        add_symbol(ctx, label, ctx->data_segment.size + ctx->data_base, 0, 0);
      } else {
        add_symbol(ctx, label, ctx->current_address, 0, 0);
      }
      trimmed = trim(colon + 1);
      if (trimmed[0] == '\0' || trimmed[0] == '#') continue;
      memmove(buffer, trimmed, strlen(trimmed));
      buffer[MAX_LINE - 1] = '\0';
    }

    // Strip inline comments
    char *comment = strchr(buffer, '#');
    if (comment) {
      *comment = '\0';
      trimmed = trim(buffer);
      if (trimmed[0] == '\0') continue;
      if (trimmed != buffer) {
        memmove(buffer, trimmed, strlen(trimmed) + 1);
      }
    }

    // Directives
    if (buffer[0] == '.') {
      char *directive = strtok(buffer, " \t");
      char *rest = strtok(NULL, "");
      if (rest) rest = trim(rest);

      if (strcmp(directive, ".data") == 0) {
        ctx->in_data_section = 1;
      }
      else if (strcmp(directive, ".text") == 0) {
        ctx->in_data_section = 0;
        ctx->current_address = ctx->text_base;
      }
      else if (strcmp(directive, ".globl") == 0 && rest) {
        add_symbol(ctx, rest, 0, 1, 0);
      }
      else if (ctx->in_data_section) {
        if (strcmp(directive, ".word") == 0 && rest) handle_word(ctx, rest);
        else if (strcmp(directive, ".half") == 0 && rest) handle_half(ctx, rest);
        else if (strcmp(directive, ".byte") == 0 && rest) handle_byte(ctx, rest);
        else if (strcmp(directive, ".ascii") == 0 && rest) handle_ascii(ctx, rest, 0);
        else if (strcmp(directive, ".asciiz") == 0 && rest) handle_ascii(ctx, rest, 1);
        else if (strcmp(directive, ".space") == 0 && rest) handle_space(ctx, parse_num(rest));
        else if (strcmp(directive, ".align") == 0 && rest) {
          int n = parse_num(rest);
          if (n == 0) ctx->align_mode = 0;
          else { ctx->align_mode = 1; align_data(ctx, n); }
        }
      }
      continue;
    }

    // Instructions
    if (!ctx->in_data_section) {

      if (ctx->text_count >= MAX_INSTRUCTIONS) {
        fprintf(stderr, "Error: Too many instructions (max %d) at line %d\n", 
            MAX_INSTRUCTIONS, ctx->source_line);
        fclose(fp);
        return 0;
      }

      ctx->text_segment[ctx->text_count].address = ctx->current_address;
      ctx->text_segment[ctx->text_count].line = strdup(trimmed);
      if (!ctx->text_segment[ctx->text_count].line) {
        fprintf(stderr, "Error: Memory allocation failed at line %d\n", ctx->source_line);
        fclose(fp);
        return 0;
      }
      ctx->text_segment[ctx->text_count].line_number = ctx->source_line;
      ctx->text_count++;
      ctx->current_address += 4;
    }
  }

  fclose(fp);
  return 1;
}

/* ---------------------------------------------------------------------------------------------------- */
/* ============================================ API FUNCTS ============================================ */

AssemblyResult assemble(const char *filename, int process_id) {
  AssemblyResult result = {0};

  result.program = malloc(sizeof(AssembledProgram));
  if (!result.program) {
    snprintf(result.error_message, 511, "Memory allocation failed");
    result.success = 0;
    return result;
  }
  memset(result.program, 0, sizeof(AssembledProgram));

  AssemblyContext ctx;

  init_context(&ctx, process_id);

  // Extract process name from filename
  const char *basename = strrchr(filename, '/');
  basename = basename ? basename + 1 : filename;

  // Sanitize: only allow alphanumeric, underscore, dash, dot
  char sanitized[256];
  int j = 0;
  for (int i = 0; basename[i] && j < 255; i++) {
    char c = basename[i];
    if (isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.') {
      sanitized[j++] = c;
    }
  }
  sanitized[j] = '\0';

  strncpy(result.program->program_name, sanitized, 255);
  result.program->program_name[255] = '\0';

  // Parse assembly file
  if (!parse_file(&ctx, filename)) {
    snprintf(result.error_message, 511, "Failed to open file: %s", filename);
    result.success = 0;
    free(result.program);
    result.program = NULL;
    return result;
  }

  // Write to memory
  if (!write_to_memory(&ctx)) {
    snprintf(result.error_message, 511, "Failed to allocate/write memory");
    result.success = 0;
    free(result.program);
    result.program = NULL;
    return result;
  }

  // Fill in process image
  result.program->text_start = ctx.allocated_text_addr;  // ACTUAL allocated address
  result.program->text_size = ctx.text_count * 4;
  result.program->data_start = ctx.allocated_data_addr;  // ACTUAL allocated address
  result.program->data_size = ctx.data_segment.size;
  uint32_t stack_size = 8192; // 8KB stack
  uint32_t stack_addr = mallocate(process_id, stack_size);
  if (stack_addr == UINT32_MAX) {
    fprintf(stderr, "Failed to allocate stack for PID %d\n", process_id);
    result.success = 0;
    free(result.program);
    result.program = NULL;
    return result;
  }
  result.program->stack_ptr = stack_addr + stack_size - 16;
  result.program->globl_ptr = ctx.allocated_data_addr;

  // Find entry point
  int entry_sym = get_symbol_address(&ctx, "main");
  if (entry_sym == -1) {
    // Default to first instruction at TEXT_BASE
    result.program->entry_point = ctx.text_base;
  } else {
    result.program->entry_point = entry_sym;
  }

  // Readjust the memory adress to a physical adress
  if (result.program->entry_point >= ctx.text_base &&
      result.program->entry_point < ctx.text_base + ctx.text_count * 4) {
    uint32_t offset = result.program->entry_point - ctx.text_base;
    result.program->entry_point = ctx.allocated_text_addr + offset;
  }

  // Copy symbols for debugging
  result.symbol_count = ctx.symbol_count;
  result.symbols = malloc(sizeof(SymbolInfo) * ctx.symbol_count);
  if (result.symbols) {
    for (int i = 0; i < ctx.symbol_count; i++) {
      strncpy(result.symbols[i].name, ctx.symbols[i].name, 63);
      result.symbols[i].name[63] = '\0';
      
      // Adjust symbol addresses to physical addresses
      if (ctx.symbols[i].address >= ctx.text_base && 
          ctx.symbols[i].address < ctx.text_base + ctx.text_count * 4) {
        // Text symbol
        uint32_t text_offset = ctx.symbols[i].address - ctx.text_base;
        result.symbols[i].address = ctx.allocated_text_addr + text_offset;
      } else if (ctx.symbols[i].address >= ctx.data_base &&
                 ctx.symbols[i].address < ctx.data_base + ctx.data_segment.size) {
        // Data symbol
        uint32_t data_offset = ctx.symbols[i].address - ctx.data_base;
        result.symbols[i].address = ctx.allocated_data_addr + data_offset;
      } else {
        result.symbols[i].address = ctx.symbols[i].address;
      }
      
      result.symbols[i].is_global = ctx.symbols[i].is_global;
      result.symbols[i].is_procedure = ctx.symbols[i].is_procedure;
    }
  }

  result.success = 1;
  return result;
}

void free_program(AssemblyResult *result) {
  if (result->symbols) {
    free(result->symbols);
    result->symbols = NULL;
  }
  if (result->program) {
    free(result->program);
    result->program = NULL;
  }
}

int get_symbol_by_name(const AssemblyResult *result, const char *name) {
  for (int i = 0; i < result->symbol_count; i++) {
    if (strcmp(result->symbols[i].name, name) == 0) {
      return result->symbols[i].address;
    }
  }
  return -1;
}


void print_assembly_result(const AssemblyResult *result) {
  if (!result->success) {
    printf("Assembly failed: %s\n", result->error_message);
    return;
  }

  printf("\n=== Process Image: %s ===\n", result->program->program_name);
  printf("Entry Point:    0x%08x\n", result->program->entry_point);
  printf("Text Segment:   0x%08x - 0x%08x (%d bytes)\n",
      result->program->text_start,
      result->program->text_start + result->program->text_size,
      result->program->text_size);
  printf("Data Segment:   0x%08x - 0x%08x (%d bytes)\n",
      result->program->data_start,
      result->program->data_start + result->program->data_size,
      result->program->data_size);
  printf("Stack Pointer:  0x%08x\n", result->program->stack_ptr);
  printf("Global Pointer: 0x%08x\n", result->program->globl_ptr);

  printf("\n=== Symbol Table (%d symbols) ===\n", result->symbol_count);
  for (int i = 0; i < result->symbol_count; i++) {
    printf("%-30s 0x%08x %s%s\n",
        result->symbols[i].name,
        result->symbols[i].address,
        result->symbols[i].is_global ? "GLOBAL " : "",
        result->symbols[i].is_procedure ? "PROC" : "");
  }
}
