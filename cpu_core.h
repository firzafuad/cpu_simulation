#include "cpu.h"

void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void *resolve_addressing(CPU *cpu, const char *operand);

int matches(const char* pattern, const char* string);
void handle_MOV(CPU* cpu, void* src, void* dest);
CPU * setup_test_environment();

char *trim(char *str);
int search_and_replace (char **str, HashMap *values);

int resolve_constants(ParserResult *result);
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest);
int execute_instruction(CPU *cpu, Instruction *instr);
Instruction* fetch_next_instruction(CPU *cpu);
int run_program(CPU *cpu);