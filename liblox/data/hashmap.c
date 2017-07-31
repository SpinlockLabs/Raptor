#include "../list.h"
#include "../hashmap.h"
#include "../string.h"

uint hashmap_string_hash(void* _key) {
    uint hash = 0;
    char* key = (char*) _key;
    int c;

    /* This is the so-called "sdbm" hash. It comes from a piece of
     * public domain code from a clone of ndbm. */
    while ((c = *key++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

int hashmap_string_comp(void* a, void* b) {
    return !strcmp(a, b);
}

void* hashmap_string_duplicate(void* key) {
    return strdup(key);
}

uint hashmap_int_hash(void* key) {
    return (uint) key;
}

int hashmap_int_comp(void* a, void* b) {
    return (int) a == (int) b;
}

void* hashmap_int_duplicate(void* key) {
    return key;
}

static void hashmap_int_free(void* ptr) {
    unused(ptr);
}

hashmap_t* hashmap_create(size_t size) {
    hashmap_t* map = zalloc(sizeof(hashmap_t));

    map->hash = hashmap_string_hash;
    map->compare = hashmap_string_comp;
    map->key_duplicate = hashmap_string_duplicate;
    map->key_free = free;
    map->value_free = free;
    map->size = size;
    map->entries = zalloc(sizeof(hashmap_entry_t*) * size);

    return map;
}

hashmap_t* hashmap_create_int(size_t size) {
    hashmap_t* map = zalloc(sizeof(hashmap_t));

    map->hash = &hashmap_int_hash;
    map->compare = &hashmap_int_comp;
    map->key_duplicate = &hashmap_int_duplicate;
    map->key_free = &hashmap_int_free;
    map->value_free = &free;

    map->size = size;
    map->entries = zalloc(sizeof(hashmap_entry_t*) * size);

    return map;
}

void* hashmap_set(hashmap_t* map, void* key, void* value) {
    if (map == NULL) {
        return NULL;
    }

    uint hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        hashmap_entry_t* e = zalloc(sizeof(hashmap_entry_t));
        e->key = map->key_duplicate(key);
        e->value = value;
        e->next = NULL;
        map->entries[hash] = e;
        return e;
    }

    hashmap_entry_t* p = NULL;
    do {
        if (map->compare(x->key, key)) {
            void* out = x->value;
            x->value = value;
            return out;
        }

        p = x;
        x = x->next;
    } while (x);
    hashmap_entry_t* e = zalloc(sizeof(hashmap_entry_t));
    e->key = map->key_duplicate(key);
    e->value = value;
    e->next = NULL;
    p->next = e;

    return e;
}

void* hashmap_get(hashmap_t* map, void* key) {
    if (map == NULL) {
        return NULL;
    }

    uint hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        return NULL;
    }

    do {
        if (map->compare(x->key, key)) {
            return x->value;
        }
        x = x->next;
    } while (x);
    return NULL;
}

void* hashmap_remove(hashmap_t* map, void* key) {
    if (map == NULL) {
        return NULL;
    }

    uint hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        return NULL;
    }

    if (map->compare(x->key, key)) {
        void* out = x->value;
        map->entries[hash] = x->next;
        map->key_free(x->key);
        map->value_free(x);
        return out;
    }

    hashmap_entry_t* p = x;
    x = x->next;
    do {
        if (map->compare(x->key, key)) {
            void* out = x->value;
            p->next = x->next;
            map->key_free(x->key);
            map->value_free(x);
            return out;
        }
        p = x;
        x = x->next;
    } while (x);

    return NULL;
}

bool hashmap_has(hashmap_t* map, void* key) {
    if (map == NULL) {
        return false;
    }

    uint hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];

    if (!x) {
        return false;
    }

    do {
        if (map->compare(x->key, key)) {
            return true;
        }
        x = x->next;
    } while (x);

    return false;
}

list_t* hashmap_keys(hashmap_t* map) {
    if (map == NULL) {
        return NULL;
    }

    list_t* l = list_create();
    l->free_values = false;

    for (uint i = 0; i < map->size; ++i) {
        hashmap_entry_t* x = map->entries[i];
        while (x) {
            list_add(l, x->key);
            x = x->next;
        }
    }

    return l;
}

list_t* hashmap_values(hashmap_t* map) {
    if (map == NULL) {
        return NULL;
    }

    list_t* l = list_create();
    l->free_values = false;

    for (uint i = 0; i < map->size; ++i) {
        hashmap_entry_t* x = map->entries[i];
        while (x) {
            list_add(l, x->value);
            x = x->next;
        }
    }

    return l;
}

void hashmap_free(hashmap_t* map) {
    if (map == NULL) {
        return;
    }

    for (uint i = 0; i < map->size; ++i) {
        hashmap_entry_t* x = map->entries[i], *p;
        while (x) {
            p = x;
            x = x->next;
            map->key_free(p->key);
            map->value_free(p);
        }
    }
    free(map->entries);
}

size_t hashmap_count(hashmap_t* map) {
    size_t count = 0;

    for (uint i = 0; i < map->size; ++i) {
        hashmap_entry_t* x = map->entries[i];
        while (x) {
            x = x->next;

            count++;
        }
    }

    return count;
}
