#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

/**
 * Main
 */
int main(int argc, char *argv[])
{
  struct cpu cpu;
  
  // Check for two arguments
  if (argc != 2) {
    fprintf(stderr, "usage: comp filename\n");
    exit(1);
  }

  cpu_init(&cpu);
  cpu_load(&cpu, argv[1]);
  cpu_run(&cpu);

  return 0;
}