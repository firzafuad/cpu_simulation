#include "exercice1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TOMBSTONE (( void *) -1)
#define TABLE_SIZE 200


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
	unsigned long idx = simple_hash(key);
	//parcourir la table de hachage a partir de l'indice idx jusqu'a ce qu'on trouve une case vide ou une case avec la valeur TOMBSTONE
	for (int i = 0; i < map->size; i++) {
		if(idx >= map->size) {
			idx=0;
		}
		if (map->table[idx].value == NULL || map->table[idx].value == TOMBSTONE) {
			map->table[idx].key = strdup(key);
			map->table[idx].value = value;
			return 1;
		}
		idx++;
	}
	return 0; // Hashmap est pleine
}

void* hashmap_get(HashMap *map, const char *key) {
	unsigned long idx = simple_hash(key);
	for (int i = 0; i < map->size; i++) {
		if(idx >= map->size) {
			idx=0;
		}
		if (map->table[idx].key && strcmp(map->table[idx].key, key) == 0) {
			return map->table[idx].value;
		}
		if (map->table[idx].value == NULL) break; // Arrêter si on trouve une case vide
		idx++;
	}
	return NULL;
}

int hashmap_remove(HashMap *map, const char *key) {
	unsigned long idx = simple_hash(key);

	for (int i = 0; i < map->size; i++) {
        if(idx >= map->size) {
			idx=0;
		}
        HashEntry *entry = &map->table[idx];
        
        if (!entry->key) {
            if (entry->value != TOMBSTONE) {
				fprintf(stderr, "Key not found: %s\n", key);
				return 0;
			}
        } else if (strcmp(entry->key, key) == 0) {
            // Free key and mark as TOMBSTONE
            free(entry->key);
            entry->key = NULL;
            entry->value = TOMBSTONE;
            map->size--;
            return 1;
        }
        idx++;
    }
	
	return 0;
}

void hashmap_destroy(HashMap* map) {
	for (int i = 0; i < map->size; i++) {
		if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
			free(map->table[i].key);
			free(map->table[i].value);
		}
	}
	free(map->table);
	free(map);
}
