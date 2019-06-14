#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define DATA_LEN 6
#define SP 7

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
void alu(struct cpu *cpu, enum alu_op op, unsigned char operand_a, unsigned char operand_b)
{ 
  unsigned char value;

  // TODO Clean up cpu->registers[operand_a]

  switch (op) {
    case ALU_MUL:
      // Set to temp variable
      value = cpu->registers[operand_b];

      // Update value * value at register a
      value = value * cpu->registers[operand_a];

      // Set value to register a
      cpu->registers[operand_a] = value;
      break;
    
    case ALU_ADD:
      // Set to temp variable
      value = cpu->registers[operand_b];

      // Update value * value at register a
      value = value + cpu->registers[operand_a];

      // Set value to register a
      cpu->registers[operand_a] = value;
      break;
    
    case ALU_CMP:    
      // Compare two values
      if (cpu->registers[operand_a] == cpu->registers[operand_b]) {

        // Set EQ flag 0b00000001
        cpu->fl ^= 0b00000001;

      } else if (cpu->registers[operand_a] > cpu->registers[operand_b]) {

        // Set greater than flag 0b00000001
        cpu->fl ^= 0b00000010;

      } else {
        
        // Set less than flag 0b00000001
        cpu->fl ^= 0b00000100;

      }
      break;
  }
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

/**
 * Instruction Helper functions
 */

void handle_push(struct cpu *cpu, unsigned char operand_a)
{
  // !!!TODO HANDLE ERRORS FOR OVER/UNDER FLOWS
  
  // Decrement stack pointer
  cpu->registers[SP]--;

  // Grab value from register index operand_a
  int value = cpu->registers[operand_a];

  // Push value on stack
  cpu_ram_write(cpu, value, cpu->registers[SP]);
}

void handle_pop(struct cpu *cpu, unsigned char operand_a)
{
  // !!!TODO HANDLE ERRORS FOR OVER/UNDER FLOWS

  // Read ram from stack pointer
  int value = cpu_ram_read(cpu, cpu->registers[SP]);
  
  // Store value in register index operand_a
  cpu->registers[operand_a] = value;

  // Increment stack pointer
  cpu->registers[SP]++;
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

void handle_call(struct cpu *cpu, unsigned char operand_a)
{ 
  // // Decrement stack pointer
  cpu->registers[SP]--;

  // // Push instruction directly after call onto the stack
  cpu_ram_write(cpu, cpu->pc + 2, cpu->registers[SP]);

  // Set pc to value from register index operand_a
  cpu->pc = cpu->registers[operand_a];

}

void handle_ret(struct cpu *cpu)
{
  // Pop the instruction count from the top of the stack and store it in the pc
  cpu->pc = cpu_ram_read(cpu, cpu->registers[SP]);

  // Increment stack pointer
  cpu->registers[SP]++;
}

void handle_hlt(int *running)
{
  // Terminate
  *running = 0;
}

void handle_jeq(struct cpu *cpu, unsigned char operand_a, int op_count)
{
  // Check if EQ flag 0b00000001 is set
  if (cpu->fl & 0b00000001) {
    
    // Set pc to value from register index operand_a
    cpu->pc = cpu->registers[operand_a];

  } else {

    // Manually set the cpu->pc
    cpu->pc += op_count + 1;
  }
}

void handle_jne(struct cpu *cpu, unsigned char operand_a, int op_count)
{
  // Check if EQ flag 0b00000001 is NOT set
  if (!(cpu->fl & 0b00000001)) {
    
    // Set pc to value from register index operand_a
    cpu->pc = cpu->registers[operand_a];

  } else {

    // Manually set the cpu->pc
    cpu->pc += op_count + 1;
  }
}

void handle_jmp(struct cpu *cpu, unsigned char operand_a)
{
  // Set pc to value from register index operand_a
  cpu->pc = cpu->registers[operand_a];
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

    // Trace Debug
    printf("TRACE: PC#:%d INSTR:%02X OP A:%02X OP B:%02X\n", cpu->pc, ir, cpu->ram[cpu->pc+1], cpu->ram[cpu->pc+2]);

    // 2. Figure out how many operands this next instruction requires
    op_count = ir >> 6;

    // 3. Get the appropriate value(s) of the operands following this instruction
    operand_a = cpu_ram_read(cpu, cpu->pc + 1);
    operand_b = cpu_ram_read(cpu, cpu->pc + 2);

    // 4. switch() over it to decide on a course of action.
    switch(ir) {
      case CALL:
        handle_call(cpu, operand_a);
        break;

      case RET:
        handle_ret(cpu);
        break;

      case PUSH:
        handle_push(cpu, operand_a);
        break;

      case POP:
        handle_pop(cpu, operand_a);  
        break;

      case LDI:
        handle_ldi(cpu, operand_a, operand_b);
        break;

      case PRN:
        handle_prn(cpu, operand_a);
        break;

      case MUL:
        alu(cpu, ALU_MUL, operand_a, operand_b);
        break;

      case ADD:
        alu(cpu, ALU_ADD, operand_a, operand_b);
        break;

      case CMP:
        alu(cpu, ALU_CMP, operand_a, operand_b);
        break;

      case JEQ:
        handle_jeq(cpu, operand_a, op_count);
        
        // Reset the flags
        cpu->fl = 0;
        break;

      case JNE:
        handle_jne(cpu, operand_a, op_count);

        // Reset the flags
        cpu->fl = 0;
        break;

      case JMP:
        handle_jmp(cpu, operand_a);
        break;

      case HLT:
        handle_hlt(&running);
        break;

      default:
        // For debugging
				printf("Unknown instruction %02x at address %02x\n", ir, cpu->pc);
        exit(1);
    }
    // Check if ir sets the PC directly, advance the program counter accordingly
    if (!(ir & SETS_PC_DIRECT)) {
      cpu->pc += op_count + 1;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // Initialize Program Counter to 0
  cpu->pc = 0;
  
  // Initialize Flags to 0
  cpu->fl = 0;

  // Set all register and ram array bytes to 0 bits
  memset(cpu->registers, 0, sizeof(cpu->registers));
  memset(cpu->ram, 0, sizeof(cpu->ram));

  // Stack Pointer points to F4 or 244
  cpu->registers[SP] = 244; 
}
