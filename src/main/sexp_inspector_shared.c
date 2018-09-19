#include <stdlib.h>

#include "sexp_inspector_shared.h"
#include "hashmap.h"

typedef struct {
    unsigned long fake_id;
    //unsigned int  initial_gc_cycle;
} *t_sexp_info;

map_t fake_id_dictionary;

unsigned int  analysis_counter = 0;
//unsigned int  gc_counter       = 0;
unsigned long sexp_counter     = 0;
unsigned long fake_id_sequence = 0;

void sexp_inspector_initialize_fake_ids() {
    fake_id_dictionary = hashmap_new();
}

int sexp_inspector_register_fake_id(SEXP sexp) {
    t_sexp_info info = malloc(sizeof(t_sexp_info));
    info->fake_id = ++fake_id_sequence;

    return hashmap_put(fake_id_dictionary, (uintptr_t) sexp, info);
}

unsigned long *sexp_inspector_retrieve_fake_id(SEXP sexp) {
    hashmap_ret_t r = hashmap_get(fake_id_dictionary, (uintptr_t) sexp);

    if (r.status == MAP_OK)
        return &((t_sexp_info) r.value)->fake_id;
    else
        return NULL;
}

int sexp_inspector_remove_fake_id(SEXP sexp) {
    return hashmap_remove(fake_id_dictionary, (uintptr_t) sexp);
}

int sexp_inspector_are_there_analyses() {
    return analysis_counter > 0;
}

void sexp_inspector_bump_analysis_counter() {
    analysis_counter++;
}

void sexp_inspector_bump_sexp_counter() {
    sexp_counter++;
}

//void sexp_inspector_bump_gc_counter() {
//    gc_counter++;
//}

unsigned long sexp_inspector_read_sexp_counter() {
    return sexp_counter;
}

//unsigned int sexp_inspector_read_gc_counter() {
//    return gc_counter;
//}

unsigned long sexp_inspector_read_fake_id_sequence() {
    return fake_id_sequence;
}