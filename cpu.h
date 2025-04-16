#include "parser.h"

typedef struct {
    MemoryHandler* memory_handler; // Gestionnaire de memoire
    HashMap* context; // Registres (AX, BX, CX, DX, IP, ZF, SF)
    HashMap* constant_pool; // Table de hachage pour stocker les valeurs immediates
} CPU ;

CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char *segment_name, int pos);
void allocate_variables(CPU* cpu, Instruction**data_instructions, int data_count);
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count);
void print_data_segment(CPU *cpu);
int find_first_free_segment (MemoryHandler* handler, int size);
