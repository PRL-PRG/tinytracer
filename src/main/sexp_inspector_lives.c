#include <stdio.h>
#include <stdlib.h>

#include "extensible_array.h"
#include "sexp_inspector_shared.h"
#include "sexp_inspector_lives.h"

#include "hashmap.h"

map_t initial_gc_cycle_map;
FILE *sexp_inspector_lived_gc_cycles;

unsigned long current_gc_cycle;
ext_unsigned_long_array lived_gc_cycles;

typedef struct  {
    unsigned int initial_gc_cycle;
}* t_gc_info;

inline int sexp_inspector_lives_is_running() {
    return sexp_inspector_lived_gc_cycles != NULL;
}

void sexp_inspector_lives_initialize() {
    char *lives_path = getenv("SEXP_INSPECTOR_LIVES");
    if (lives_path == NULL) {
        sexp_inspector_lived_gc_cycles = NULL;
    } else {
        sexp_inspector_bump_analysis_counter();
        sexp_inspector_lived_gc_cycles = fopen(lives_path, "w");
        init_unsigned_long(&lived_gc_cycles, (size_t) 128);
        fprintf(sexp_inspector_lived_gc_cycles, "gc_cycles;count;percent\n");
        initial_gc_cycle_map = hashmap_new();
    }
}

int sexp_inspector_lives_register(SEXP sexp) {
    t_gc_info info = malloc(sizeof(t_gc_info));
    info->initial_gc_cycle = current_gc_cycle;
    return hashmap_put(initial_gc_cycle_map, (uintptr_t) sexp, info);
}

int record_live_objects_length_of_life(hashmap_key_t sexp, hashmap_val_t info, void *extra) {
    unsigned int lifespan = current_gc_cycle
                            - ((t_gc_info) info)->initial_gc_cycle;
    increment_unsigned_long(&lived_gc_cycles, lifespan);
    return MAP_OK;
}

void sexp_inspector_lives_close() {
    if (sexp_inspector_lives_is_running()) {
        hashmap_iterate(initial_gc_cycle_map,
                        record_live_objects_length_of_life, NULL);
        for (unsigned int i = 1; i < current_gc_cycle; i++)
            if (i < lived_gc_cycles.size)
                fprintf(sexp_inspector_lived_gc_cycles, "%i;%lu;%f\n", i, lived_gc_cycles.array[i],
                        100 * ((double) lived_gc_cycles.array[i]) / ((double) sexp_inspector_read_sexp_counter()));
            else
                fprintf(sexp_inspector_lived_gc_cycles, "%i;%lu;%f\n", i, 0L, 0.);
        fclose(sexp_inspector_lived_gc_cycles);
    }
}

void sexp_inspector_lives_gc_unmark(SEXP sexp) {
    hashmap_ret_t r = hashmap_get(initial_gc_cycle_map, (uintptr_t) sexp);

    if (r.status == MAP_OK) {
        unsigned int initial_gc_cycle = ((t_gc_info)r.value)->initial_gc_cycle;
        increment_unsigned_long(&lived_gc_cycles,
                                current_gc_cycle - initial_gc_cycle);
    }

    hashmap_remove(initial_gc_cycle_map, (uintptr_t) sexp);
}

void sexp_inspector_lives_new_gc_cycle() {
    current_gc_cycle++;
}