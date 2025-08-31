#include "utils/hash_map.h"

bool stdcomp_int(const void *value1, const void *value2) {
	return (uint64_t)value1 == (uint64_t)value2;
}

bool stdcomp_string(const void *value1, const void *value2) {
	bool is_null1=(value1==NULL), is_null2=(value2==NULL);
	if (is_null1 || is_null2)
		return is_null1 && is_null2;
	
	return strcmp(value1, value2)==0;
}

bool stdcomp_wstring(const void *value1, const void *value2) {
	bool is_null1=(value1==NULL), is_null2=(value2==NULL);
	if (is_null1 || is_null2)
		return is_null1 && is_null2;
	
	return wcscmp(value1, value2)==0;
}

#define FNV1_PRIME 0x100000001b3

uint64_t stdhash_FNV1(const void *buffer, size_t size) {
	const uint64_t *data = buffer;
	register uint64_t hash = 0xcbf29ce484222325;
	register size_t i, len = size/sizeof(uint64_t);
	for (i=0; i<len; i++) {
		hash*=FNV1_PRIME;
		hash^=data[i];
	}
	
	uint64_t num=0, tailsize=size%sizeof(uint64_t);
	if (!tailsize)
		return hash;
	memcpy(&num, data+len, tailsize);
	hash*=FNV1_PRIME;
	return hash^num;
}

uint64_t stdhash_int(const void *buffer) {
	return (uint64_t)buffer;
}

uint64_t stdhash_string(const void *buffer) {
	return stdhash_FNV1(buffer, strlen((char *)buffer));
}

uint64_t stdhash_wstring(const void *buffer) {
	return stdhash_FNV1(buffer, wcslen((wchar_t *)buffer));
}


static hash_entry *new_hash_entry(void *key, void *value) {
	hash_entry *hash_entry = malloc(sizeof(hash_entry));
	hash_entry->key=key;
	hash_entry->value=value;
	hash_entry->next=NULL;
	
	return hash_entry;
}

static void free_hash_entry(hash_entry *hash_entry) {
	if (hash_entry==NULL)
		return;
	
	free_hash_entry(hash_entry->next);
	free(hash_entry);
}


struct HashMap {
	size_t buckets, *bucket_sizes;
	hash_entry **table;
	bool (*comp_func)(const void *, const void *);
	uint64_t (*hash_func)(const void *);
};

HashMap *new_HashMap(size_t buckets, bool (*comp_func)(const void *, const void *),
								   size_t (*hash_func)(const void *)) {
	HashMap *hash_map = malloc(sizeof(HashMap));
	hash_map->table = calloc(buckets, sizeof(hash_entry *));
	
	size_t i, *sizes = calloc(buckets, sizeof(size_t));
	for (i=0; i<buckets; i++) {
		sizes[i]=0;
		hash_map->table[i]=NULL;
	}
	
	hash_map->buckets=buckets;
	hash_map->bucket_sizes=sizes;
	hash_map->comp_func = (comp_func!=NULL)? comp_func: &stdcomp_int;
	hash_map->hash_func = (hash_func!=NULL)? hash_func: &stdhash_int;
	return hash_map;
}

void free_HashMap(HashMap *hash_map) {
	if (hash_map==NULL)
		return;
	
	for (size_t i=0; i<hash_map->buckets; i++)
		free_hash_entry(hash_map->table[i]);
	free(hash_map->table);
	free(hash_map->bucket_sizes);
	free(hash_map);
}

void HashMap_rehash(HashMap *hash_map, size_t buckets) {
	hash_entry *node, *node0, **table = calloc(buckets, sizeof(hash_entry *));
	size_t i, *sizes = realloc(hash_map->bucket_sizes, buckets*sizeof(size_t));
	for (i=0; i<buckets; i++) {
		sizes[i]=0;
		table[i]=NULL;
	}
	
	uint64_t index;
	for (i=0; i<hash_map->buckets; i++) {
		node0=hash_map->table[i];
		while (node0!=NULL) {
			index = hash_map->hash_func(node->key)%buckets;
			node=node0; node0=node0->next;
			node->next=table[index];
			table[index]=node;
			sizes[index]++;
		}
	}
	
	hash_map->buckets=buckets;
	hash_map->bucket_sizes=sizes;
	free(hash_map->table);
	hash_map->table=table;
}


void *HashMap_get(HashMap *hash_map, const void *key) {
	uint64_t index = hash_map->hash_func(key)%hash_map->buckets;
	hash_entry *node = hash_map->table[index];
	
	while (node!=NULL) {
		if (hash_map->comp_func(node->key, key))
			return node->value;
		node=node->next;
	}
	
	return NULL;
}

void HashMap_set(HashMap *hash_map, const void *key, const void *value) {
	uint64_t index = hash_map->hash_func(key)%hash_map->buckets;
	hash_entry *node = hash_map->table[index];
	
	while (node!=NULL) {
		if (hash_map->comp_func(node->key, key)) {
			node->value=value;
			return;
		}
		
		node=node->next;
	}
	
	node = new_hash_entry(key, value);
	node->next = hash_map->table[index];
	hash_map->table[index] = node;
	hash_map->bucket_sizes[index]++;
}

bool HashMap_delete(HashMap *hash_map, const void *key) {
	uint64_t index = hash_map->hash_func(key)%hash_map->buckets;
	hash_entry **place = hash_map->table+index, *node = *place;
	
	while (node!=NULL) {
		if (!hash_map->comp_func(node->key, key)) {
			place=&node->next; node=*place;
			continue;
		}
		
		*place=node->next; free(node);
		hash_map->bucket_sizes[index]--;
		return true;
	}
	
	return false;
}


void init_iterHashMap(iterHashMap *iter_hm, HashMap *hash_map) {
	iter_hm->hash_map = hash_map;
	iter_hm->entry = NULL;
	iter_hm->index = 0;
	iter_hm->length = hash_map->buckets;
}

hash_entry *iterHashMap_next(iterHashMap *iter_hm) {
	hash_entry *entry = iter_hm->entry;
	if (entry!=NULL) {
		iter_hm->entry = entry->next;
		return entry;
	}

	int i=iter_hm->index, length=iter_hm->length;
	if (i>=length)
		return NULL;

	hash_entry **table = iter_hm->hash_map->table;
	for (; i<length; i++) {
		entry=table[i];
		if (entry==NULL) continue;

		iter_hm->entry=entry->next;
		iter_hm->index=i+1;
		return entry;
	}

	iter_hm->index=i;
	return NULL;
}

