#include <stdio.h>
#include "cpu.h"

CPU* cpu_init(int memory_size) {
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (!cpu) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();
    return cpu;
}

void cpu_destroy(CPU* cpu) {
    if (!cpu) return;
    hashmap_destroy(cpu->context);
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
    free(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data) {
    Segment* seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (!seg || pos < 0 || pos >= seg->size) {
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