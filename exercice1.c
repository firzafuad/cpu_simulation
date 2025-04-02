#include "exercice1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TOMBSTONE (( void *) -1)
#define TABLE_SIZE 128


unsigned long simple_hash(const char *str) {
	int i = 0, index = 0;
	while (str[i] != '\0') {
		index += str[i++];
	}
	return index%TABLE_SIZE;
}

HashMap* hashmap_create() {
	HashMap* h = (HashMap*)malloc(sizeof(HashMap));
	h->size = TABLE_SIZE;
	h->table = (HashEntry*)malloc(sizeof(HashEntry)*h->size);
	for(int i = 0; i < h->size; i++) {
		h->table[i].key = NULL;
		h->table[i].value = NULL;
	}
	return h;
}

int hashmap_insert(HashMap* map, const char *key, void* value) {
	unsigned long index = simple_hash(key);
	while (index < map->size*2 &&
		!(map->table[index%map->size].value == NULL ||
		map->table[index%map->size].value == TOMBSTONE)) {
		index += 1;
	}
	if (index > map->size*2) {
		return 0;
	}
	
	map->table[index].key = strdup(key);
	map->table[index].value = value;
	return 1;
}

void* hashmap_get(HashMap *map, const char *key) {
	unsigned long idx = simple_hash(key);
	if (map->table[idx].value != TOMBSTONE) {
		return map->table[idx].value;
	}
	return NULL;
}

int hashmap_remove(HashMap *map, const char *key) {
	unsigned long idx = simple_hash(key);
	if (map->table[idx].value != TOMBSTONE) {
		free(map->table[idx].value);
		free(map->table[idx].key);
		map->table[idx].value = TOMBSTONE;
		return 1;
	}
	
	return 0;
}

void hashmap_destroy(HashMap* map) {
	HashEntry* table = map->table;
	for (int i = 0; i < map->size; i++) {
		free(table[i].key);
		free(table[i].value);
	}
	free(table);
	free(map);
}
