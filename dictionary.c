#include "dictionary.h"

static unsigned int djb_hash(char *string)
{
    unsigned int hash = 5381;
    for(; *string; ++string)
        hash = ((hash << 5) + hash + *string);
    return hash;
}

static dict_item get_container(dictionary dict, char *key)
{
    unsigned int hash = djb_hash(key);
    unsigned int index = hash%dict->alloc_count;
    if(!dict->item_array[index]) return NULL;
    if(!strcmp(dict->item_array[index]->key, key))
        return dict->item_array[index];

#if PROBING_SCHEME==LINEAR_PROBING
    int offset = 1;
    while((index + offset) % dict->alloc_count != index) {
        if(!dict->item_array[index + offset]) {
            return NULL;
        } else if(!strcmp(dict->item_array[index + offset]->key, key)) {
            return dict->item_array[index + offset];
        }
        ++offset;
    }
#endif
    return NULL;
}
dictionary create_dictionary(unsigned int size)
{
    dictionary tmp = malloc(sizeof(struct _dictionary));
    tmp->num_items = 0;
    tmp->alloc_count = size;
    tmp->item_array = calloc(size, sizeof(dict_item));
    return tmp;
}

#if INDIVIDUAL_FUNCTIONS==0
int dict_insert(dictionary dict, char *key, void *val)
{
    if(dict->num_items > dict->alloc_count) {
        return -1;
#if PRINT_ERRORS==1
        fprintf(stdout, "Dictionary already full, insertion"
            " failed for key: %s\n", key);
#endif
    }
    unsigned int hash = djb_hash(key);
    unsigned int index = hash%dict->alloc_count;
    dict_item tmp = malloc(sizeof(struct _dict_item));
    tmp->hash = hash;
    tmp->data = val;
    tmp->key = strdup(key);
    dict_item at_index = dict->item_array[index];
    if(!at_index || at_index->hash == hash) {
        if(!at_index) {
            dict->item_array[index] = tmp;
            ++dict->num_items;
        } else {
            if(dict->free_item)
                dict->free_item(at_index);
            else
                free(at_index->data);
            *at_index = *tmp;
            ++dict->num_items;
        }
        return 0;
    }
#if PROBING_SCHEME==LINEAR_PROBING
    int size = dict->alloc_count;
    while(++index%size != size) {
        dict_item at_index = dict->item_array[index%size];
        if(!at_index || at_index->hash == hash) {
            if(!at_index) {
                dict->item_array[index] = tmp;
                ++dict->num_items;
                return 0;
            } else {
                if(dict->free_item)
                    dict->free_item(at_index->data);
                else
                    free(at_index->data);
                *at_index = *tmp;
                ++dict->num_items;
                return 0;
            }
        }
    }
    free(tmp->key);
    free(tmp);
    return 1;
#endif
}
#elif INDIVIDUAL_FUNCTIONS==1
int dict_insert(dictionary dict, char *key, void *val,
        void (*free_item)(void *item))
{
    if(dict->num_items > dict->alloc_count) {
        return -1;
#if PRINT_ERRORS==1
        fprintf(stdout, "Dictionary already full, insertion"
            " failed for key: %s\n", key);
#endif
    }
    unsigned int hash = djb_hash(key);
    unsigned int index = hash%dict->alloc_count;
    dict_item tmp = malloc(sizeof(struct _dict_item));
    tmp->hash = hash;
    tmp->data = val;
    tmp->key = strdup(key);
    tmp->free_item = free_item;
    if(!dict->item_array[index]) {
        dict->item_array[index] = tmp;
        ++dict->num_items;
        return 0;
    }
#if PROBING_SCHEME==LINEAR_PROBING
    int size = dict->alloc_count;
    while(++index%size != size) {
        if(!dict->item_array[index%size]) {
            dict->item_array[index] = tmp;
            return 0;
        }
    }
#endif
    free(tmp->key);
    free(tmp);
    return 1;
}
#endif

void* dict_get(dictionary dict, char *key)
{
    dict_item tmp = get_container(dict, key);
    if(tmp)
        if(tmp->data)
            return tmp->data;
#if PRINT_ERRORS==1
    fprintf(stdout, "key: \"%s\" not in dictionary\n", key);
#endif
    return NULL;
}

void dict_free(dictionary dict)
{
#if INDIVIDUAL_FUNCTIONS==0
    if(!dict->free_item) {
        dict->free_item = free;
#if PRINT_ERRORS==1
        fprintf(stderr, "No item freeing function set, "
                "using \"free\" as default.\n");
    }
#endif
    for(int i = 0; i < dict->alloc_count; ++i) {
        if(dict->item_array[i]) {
            free(dict->item_array[i]->key);
            dict->free_item(dict->item_array[i]);
        }
    }
#elif INDIVIDUAL_FUNCTIONS==1
    for(int i = 0; i < dict->alloc_count; ++i) {
        if(dict->item_array[i]) {
            if(dict->item_array[i]->free_item) {
                dict->item_array[i]->free_item(
                        dict->item_array[i]->data);
            } else free(dict->item_array[i]->data);
        free(dict->item_array[i]->key);
        }
    }
#endif
#if INDIVIDUAL_FUNCTIONS==0
}
#endif
}
