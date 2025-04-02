#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

Instruction *parse_data_instruction(const char *line, HashMap* memory_locations) {
	char mnemonic[256], operand1[256], operand2[256];
	if (sscanf(line, "%s %s %s", mnemonic, operand1, operand2) != 3) {
		return NULL;
	}
	printf("ok\n");
	Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
	inst->mnemonic = strdup(mnemonic);
	inst->operand1 = strdup(operand1);
	inst->operand2 = strdup(operand2);
	char* separateur = ",";
	char *token = strtok(operand2, separateur);
	while (token != NULL) {
		hashmap_insert(memory_locations, inst->mnemonic, token);
		printf("ajout data ok\n");
		token = strtok(NULL, separateur);
	}
	return inst;
}
	

Instruction *parse_code_instruction(const char *line, HashMap* labels, int code_count) {
	Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
	char mnemonic[256], operand[256], label[256];
	char* separateur = ",", *token;
	
	if (sscanf(line, "%s: %s %s", label, mnemonic, operand) != 3) {
		sscanf(line, "%s %s", mnemonic, operand);
	} else {
		hashmap_insert(labels, label, (void*)&code_count);
		printf("ajout code ok\n");
	}
	inst->mnemonic = strdup(mnemonic);
	
	token = strtok(operand, separateur);
	inst->operand1 = strdup(token);
	
	if ((token = strtok(NULL, separateur) )!= NULL) {
		inst->operand2 = strdup(token);
	} else {
		char *no = "";
		inst->operand2 = strdup(no);
	}
	
	return inst;
}

ParserResult *parse(const char *filename) {
	FILE* f = fopen(filename, "r");
	
	char buffer[256];
	fscanf(f, "%s", buffer);
	if (strcmp(buffer, ".DATA") != 0) {
		fclose(f);
		return NULL;
	}
	ParserResult *p = (ParserResult *)malloc(sizeof(ParserResult));
	int n = 50;
	p->data_instructions = (Instruction**)malloc(sizeof(Instruction*)*n);
	p->data_count = 0;
	p->code_instructions = (Instruction**)malloc(sizeof(Instruction*)*n);
	p->code_count = 0;
	p->labels = hashmap_create();
	p->memory_locations = hashmap_create();
	
	fgets(buffer, 256, f);
	buffer[strlen(buffer)-1] = '\0';
	do {
		printf("%s\n", buffer);
		Instruction *ins = parse_data_instruction(buffer, p->memory_locations);
		p->data_instructions[p->data_count++] = ins;
		if(p->data_count >= n) {
			p->data_instructions = realloc(p->data_instructions, sizeof(Instruction*)*(p->data_count+1));
		}
		fgets(buffer, 256, f);
		int len = strlen(buffer);
		buffer[len-1] = '\0';
	} while (strcmp(buffer, ".CODE") != 0);
	
	while (fgets(buffer, 256, f)) {
		int len = strlen(buffer);
		buffer[len-1] = '\0';
		printf("%s\n", buffer);
		Instruction *ins = parse_code_instruction(buffer, p->labels, p->code_count);
		p->code_instructions[p->code_count++] = ins;
		if(p->code_count >= n) {
			p->code_instructions = realloc(p->code_instructions, sizeof(Instruction*)*(p->code_count+1));
		}
	}
	fclose(f);
	return p;
}


	
	
	
	
	
	
	
	
	
	
	
