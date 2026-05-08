#ifndef DICT_H
#define DICT_H

#include <stddef.h>

typedef struct DictEntry {
    char *key;
    char *value;
    struct DictEntry *next;
} DictEntry;

typedef struct Dict {
    DictEntry **table;
    size_t size;
    size_t used;
} Dict;

Dict* dictCreate(size_t initial_size);
int dictSet(Dict *d, const char *key, const char *value);
char* dictGet(Dict *d, const char *key);
void dictFree(Dict *d);

#endif