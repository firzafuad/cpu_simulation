#include "cpu.h"

void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void *resolve_addressing(CPU *cpu, const char *operand);

int matches(const char* pattern, const char* string);
void handle_MOV(CPU* cpu, void* src, void* dest);
CPU * setup_test_environment();