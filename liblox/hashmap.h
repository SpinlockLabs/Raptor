/**
 * Implements a basic key-value hashmap.
 * Used the implementation from Toaruos.
 */
#pragma once

#include <stdbool.h>

#include "common.h"
#include "list.h"

typedef uint (*hashmap_hash_t)(void* key);
typedef int (*hashmap_compare_t)(void* a, void* b);
typedef void (*hashmap_free_t)(void*);
typedef void* (*hashmap_duplicate_t)(void*);

typedef struct hashmap_entry {
    void* key;
    void* value;
    struct hashmap_entry* next;
} hashmap_entry_t;

typedef struct hashmap {
    hashmap_hash_t hash;
    hashmap_compare_t compare;
    hashmap_duplicate_t key_duplicate;
    hashmap_free_t key_free;
    hashmap_free_t value_free;
    size_t size;
    hashmap_entry_t** entries;
} hashmap_t;

hashmap_t* hashmap_create(size_t size);
hashmap_t* hashmap_create_int(size_t size);
void* hashmap_set(hashmap_t* map, void* key, void* value);
void* hashmap_get(hashmap_t* map, void* key);
bool hashmap_has(hashmap_t* map, void* key);
list_t* hashmap_keys(hashmap_t* map);
list_t* hashmap_values(hashmap_t* map);
void hashmap_free(hashmap_t* map);
uint hashmap_string_hash(void* key);
int hashmap_string_compare(void* a, void* b);
void* hashmap_string_duplicate(void* key);
