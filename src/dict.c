#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/dict.h"

static unsigned long hashFunction(const char *str) {
    unsigned long hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

//每次Set和Get时搬运一个筒；Dict的size和used属性必须时刻根据增删变化！！！
static int dictRehash(Dict *d, int n) {
    if (d->rehashidx == -1) return 0;
    while(n--) {
        if (d->used[0] == 0) {
            free(d->table[0]);
            d->table[0] = d->table[1];
            d->size[0] = d->size[1];
            d->used[0] = d->used[1];

            d->table[1] = NULL;
            d->size[1] = 0;
            d->used[1] = 0;
            d->rehashidx = -1;
            return 0;
        }
        while (d->rehashidx < d->size[0] && d->table[0][d->rehashidx] == NULL) d->rehashidx++;
        int idx0 = d->rehashidx++;
        DictEntry *curr = d->table[0][idx0];
        while(curr) {
            DictEntry *next = curr->next;
            unsigned int idx1 = hashFunction(curr->key) % d->size[1];
            curr->next = d->table[1][idx1];
            d->table[1][idx1] = curr;
            d->used[0]--;
            d->used[1]++;
            curr = next;
        }
        d->table[0][idx0] = NULL;
    }
    return 1;
}


//封装
static void _dictRehashStep(Dict *d) {
    dictRehash(d, 1);
}

//创建变量后及时初始化：malloc出的是垃圾值，记得将table指向NULL；malloc后立即检查是否分配内存成功
Dict* dictCreate(size_t initial_size) {
    Dict *d = malloc(sizeof(Dict));
    if (!d) return NULL;
    d->size[0] = initial_size;
    d->used[0] = 0;
    d->table[1] = NULL;
    d->size[1] = 0;
    d->used[1] = 0;
    d->rehashidx = -1;

    d->table[0] = calloc(initial_size, sizeof(DictEntry*));
    if (!d->table[0]) {
        free(d);
        return NULL;
    }
    return d;
}

int dictSet(Dict *d, const char *key, const char *value) {
    if(d==NULL || key==NULL || value==NULL) return 0;
    //如果超出table[0]大小则开table[1]
    if (d->rehashidx == -1 && d->used[0] == d->size[0]) {
        d->rehashidx = 0;
        d->table[1] = calloc(2 * d->size[0], sizeof(DictEntry*));
        d->size[1] = 2 * d->size[0];
        d->used[1] = 0;
    }
    int i = (d->rehashidx == -1)? 0: 1;
    if (i == 1) _dictRehashStep(d); //搬运一个
    //i = 1时：先检查table[0]有没有；有直接修改，没有再去table[1]找；有直接修改，没有就在table[1]里加，保证table[0]只减不增
    size_t index = hashFunction(key) % d->size[0];
    DictEntry *e = d->table[0][index];
    while(e) {
        if (strcmp(e->key, key) == 0) {
            free(e->value);
            e->value = strdup(value);
            return 1;
        }
        e = e->next;
    }
    if (i == 1) {
        index = hashFunction(key) % d->size[i];
        e = d->table[i][index];
        while(e){
            if (strcmp(e->key, key) == 0) {
                free(e->value);
                e->value = strdup(value);
                return 1;
            }
            e = e->next;
        }
    }
    DictEntry *new_entry = malloc(sizeof(DictEntry));
    if (new_entry == NULL) return 0;
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = d->table[i][index];
    d->table[i][index] = new_entry;
    d->used[i]++;
    return 1;
    
}

char* dictGet(Dict *d, const char *key) {
    if (d == NULL || key == NULL) return NULL;
    if (d->rehashidx != -1) _dictRehashStep(d);
    //先去table[0]找再去table[1]找
    for(int i = 0; i < 2; i++) {
        size_t index = hashFunction(key) % d->size[i];
        DictEntry *e = d->table[i][index];
        while(e) {
            if (strcmp(e->key, key) == 0) {
                return e->value;
            }
            e = e->next;
        }
        if (d->rehashidx == -1) break;
    }
    return NULL;
}


void dictFree(Dict *d) {
    if (d == NULL) return;
    DictEntry *temp = NULL;
    for(int idx = 0; idx < 2; idx++) {
        if (d->table[idx] == NULL) break;
        for (size_t i = 0; i < d->size[idx]; i++) {
            DictEntry *e = d->table[idx][i];
            while(e) {
                temp = e->next;
                free(e->key);
                free(e->value);
                free(e);
                e = temp;
            }
        }
        free(d->table[idx]);
    }
    free(d);
}

int dictDelete(Dict *d, char *key) {
    if (d == NULL || key == NULL) return 0;
    if (d->rehashidx != -1) _dictRehashStep(d);
    for(int i = 0; i < 2; i++) {
        unsigned int idx = hashFunction(key) % d->size[i];
        DictEntry *e = d->table[i][idx];
        DictEntry *pre = NULL; //处理首节点时将pre设为NULL
        while(e) {
            if (strcmp(e->key, key) == 0) {
                if (pre == NULL) {
                    d->table[i][idx] = e->next;
                }
                else {
                    pre->next = e->next;
                }
                free(e->value);
                free(e->key);
                free(e);
                d->used[i]--;
                return 1;
            }
            pre = e;
            e = e->next;
        }
        if (d->rehashidx == -1) break;
    }
    return 0;
} 