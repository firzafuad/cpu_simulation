#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

CPU* cpu_init(int memory_size) {
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (!cpu) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();
	cpu->constant_pool = hashmap_create();

    hashmap_insert(cpu->context, "AX", (void*) ((int*)malloc(sizeof(int))));
    hashmap_insert(cpu->context, "BX", (void*) ((int*)malloc(sizeof(int))));
    hashmap_insert(cpu->context, "CX", (void*) ((int*)malloc(sizeof(int))));
    hashmap_insert(cpu->context, "DX", (void*) ((int*)malloc(sizeof(int))));
    
    return cpu;
}

void cpu_destroy(CPU* cpu) {
    if (!cpu) return;
    hashmap_destroy(cpu->context);
	hashmap_destroy(cpu->constant_pool);
    Segment* seg = cpu->memory_handler->free_list;
    while (seg) {
        Segment* next = seg->next;
        free(seg);
        seg = next;
    }
    for (int i = 0; i < cpu->memory_handler->total_size; i++) {
        if (cpu->memory_handler->memory[i]) {
            free(cpu->memory_handler->memory[i]);
        }
    }
    hashmap_destroy(cpu->memory_handler->allocated);
	free(cpu->memory_handler->memory);
    free(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data) {
    Segment* seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (!seg || pos < 0 || pos > seg->size) {
        printf("Invalid segment or position\n");
        return NULL;
    }
    handler->memory[seg->start + pos] = data;
    return data;
}

void* load(MemoryHandler *handler, const char *segment_name, int pos) {
    Segment* seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (!seg || pos < 0 || pos >= seg->size) {
        printf("Invalid segment or position\n");
        return NULL;
    }
    return handler->memory[seg->start + pos];
}

void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count) {
	int total = 0;
	for (int i = 0; i < data_count; i++) {
		int count = 0;
		char *values = data_instructions[i]->operand2;
		char tabValues[256];
		strcpy(tabValues, values);
		char *token = strtok(tabValues, ",");
		while (token != NULL ) {
			count++;
			token = strtok(NULL, ",");
		}
		total += count;
	}
	
	int start = -1;
	Segment *seg = cpu->memory_handler->free_list;
	while (seg != NULL) {
		if (seg->size >= total) {
			start = seg->start;
			break;
		}
		seg = seg->next;
	}
	if (start == -1) {
		printf("no space available\n");
		return ;
	}
	int res = create_segment(cpu->memory_handler, "DS", start, total);
	if (res == 0) {
		printf("error creating segment\n");
		return ;
	}
	int j = 0;
	for (int i = 0; i < data_count; i++) {
		char *values = data_instructions[i]->operand2;
		char tabValues[256];
		strcpy(tabValues, values);
		char *token = strtok(tabValues, ",");
		while (token != NULL ) {
			int *data = (int*)malloc(sizeof(int));
			*data = atoi(token);
			store(cpu->memory_handler, "DS", j++, (void*)data);
			token = strtok(NULL, ",");
		}
	}
}

void print_data_segment(CPU *cpu) {
	Segment *seg = hashmap_get(cpu->memory_handler->allocated, "DS");
	if (seg == NULL) {
		printf("No data segment allocated\n");
		return ;
	}
	printf("memory location : (%d, %d)\n", seg->start, seg->size);
	for (int i = 0; i < seg->size; i++) {
		void *val = load(cpu->memory_handler, "DS", i);
		if (val)
			printf("[%d] : %d\n", i,  *(int*)val);
		else
			printf("[%d] : NULL\n", i);
	}
}


