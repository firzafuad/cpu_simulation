#include "parser.h"

typedef struct {
    MemoryHandler * memory_handler ; // Gestionnaire de memoire
    HashMap * context ; // Registres (AX, BX, CX, DX)
} CPU ;

CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char *segment_name, int pos);
void allocate_variables(CPU* cpu, Instruction**data_instructions, int data_count);
void print_data_segment(CPU *cpu);
