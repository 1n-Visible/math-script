#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

//#include "module2.h"

#ifndef HASH_MAP_H
#define HASH_MAP_H

uint64_t stdhash_generic(const char *data, size_t size);
uint64_t stdhash_int(const void *data);
uint64_t stdhash_string(const void *data);

typedef struct HashMap HashMap;

HashMap *new_HashMap(size_t buckets, size_t (*hash_func)(const void *));
void free_HashMap(HashMap *hash_map);
void HashMap_rehash(HashMap *hash_map, size_t buckets);

#endif
