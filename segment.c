#include "segment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


MemoryHandler *memory_init(int size) {
	MemoryHandler *mh = (MemoryHandler*)malloc(sizeof(MemoryHandler));
	mh->total_size = size;
	mh->memory = malloc(sizeof(Segment*) * size);
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
	
	Segment* new_seg = (Segment*)malloc(sizeof(Segment));
	if (!new_seg) return 0;
	new_seg->start = start;
	new_seg->size = size;
	new_seg->next = NULL;
	hashmap_insert(handler->allocated, name, new_seg);
	
	int end = start + size;
	int segEnd = seg->start + seg->size;
	if (start == seg->start) {
		if (end == segEnd) {
			if (prev) prev->next = seg->next;
			else handler->free_list = seg->next;
			free(seg);
		} else {
			seg->start = end;
			seg->size = segEnd - end;
		}
	} else {
		if (end == segEnd) {
			seg->size = start - seg->start;
		} else {
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
		if ( (prev->start + prev->size) == seg->start) {
			prev->size += seg->size;
		} else {
			prev->next = seg;
		}
	} else if (prev == NULL) {
		if((seg->start + seg->size) == ls->start) {
			seg->size += ls->size;
			seg->next = ls->next;
			free(ls);
		}
	} else {	
		if ( (prev->start + prev->size) == seg->start) {
			prev->size += seg->size;
			if ( (prev->start + prev->size) == ls->start) {
				prev->size += ls->size;
				prev->next = ls->next;
				free(ls);
			}
			free(seg);
		} else {
			if ((seg->start+seg->size == ls->start)) {
				ls->start = seg->start;
				ls->size += seg->size;
				free(seg);
			} else {
				prev->next = seg;
				seg->next = ls;
			}
		}
	}
	return 1;		
}
