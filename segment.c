#include "segment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


MemoryHandler *memory_init(int size) {
	MemoryHandler *mh = (MemoryHandler*)malloc(sizeof(MemoryHandler));
	mh->total_size = size;
	mh->memory = malloc(sizeof(void*) * size);
	for (int i = 0; i < size; i++) mh->memory[i] = NULL;
	mh->free_list = (Segment*)malloc(sizeof(Segment));
	mh->free_list->start = 0;
	mh->free_list->size = size;
	mh->free_list->next = NULL;
	mh->allocated = hashmap_create();
	
	return mh;
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev) {
	Segment *seg = handler->free_list;
	while(seg) {
		if (seg->start <= start && seg->start+seg->size >= start+size) break;
		*prev = seg;
		seg = seg->next;
	}
	
	return seg;
}


int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
	Segment *prev = NULL;
	Segment *seg = find_free_segment(handler, start, size, &prev);
	if (!seg) return 0;
	
	// Initialiser le segment
	Segment* new_seg = (Segment*)malloc(sizeof(Segment));
	if (!new_seg) return 0;
	new_seg->start = start;
	new_seg->size = size;
	new_seg->next = NULL;
	hashmap_insert(handler->allocated, name, new_seg);
	
	int end = start + size;
	int segEnd = seg->start + seg->size;
	if (start == seg->start) {
		// Cas 1 : Le debut du segment est le debut d'un segment libre
		if (end == segEnd) {
			// Cas 1.1 : le segment est exactement égal à un segment libre
			if (prev) prev->next = seg->next;
			else handler->free_list = seg->next;
			free(seg);
		} else {
			// Cas 1.2 : le segment est plus petit que le segment libre
			seg->start = end;
			seg->size = segEnd - end;
		}
	} else {
		// Cas 2 : Le debut du segment est au milieu d'un segment libre
		if (end == segEnd) {
			// Cas 2.1 : le segment se situe a la fin d'un segment libre
			seg->size = start - seg->start;
		} else {
			// Cas 2.2 : la fin de segment est au milieu d'un segment libre
			Segment* next_seg = (Segment*)malloc(sizeof(Segment));
			if (!next_seg) return 0;
			next_seg->start = end;
			next_seg->size = segEnd - end;
			next_seg->next = seg->next;
			seg->next = next_seg;
		}
	}
	return 1;
}

int remove_segment(MemoryHandler* handler, const char *name) {
	Segment* seg = (Segment*)hashmap_get(handler->allocated, name);
	if (seg == NULL) {
		///segment n'est pas trouvé
		return 0;
	}
	Segment *ls = handler->free_list;
	
	Segment *prev = NULL;
	while(ls != NULL && (ls->start + ls->size) <= seg->start) {
		prev= ls;
		ls = ls->next;
	}
	if (ls == NULL) {
		// Cas 1 : le segment est à la fin de la liste
		if ( (prev->start + prev->size) == seg->start) {
			// Cas 1.1 : le segment est adjacent au segment libre
			prev->size += seg->size;
		} else {
			// Cas 1.2 : le segment n'est pas adjacent au segment libre
			prev->next = seg;
		}
	} else if (prev == NULL) {
		// Cas 2 : le segment est au debut de la liste
		if((seg->start + seg->size) == ls->start) {
			// Cas 2.1 : le segment est adjacent au segment libre
			seg->size += ls->size;
			seg->next = ls->next;
			free(ls);
		} else {
			// Cas 2.2 : le segment n'est pas adjacent au segment libre
			seg->next = ls;
			handler->free_list = seg;
		}
	} else {
		// Cas 3 : le segment est au milieu de la liste
		if ( (prev->start + prev->size) == seg->start) {
			// Cas 3.1 : le segment est adjacent au segment libre precedent
			prev->size += seg->size;
			if ( (prev->start + prev->size) == ls->start) {
				// Cas 3.1.1 : le segment est adjacent au segment libre suivant
				prev->size += ls->size;
				prev->next = ls->next;
				free(ls);
			}
			free(seg);
		} else {
			// Cas 3.2 : le segment n'est pas adjacent au segment libre precedent
			if ((seg->start+seg->size == ls->start)) {
				// Cas 3.2.1 : le segment est adjacent au segment libre suivant
				ls->start = seg->start;
				ls->size += seg->size;
				free(seg);
			} else {
				// Cas 3.2.2 : le segment n'est pas adjacent au segment libre suivant
				prev->next = seg;
				seg->next = ls;
			}
		}
	}
	return 1;		
}
