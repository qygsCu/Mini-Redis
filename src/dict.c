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