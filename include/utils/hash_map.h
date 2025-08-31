#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

bool stdcomp_int(const void *, const void *);
bool stdcomp_string(const void *, const void *);
bool stdcomp_wstring(const void *, const void *);

uint64_t stdhash_FNV1(const void *buffer, size_t size);
uint64_t stdhash_int(const void *buffer);
uint64_t stdhash_string(const void *buffer);
uint64_t stdhash_wstring(const void *buffer);

typedef struct hash_entry hash_entry;
typedef struct HashMap HashMap;

struct hash_entry {
	void *key, *value;
	hash_entry *next;
};

HashMap *new_HashMap(size_t buckets, bool (*comp_func)(const void *, const void *),
								   size_t (*hash_func)(const void *));
void free_HashMap(HashMap *);
void HashMap_rehash(HashMap *, size_t buckets);

void *HashMap_get(HashMap *, const void *key);
void HashMap_set(HashMap *, const void *key, const void *value);
bool HashMap_delete(HashMap *, const void *key);

typedef struct iterHashMap {
	HashMap *hash_map;
	hash_entry *entry;
	size_t index, length;
} iterHashMap;

void init_iterHashMap(iterHashMap *, HashMap *);
hash_entry *iterHashMap_next(iterHashMap *);

#endif
