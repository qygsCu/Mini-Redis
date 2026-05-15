#ifndef DICT_H
#define DICT_H

#include <stddef.h>

typedef struct DictEntry {
    char *key;
    char *value;
    struct DictEntry *next;
} DictEntry;

typedef struct Dict {
    DictEntry **table[2];
    size_t size[2];
    size_t used[2];
    int rehashidx;
} Dict;

Dict* dictCreate(size_t initial_size);
int dictSet(Dict *d, const char *key, const char *value);
char* dictGet(Dict *d, const char *key);
void dictFree(Dict *d);
int dictDelete(Dict *d, char *key);

#endif