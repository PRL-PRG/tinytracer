#include <stdlib.h>

#include "sexp_inspector_shared.h"
#include "hashmap.h"
#include "hashset.h"

set_t garbage_collected_sexps;

typedef struct {
    unsigned long fake_id;
    //unsigned int  initial_gc_cycle;
} *t_sexp_info;

map_t fake_id_dictionary;

unsigned int  analysis_counter = 0;
//unsigned int  gc_counter       = 0;
unsigned long sexp_counter     = 0;
unsigned long fake_id_sequence = 0;
unsigned long fake_id_counter  = 0;

void sexp_inspector_initialize_fake_ids() {
    fake_id_dictionary = hashmap_new("Fake ID dictionary");
}

int sexp_inspector_register_fake_id(SEXP sexp) {
    t_sexp_info info = malloc(sizeof(t_sexp_info));
    info->fake_id = ++fake_id_sequence;
    fake_id_counter++;

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
    fake_id_counter--;
    return hashmap_remove(fake_id_dictionary, (uintptr_t) sexp, 0); // XXX should be 1?
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

unsigned long sexp_inspector_count_registered_sexps() {
    return hashmap_length(fake_id_dictionary);
}

void sexp_inspector_iterate_over_tracked_sexps(sexp_iter f, void *result) {
    hashmap_iterate_keys(fake_id_dictionary, f, result);
}

void sexp_inspector_initialize_gc_memory_registry() {
    garbage_collected_sexps = hashset_new("GC'd SEXPS");
    if (garbage_collected_sexps == NULL)
        fprintf(stderr, "BELGIUM: (tinytracer) Could not initialize "
                "hashset for garbage collected SEXPs");
}

void sexp_inspector_unregister_gc_memory(SEXP sexp) {
    set_remove(garbage_collected_sexps, (uintptr_t) sexp);
}

/*
 * If the memory was previously registered returns 1
 */
int sexp_inspector_register_gc_memory(SEXP sexp) {
    int have_we_seen_this_one_before = set_member(garbage_collected_sexps, (uintptr_t) sexp);
    if (have_we_seen_this_one_before == SET_OK) {
        //fprintf(stderr, "BELGIUM: I have seen this SEXP in GC before without reallocation %p/%i/%i/%i\n",
        //        sexp, TYPEOF(sexp), classify_sexp(TYPEOF(sexp))), r.status;
        return 1;
    }

    set_add(garbage_collected_sexps, (uintptr_t) sexp);
    return 0;
}