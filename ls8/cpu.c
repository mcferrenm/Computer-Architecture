#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define DATA_LEN 6

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *filename)
{
  FILE *fp;
  char line[1024];  
  int address = 0;

  // Get a file handle
  fp = fopen(filename, "r");

  // Error check for file
  if (fp == NULL) {
    fprintf(stderr, "file not found\n");
    exit(1);
  }

  while (fgets(line, 1024, fp) != NULL) {
    char *endptr;

    // Parse line for number, copy first non number char to endptr  
    unsigned char v = strtoul(line, &endptr, 2);

    // If the error begins at first char, then ignore it (instructions always left aligned)
    if (endptr == line) {
      continue;
    }
    
    // load bytes into ram line by line
    cpu_ram_write(cpu, v, address++);
  }
  // Close at EOL
  fclose(fp);
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char reg_a, unsigned char reg_b)
{ 
  int reg_index;
  unsigned char value;

  switch (op) {
    case ALU_MUL:
      // Get index of register b
      reg_index = reg_b & 0b00000111;

      // Set to temp variable
      value = cpu->registers[reg_index];

      // Get index of register a
      reg_index = reg_a & 0b00000111;

      // Update value * value at register a
      value = value * cpu->registers[reg_index];

      // Set value to register a
      cpu->registers[reg_index] = value;

      break;
  }
}

void handle_ldi(struct cpu *cpu, unsigned char operand_a, unsigned char operand_b)
{
  // Set register at first operand to value of second operand
  int reg_index = operand_a & 0b00000111;
  cpu->registers[reg_index] = operand_b;
}

void handle_prn(struct cpu *cpu, unsigned char operand_a)
{
  // Access register at first operand register, no second operand!  
  int reg_index = operand_a & 0b00000111;

  // Print value
  printf("%d\n", cpu->registers[reg_index]);
}

void handle_hlt(int *running)
{
  // Terminate
  *running = 0;
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  unsigned char ir;
  unsigned char operand_a;
  unsigned char operand_b;
  int op_count;

  while (running) {
    // 1. Get the value of the current instruction (in address PC).
    ir = cpu_ram_read(cpu, cpu->pc);

    // 2. Figure out how many operands this next instruction requires
    op_count = ir >> 6;

    // 3. Get the appropriate value(s) of the operands following this instruction
    operand_a = cpu_ram_read(cpu, cpu->pc + 1);
    operand_b = cpu_ram_read(cpu, cpu->pc + 2);

    // 4. switch() over it to decide on a course of action.
    switch(ir) {
      case LDI:
        handle_ldi(cpu, operand_a, operand_b);
        break;

      case PRN:
        handle_prn(cpu, operand_a);
        break;

      case MUL:
        // Use alu helper function to mult operand_a and operand_b 
        // and store result in operand_a (reg A)
        alu(cpu, ALU_MUL, operand_a, operand_b);
        break;

      case HLT:
        handle_hlt(&running);
        break;

      default:
        // For debugging
				printf("Unknown instruction %02x at address %02x\n", ir, cpu->pc);
        exit(1);
    }
    // Advance the program counter
    cpu->pc += op_count + 1;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // Initialize Program Counter to 0
  cpu->pc = 0;

  // Set all register and ram array bytes to 0 bits
  memset(cpu->registers, 0, sizeof(cpu->registers));
  memset(cpu->ram, 0, sizeof(cpu->ram));
}

/**
 * Read from RAM
 */
unsigned char cpu_ram_read(struct cpu *cpu, int index)
{ 
  return cpu->ram[index];
}

/**
 * Write to RAM
 */
void cpu_ram_write(struct cpu *cpu, unsigned char value, int index)
{
  cpu->ram[index] = value;
}
