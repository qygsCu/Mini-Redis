#include<stdio.h>
#include "../include/dict.h"

int main() {
    Dict *d = dictCreate(8);
    dictSet(d, "name", "Pengze");
    dictSet(d, "University", "WHU");
    char *name = dictGet(d, "name");
    char *University = dictGet(d, "University");
    printf("%s\n", name);
    printf("%s\n", University);
    dictFree(d);
}