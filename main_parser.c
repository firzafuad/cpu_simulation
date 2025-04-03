#include "parser.h"
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
			printf("%s %d\n", p->memory_locations->table[i].key, i);
		}
	}
	
	int code = p->code_count;
	printf("\ncode instructions : \n");
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
	
	free_parser_result(p);
	return 0;
}
