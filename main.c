#include "cpu_core.h"
#include <stdio.h>

int main() {
	ParserResult * p = parse("test.txt");
	int data = p->data_count;
	printf("data instructions : \n");
	for (int i = 0; i < data; i++) {
		printf("%s %s %s\n", p->data_instructions[i]->mnemonic, p->data_instructions[i]->operand1, p->data_instructions[i]->operand2);
	}

	printf("\nmemory locations : \n");
	for (int i = 0; i < p->memory_locations->size; i++)
	{
		if (p->memory_locations->table[i].key != NULL) {
			printf("%s %d\n", p->memory_locations->table[i].key, *(int *)(p->memory_locations->table[i].value));
		}
	}
	
	int code = p->code_count;
	printf("\ncode instructions (avant): \n");
	for (int i = 0; i < code; i++) {
		printf("%s %s %s\n", p->code_instructions[i]->mnemonic, p->code_instructions[i]->operand1, p->code_instructions[i]->operand2);
	}
	resolve_constants(p);
	printf("code instructions (après): \n");
	for (int i = 0; i < code; i++) {
		printf("%s %s %s\n", p->code_instructions[i]->mnemonic, p->code_instructions[i]->operand1, p->code_instructions[i]->operand2);
	}

	printf("\nlabels : \n");
	for (int i = 0; i < p->labels->size; i++)
	{
		if (p->labels->table[i].key != NULL) {
			printf("%s %d\n", p->labels->table[i].key, *(int*)p->labels->table[i].value);
		}
	}

	CPU *cpu = cpu_init(1024);
	int* ax = (int *)hashmap_get(cpu->context, "AX");
    int* bx = (int *)hashmap_get(cpu->context, "BX");
    int* cx = (int *)hashmap_get(cpu->context, "CX");
    int* dx = (int *)hashmap_get(cpu->context, "DX");
    *ax = 3;
    *bx = 6;
    *cx = 1;
    *dx = 0;
	allocate_variables(cpu, p->data_instructions, p->data_count);
	allocate_code_segment(cpu, p->code_instructions, p->code_count);

	printf("\nrunning program...\n");
	run_program(cpu);
	
	cpu_destroy(cpu);
	free_parser_result(p);
	return 0;
}
