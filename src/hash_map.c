#include "hash_map.h"

uint64_t stdhash_generic(const char *data, size_t size) {
    uint64_t *buffer = (uint64_t *)data, hash_value = 0x28bce647778c5f3;
    for (size_t i=0; i<size/sizeof(uint64_t); i++) {
        ;
    }
    
    return hash_value;
}

uint64_t stdhash_int(const void *data) {
    return (uint64_t)data;
}

uint64_t stdhash_string(const void *data);


struct hash_entry {
    void *key, *value;
    struct hash_entry *next;
};

struct hash_entry *new_hash_entry(void *key, void *value) {
    struct hash_entry *hash_entry = malloc(sizeof(struct hash_entry));
    hash_entry->key=key;
    hash_entry->value=value;
    hash_entry->next=NULL;
    
    return hash_entry;
}

void free_hash_entry(struct hash_entry *hash_entry) {
    if (hash_entry==NULL)
        return;
    
    free_hash_entry(hash_entry->next);
    free(hash_entry);
}

struct HashMap {
    size_t buckets, *bucket_sizes;
    struct hash_entry **table;
    uint64_t (*hash_func)(const void *);
};

HashMap *new_HashMap(size_t buckets, size_t (*hash_func)(const void *)) {
    HashMap *hash_map = malloc(sizeof(HashMap));
    hash_map->table = calloc(buckets, sizeof(struct hash_enrty *));
    
    size_t i, *sizes = calloc(buckets, sizeof(size_t));
    for (i=0; i<buckets; i++) {
        sizes[i]=0;
        hash_map->table[i]=NULL;
    }
    
    hash_map->buckets=buckets;
    hash_map->bucket_sizes=sizes;
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
    return;
}

void *HashMap_get(HashMap *hash_map, const void *key);
void *HashMap_set(HashMap *hash_map, const void *key, const void *value);
