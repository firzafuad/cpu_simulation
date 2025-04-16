#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

//initialiser une variable static pour stocker l'adresse mémoire
static int current_address = 0;

Instruction *parse_data_instruction(const char *line, HashMap* memory_locations) {
	char mnemonic[256], operand1[256], operand2[256];
	if (sscanf(line, "%s %s %s", mnemonic, operand1, operand2) != 3) {
		fprintf(stderr, "Invalid data instruction format: %s\n", line);
		return NULL;
	}

	Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
	inst->mnemonic = strdup(mnemonic);
	inst->operand1 = strdup(operand1);
	inst->operand2 = strdup(operand2);
	
	//calculer le nombre de valeur en operand2
	int count = 0;
	char *token = strtok(operand2, ",");
	while (token != NULL) {
		count++;
		token = strtok(NULL, ",");
	}
	//ajouter l'adresse memoire à la hashmap
	int *addr = (int*)malloc(sizeof(int));
	*addr = current_address;
	hashmap_insert(memory_locations, inst->mnemonic, (void *)addr);
	
	current_address += count; //mise à	jour de la variable static
	return inst;
}
	

Instruction *parse_code_instruction(const char *line, HashMap* labels, int code_count) {
	char mnemonic[256], operand[256], label[256];
	char* token;
	Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
	if (sscanf(line, "%s %s %s", label, mnemonic, operand) != 3) { //verifier si il y a un label
		if (sscanf(line, "%s %s", mnemonic, operand) != 2) { //si ce n'est pas le cas, rescanner sans label
			if ((sscanf(line, "%s", mnemonic) != 2) == 1) {
				inst->mnemonic = strdup(mnemonic);
				inst->operand1 = strdup("");
				inst->operand2 = strdup("");
				return inst;
			}
			free(inst);
			fprintf(stderr, "Invalid code instruction format: %s\n", line);
			return NULL;
		}
	} else {
		label[strcspn(label, ":")] = '\0'; // supprimer le ":" à la fin du label
		int *count = (int*)malloc(sizeof(int));
		*count = code_count;
		hashmap_insert(labels, label, count); //si label est trouvé, l'ajouter à la hashmap
	}
	if (!inst) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	inst->mnemonic = strdup(mnemonic);

	token = strtok(operand, ",");
	inst->operand1 = strdup(token);
	
	if ((token = strtok(NULL, ",")) != NULL) {
		inst->operand2 = strdup(token);
	} else {
		inst->operand2 = strdup("");
	}
	
	return inst;
}

ParserResult *parse(const char *filename) {
	FILE* f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Failed to open file\n");
		return NULL;
	}
	
	char buffer[256];
	if (!fgets(buffer, sizeof(buffer), f) || strcmp(buffer, ".DATA\n") != 0) {
		fclose(f);
		return NULL;
	}
	
	ParserResult *p = (ParserResult *)malloc(sizeof(ParserResult));
	if (!p) {
		fclose(f);
		return NULL;
	}
	int n = 50;
	p->data_instructions = (Instruction**)malloc(sizeof(Instruction*) * n);
	p->code_instructions = (Instruction**)malloc(sizeof(Instruction*) * n);
	if (!p->data_instructions || !p->code_instructions) {
		free(p);
		fclose(f);
		return NULL;
	}
	p->data_count = 0;
	p->code_count = 0;
	p->labels = hashmap_create();
	p->memory_locations = hashmap_create();
	
	// Lire les instructions de .DATA
	while (fgets(buffer, sizeof(buffer), f)) {
		buffer[strcspn(buffer, "\n")] = '\0'; // supprimer le retour à la ligne
		if (strcmp(buffer, ".CODE") == 0) break;
		Instruction *ins = parse_data_instruction(buffer, p->memory_locations);
		if (ins) {
			p->data_instructions[p->data_count++] = ins;
			// Redimensionner le tableau si le nombre d'instructions dépasse la capacité
			if (p->data_count >= n) {
				n *= 2;
				p->data_instructions = realloc(p->data_instructions, sizeof(Instruction*) * n);
				if (!p->data_instructions) {
					fclose(f);
					return NULL;
				}
			}
		}
	}
	
	n = 50; // Réinitialiser n pour le code
	// Lire les instructions de .CODE
	while (fgets(buffer, sizeof(buffer), f)) {
		buffer[strcspn(buffer, "\n")] = '\0'; // supprimer le retour à la ligne
		Instruction *ins = parse_code_instruction(buffer, p->labels, p->code_count);
		if (ins) {
			p->code_instructions[p->code_count++] = ins;
			// Redimensionner le tableau si le nombre d'instructions dépasse la capacité
			if (p->code_count >= n) {
				n *= 2;
				p->code_instructions = realloc(p->code_instructions, sizeof(Instruction*) * n);
				if (!p->code_instructions) {
					fclose(f);
					return NULL;
				}
			}
		}
	}
	// Redimensionner les tableaux à la taille finale pour éviter les fuites de mémoire
	p->data_instructions = realloc(p->data_instructions, sizeof(Instruction*) * (p->data_count));
	p->code_instructions = realloc(p->code_instructions, sizeof(Instruction*) * (p->code_count));

	fclose(f);
	return p;
}

void free_parser_result(ParserResult* p) {
	if (!p) return;
	for (int i = 0; i < p->data_count; i++) {
		free(p->data_instructions[i]->mnemonic);
		free(p->data_instructions[i]->operand1);
		free(p->data_instructions[i]->operand2);
		free(p->data_instructions[i]);
	}
	for (int i = 0; i < p->code_count; i++) {
		free(p->code_instructions[i]->mnemonic);
		free(p->code_instructions[i]->operand1);
		free(p->code_instructions[i]->operand2);
		free(p->code_instructions[i]);
	}
	hashmap_destroy(p->labels);
	hashmap_destroy(p->memory_locations);
	free(p->data_instructions);
	free(p->code_instructions);
	free(p);
}











