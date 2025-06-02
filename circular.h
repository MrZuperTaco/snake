#ifndef CIRCULAR_H
#define CIRCULAR_H

#include <stdint.h>

typedef struct CircularUInt32 CircularUInt32;

CircularUInt32 *circular_uint32_create(uint64_t capacity);
void circular_uint32_destroy(CircularUInt32 *circular);

uint64_t circular_uint32_length(CircularUInt32 *circular);
uint64_t circular_uint32_capacity(CircularUInt32 *circular);

int32_t circular_uint32_is_empty(CircularUInt32 *circular);
int32_t circular_uint32_is_full(CircularUInt32 *circular);

uint32_t circular_uint32_get(CircularUInt32 *circular, uint64_t index);

int32_t circular_uint32_push_head(CircularUInt32 **circular_ptr, uint32_t item);
int32_t circular_uint32_push_tail(CircularUInt32 **circular_ptr, uint32_t item);

uint32_t circular_uint32_pop_head(CircularUInt32 *circular);
uint32_t circular_uint32_pop_tail(CircularUInt32 *circular);

void circular_uint32_clear(CircularUInt32 *circular);

#endif
