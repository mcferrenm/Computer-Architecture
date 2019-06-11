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
    cpu->ram[address++] = v;
  }
  // Close at EOL
  fclose(fp);
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      // TODO
      break;

    // TODO: implement more ALU ops
  }
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
  int reg_index;
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
        // Set register at first operand to value of second operand
        reg_index = operand_a & 0b00000111;
        cpu->registers[reg_index] = operand_b;

        break;

      case PRN:
        // Access register at first operand register, no second operand!  
        reg_index = operand_a & 0b00000111;

        // Print value
        printf("%d\n", cpu->registers[reg_index]);

        break;

      case HLT:
        // Terminate
        running = 0;
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
