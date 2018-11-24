#include "hashset.h"
#include <stdlib.h>
#include <stdio.h>

#define SET_INITIAL_SIZE (256)
#define SET_MAX_CHAIN_LENGTH (8)

typedef struct _hashset_element{
    set_elem_t value;
    int in_use;
} hashset_element_t;

typedef struct _hashset_set{
    char *name;
    int table_size;
    unsigned long size;
    hashset_element_t *data;
} hashset_t;

set_t hashset_new(char *name) {
    hashset_t* set = (hashset_t*) malloc(sizeof(hashset_t));

    if (set == NULL)
        return NULL;

    set->data = (hashset_element_t*) calloc(SET_INITIAL_SIZE, sizeof(hashset_element_t));
    if(set->data == NULL) {
        free(set);
        return NULL;
    }

    set->table_size = SET_INITIAL_SIZE;
    set->size = 0;
    set->name = name;

    return set;
}

unsigned int hashset_hash_int(hashset_t * m, set_elem_t key){

    /* Robert Jenkins' 32 bit Mix Function */
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    /* Knuth's Multiplicative Method */
    key = (key >> 3) * 2654435761;

    return key % m->table_size;
}

int hashset_hash(set_t in, set_elem_t key){
    /* Cast the hashmap */
    hashset_t *set = (hashset_t *) in;

    /* If full, return immediately */
    if(set->size >= (set->table_size/2)) return SET_FULL;

    /* Find the best index */
    int curr = hashset_hash_int(set, key);

    /* Linear probing */
    for(int i = 0; i < SET_MAX_CHAIN_LENGTH; i++){
        if(set->data[curr].in_use == 0)
            return curr;

        if(set->data[curr].in_use == 1 && (set->data[curr].value == key))
            return curr;

        curr = (curr + 1) % set->table_size;
    }

    return SET_FULL;
}

int hashset_rehash(set_t in){
    /* Cast the hashmap */
    hashset_t *set = (hashset_t *) in;

    /* Setup the new elements */
    hashset_element_t *temp = (hashset_element_t *) calloc(2 * set->table_size, sizeof(hashset_element_t));

    if (temp == NULL)
        return SET_OMEM;

    /* Update the array */
    hashset_element_t *curr = set->data;
    set->data = temp;

    /* Update the size */
    int old_size = set->table_size;
    set->table_size = 2 * set->table_size;
    set->size = 0;

    /* Rehash the elements */
    for(int i = 0; i < old_size; i++){
        int status;

        if (curr[i].in_use == 0)
            continue;

        status = set_add(set, curr[i].value);
        if (status != SET_OK)
            return status;
    }

    free(curr);

    return SET_OK;
}

int set_add(set_t in, set_elem_t value) {
    /* Cast the hashmap */
    hashset_t *set = (hashset_t *) in;

    /* Find a place to put our value */
    int index = hashset_hash(in, value);
    while(index == SET_FULL){
        if (hashset_rehash(in) == SET_OMEM) {
            return SET_OMEM;
        }
        index = hashset_hash(in, value);
    }

    if (set->data[index].in_use == 1)
        fprintf(stderr, "[%s] %p ALREADY A MEMBER\n", set->name, value);

    /* Set the data */
    set->data[index].value = value;
    set->data[index].in_use = 1;
    set->size++;

    return SET_OK;
}

int set_member(set_t in, set_elem_t value) {
    /* Cast the hashmap */
    hashset_t* set = (hashset_t *) in;

    /* Find data location */
    int curr = hashset_hash_int(set, value);

    /* Linear probing, if necessary */
    for(int i = 0; i < SET_MAX_CHAIN_LENGTH; i++){

        int in_use = set->data[curr].in_use;
        if (in_use == 1){
            if (set->data[curr].value == value){
                return SET_OK;
            }
        }

        curr = (curr + 1) % set->table_size;
    }

    /* Not found */
    return SET_MISSING;
}

int set_iterate(set_t in, set_iter_t f, void *extra) {
    /* Cast the hashmap */
    hashset_t *set = (hashset_t*) in;

    /* On empty hashmap, return immediately */
    if (hashmap_length(set) <= 0)
        return SET_MISSING;

    /* Linear probing */
    for(int i = 0; i < set->table_size; i++)
        if(set->data[i].in_use != 0) {
            int status = f(set->data[i].value, extra);
            if (status != SET_OK) {
                return status;
            }
        }

    return SET_OK;
}

int set_remove(set_t in, set_elem_t value) {
    /* Cast the hashmap */
    hashset_t *set = (hashset_t *) in;

    /* Find key */
    int curr = hashset_hash_int(set, value);

    /* Linear probing, if necessary */
    for(int i = 0; i < SET_MAX_CHAIN_LENGTH; i++){

        int in_use = set->data[curr].in_use;
        if (in_use == 1){
            if (set->data[curr].value == value){
                //if (free_value)
                //    free(set->data[curr].data);

                /* Blank out the fields */
                set->data[curr].in_use = 0;
                set->data[curr].value = /*NULL*/ 0;

                /* Reduce the size */
                set->size--;
                return SET_OK;
            }
        }

        curr = (curr + 1) % set->table_size;
    }

    /* Data not found */
    return SET_MISSING;
}

void set_free(set_t in){
    hashset_t *set = (hashset_t*) in;
    free(set->data);
    free(set);
}

void set_clear(set_t in) {
        /* Cast the hashmap */
        hashset_t *set = (hashset_t *) in;

        /* Find key */
        for(int curr = 0; curr < set->table_size; curr++) {
            int in_use = set->data[curr].in_use;
            if (in_use == 1){

                /* Blank out the fields */
                set->data[curr].in_use = 0;
                set->data[curr].value = /*NULL*/ 0;

                /* Reduce the size */
                set->size--;
            }
            //curr = (curr + 1) % m->table_size;
        }
}

unsigned long set_length(set_t in){
    hashset_t *set = (hashset_t *) in;
    if(set != NULL)
        return set->size;
    else
        return 0;
}