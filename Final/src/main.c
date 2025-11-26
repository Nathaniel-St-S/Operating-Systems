#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/assembler.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_PROGRAMS 10

AssemblyResult programs[MAX_PROGRAMS];

void initialize(){
  init_cpu();
  init_memory();
}

void free_all(){
  for(int i = 0; i < MAX_PROGRAMS; i++){
    free_program(&programs[i]);
  }
  free_memory();
}

int main(int argc, char *argv[])
{
  initialize();
  // Write main code here


  // Free up everything.
  // Dont delete this
  // No memory leaks
  free_all();
  return 0;
}
