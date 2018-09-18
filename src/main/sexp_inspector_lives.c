#include <stdio.h>
#include <stdlib.h>

#include "extensible_array.h"
#include "sexp_inspector_shared.h"
#include "sexp_inspector_lives.h"

#include "hashmap.h"

FILE *sexp_inspector_lived_gc_cycles;

unsigned long current_gc_cycle;
ext_unsigned_long_array lived_gc_cycles;

inline int sexp_inspector_lives_is_running() {
    return sexp_inspector_lived_gc_cycles != NULL;
}

void sexp_inspector_lives_initialize() {
    // Initialize an output file for SEXP length-of-life analysis, if path is provided.
    char *lives_path = getenv("SEXP_INSPECTOR_LIVES");
    if (lives_path == NULL) {
        sexp_inspector_lived_gc_cycles = NULL;
    } else {
        sexp_inspector_bump_analysis_counter();
        sexp_inspector_lived_gc_cycles = fopen(lives_path, "w");
        init_unsigned_long(&lived_gc_cycles, 128);
        fprintf(sexp_inspector_lived_gc_cycles, "gc_cycles;count;percent\n");
    }

}

int record_live_objects_length_of_life(hashmap_key_t sexp, hashmap_val_t sexp_info, void *extra) {
    increment_unsigned_long(&lived_gc_cycles, current_gc_cycle - ((t_sexp_info)sexp_info)->initial_gc_cycle);
    return MAP_OK;
}

void sexp_inspector_lives_close() {
    if (sexp_inspector_lived_gc_cycles != NULL) {
        hashmap_iterate((map_t) hack_get_fake_id_dictionary_do_not_use(),
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

void sexp_inspector_lives_increment(unsigned int initial_gc_cycle) {
    increment_unsigned_long(&lived_gc_cycles, current_gc_cycle - initial_gc_cycle);
}
