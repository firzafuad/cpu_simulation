#include "cpu.h"
#include <stdio.h>

int main() {
	ParserResult * p = parse("test.txt");
	int data = p->data_count;
	CPU *cpu = cpu_init(1024);
	for (int i = 0; i < data; i++) {
		printf("%s %s %s\n", p->data_instructions[i]->mnemonic, p->data_instructions[i]->operand1, p->data_instructions[i]->operand2);
	}
	
	allocate_variables(cpu, p->data_instructions, data);
	
	printf("data instructions : \n");
	for (int i = 0; i < data; i++) {
		printf("%s %s %s\n", p->data_instructions[i]->mnemonic, p->data_instructions[i]->operand1, p->data_instructions[i]->operand2);
	}
	print_data_segment(cpu);
	
	cpu_destroy(cpu);
	free_parser_result(p);
	return 0;
}
