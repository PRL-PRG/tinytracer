#include <stdlib.h>
#include "extensible_array.h"

void init_unsigned_long(ext_unsigned_long_array *a, size_t initial_size) {
    a->array = (unsigned long *) malloc(initial_size * sizeof(unsigned long));
    a->size = initial_size;

    //for (unsigned int i = 0; i < a->size; i++)
    //    a->array[i] = 0L;
}

void insert_unsigned_long(ext_unsigned_long_array *a, int index, unsigned long element) {
    if (a->size < index) {
        size_t old_size = a->size;
        a->size = index + 1;
        a->array = (unsigned long *) realloc(a->array, a->size * sizeof(unsigned long));

        //for(unsigned int i = old_size; i < a->size; i++)
        //    a->array[i] = 0L;
    }
    a->array[index] = element;
}

unsigned long get_unsigned_long(ext_unsigned_long_array *a, int index) {
    if (a->size < index)
        return a->array[index];

    return 0L;
}

unsigned long increment_unsigned_long(ext_unsigned_long_array *a, int index) {
    if (index < a->size)
        return ++a->array[index];

    return 0L;
}

unsigned long *get_reference_unsigned_long(ext_unsigned_long_array *a, int index) {
    if (a->size < index)
        return &(a->array[index]);

    return NULL;
}

void free_unsigned_long(ext_unsigned_long_array *a) {
    free(a->array);
    a->array = NULL;
    a->size = 0L;
}



