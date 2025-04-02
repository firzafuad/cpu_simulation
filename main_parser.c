#include "parser.h"
#include <stdio.h>

int main() {
	ParserResult * p = parse("test.txt");
	int data = p->data_count;
	printf("data instructions : \n");
	for (int i = 0; i < data; i++) {
		printf("%s %s %s\n", p->data_instructions[i]->mnemonic, p->data_instructions[i]->operand1, p->data_instructions[i]->operand2);
	}
	
	int code = p->code_count;
	printf("code instructions : \n");
	for (int i = 0; i < code; i++) {
		printf("%s %s %s\n", p->code_instructions[i]->mnemonic, p->code_instructions[i]->operand1, p->code_instructions[i]->operand2);
	}
	
	return 0;
}
