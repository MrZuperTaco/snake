#include "circular.h"
#include <stdlib.h>

typedef struct CircularUInt32 {
	uint32_t *array;
	uint64_t capacity;
	uint64_t length;
	uint64_t index_0;
} CircularUInt32;

CircularUInt32 *circular_uint32_create(uint64_t capacity) {
	if (capacity == 0) {
		return NULL;
	}
	
	uint64_t sizeof_circular = sizeof(CircularUInt32);
	void *circular_alloc = malloc(sizeof_circular + (sizeof(uint32_t) * capacity));

	if (circular_alloc == NULL) {
		return NULL;
	}
	
	CircularUInt32 *circular = (CircularUInt32 *)circular_alloc;
	circular->array = circular_alloc + sizeof_circular;
	circular->capacity = capacity;
	circular->length = 0;
	circular->index_0 = 0;

	return circular;
}

void circular_uint32_destroy(CircularUInt32 *circular) {
	free(circular);
}

uint64_t circular_uint32_length(CircularUInt32 *circular) {
	return circular->length;
}

uint64_t circular_uint32_capacity(CircularUInt32 *circular) {
	return circular->capacity;
}

int32_t circular_uint32_is_empty(CircularUInt32 *circular) {
	return circular->length == 0;
}

int32_t circular_uint32_is_full(CircularUInt32 *circular) {
	return circular->length == circular->capacity;
}

static inline uint64_t index_of(CircularUInt32 *circular, uint64_t index) {
	return (circular->index_0 + index) % circular->capacity;	
}

uint32_t circular_uint32_get(CircularUInt32 *circular, uint64_t index) {
	return circular->array[index_of(circular, index)];
}

static int32_t resize_if_full(CircularUInt32 **circular_ptr) {
	CircularUInt32 *circular = *circular_ptr;

	if (!circular_uint32_is_full(circular)) {
		return 0;
	}
	
	CircularUInt32 *new_circular = circular_uint32_create(circular->capacity * 2);

	if (new_circular == NULL) {
		return 1;
	}

	for (uint64_t i = 0; i < circular->length; ++i) {
		new_circular->array[i] = circular_uint32_get(circular, i);
	}
	new_circular->length = circular->length;
		
	circular_uint32_destroy(circular);
	*circular_ptr = new_circular;

	return 0;
}

int32_t circular_uint32_push_head(CircularUInt32 **circular_ptr, uint32_t item) {
	if (resize_if_full(circular_ptr)) {
		return 1;
	}
	CircularUInt32 *circular = *circular_ptr;

	int64_t new_index_0 = circular->index_0 == 0 ? circular->capacity - 1 : circular->index_0 - 1;
	circular->array[new_index_0] = item;
	circular->index_0 = new_index_0;
	circular->length++;

	return 0;
}

int32_t circular_uint32_push_tail(CircularUInt32 **circular_ptr, uint32_t item) {
	if (resize_if_full(circular_ptr)) {
		return 1;	
	}
	CircularUInt32 *circular = *circular_ptr;

	circular->array[index_of(circular, circular->length)] = item;
	circular->length++;

	return 0;
}

uint32_t circular_uint32_pop_head(CircularUInt32 *circular) {
	if (circular_uint32_is_empty(circular)) {
		return 0;
	}
	
	uint32_t pop = circular->array[circular->index_0];
	circular->index_0 = index_of(circular, 1);
	circular->length--;	

	return pop;
}

uint32_t circular_uint32_pop_tail(CircularUInt32 *circular) {
	if (circular_uint32_is_empty(circular)) {
		return 0;
	}
	
	uint32_t pop = circular->array[index_of(circular, circular->length - 1)];
	circular->length--;	

	return pop;	
}

void circular_uint32_clear(CircularUInt32 *circular) {
	circular->length = 0;
	circular->index_0 = 0;
}
