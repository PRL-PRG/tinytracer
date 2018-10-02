#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "sexp_inspector_shared.h"
#include "sexp_inspector_types.h"
#include "sexp_inspector_lives.h"
#include "sexp_inspector_composition.h"
#include "sexp_inspector_debug.h"

void sexp_inspector_init() {
    sexp_inspector_types_initialize();
    sexp_inspector_lives_initialize();
    sexp_inspector_composition_initialize();

    if (sexp_inspector_are_there_analyses()) {
        sexp_inspector_debug_initialize();
        sexp_inspector_initialize_fake_ids();
    }
}

void sexp_inspector_allocation(SEXP sexp) {
    if (!sexp_inspector_are_there_analyses())
        return;
c
    sexp_inspector_register_fake_id(sexp);
    sexp_inspector_bump_sexp_counter();

    if (sexp_inspector_types_is_running())
        sexp_inspector_types_increment(sexp);

    if (sexp_inspector_lives_is_running())
        sexp_inspector_lives_register(sexp);

    if (sexp_inspector_debug_is_running())
        sexp_inspector_debug_note_allocation(sexp);
}

void sexp_inspector_inspect_all_known() {
    if (!sexp_inspector_are_there_analyses())
        return;

    //XXX hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known, NULL);
    sexp_inspector_lives_new_gc_cycle();
}

void sexp_inspector_gc(SEXP sexp) {
    if (!sexp_inspector_are_there_analyses())
        return;

    unsigned long *fake_id = sexp_inspector_retrieve_fake_id(sexp);

    if (sexp_inspector_lives_is_running())
        sexp_inspector_lives_gc_unmark(sexp);

    if (sexp_inspector_debug_is_running() && (fake_id == NULL))
        sexp_inspector_debug_note_unknown_SEXP(sexp);

    if (sexp_inspector_composition_is_running())
        sexp_inspector_composition_register(sexp);

    if (sexp_inspector_debug_is_running()) {
        unsigned long fake_id_value = fake_id != NULL ? *fake_id : ULONG_MAX;
        sexp_inspector_debug_note_gc_unmark(sexp, fake_id_value);
    }

    sexp_inspector_remove_fake_id(sexp);
}

void sexp_inspector_close() {
    if (!sexp_inspector_are_there_analyses())
        return;

    //XXX hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known, NULL);
    sexp_inspector_lives_new_gc_cycle();

    sexp_inspector_types_close();
    sexp_inspector_lives_close();
    sexp_inspector_composition_close();

    sexp_inspector_debug_close();
}

// TODO investigate write barrier

//int sexp_inspector_inspect_register(SEXP sexp, hashmap_val_t sexp_info) {
//    sexp_inspector_composition_increment(sexp);
//    return MAP_OK;
//}

// XXX
//int sexp_inspector_inspect_one_known(hashmap_key_t sexp, hashmap_val_t sexp_info) {
//    if (sexp_inspector_debug != NULL)
//        fprintf(sexp_inspector_debug, "inspecting current_gc_cycle=%lu sexp=%p fake_id=%lu\n",
//                current_gc_cycle, (void *) sexp, sexp_info->fake_id);
//
//    return MAP_OK;
//}