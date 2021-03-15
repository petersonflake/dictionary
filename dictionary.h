#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <stdlib.h>
#include <string.h>
#if PRINT_ERRORS==1
#include <stdio.h>
#endif
#define LINEAR_PROBING 0
#define PROBING_SCHEME LINEAR_PROBING
#define INDIVIDUAL_FUNCTIONS 0

typedef struct _dict_item *dict_item;

typedef struct _dictionary *dictionary;

struct _dict_item {
    void *data;
    unsigned int hash;
    char *key;
#if INDIVIDUAL_FUNCTIONS==1
    void (*free_item)(void *item);
#endif
};

struct _dictionary {
    unsigned int alloc_count;
    unsigned int num_items;
    dict_item *item_array;
#if INDIVIDUAL_FUNCTIONS==0
    void (*free_item)(void *item);
#endif
};

dictionary create_dictionary(unsigned int size);

#if INDIVIDUAL_FUNCTIONS==0
int dict_insert(dictionary dict, char *key, void *val);
#else
int dict_insert(dictionary dict, char *key, void *val,
        void (*free_item)(void *item));
#endif

void* dict_get(dictionary dict, char *key);

void dict_free(dictionary dict);
#endif
