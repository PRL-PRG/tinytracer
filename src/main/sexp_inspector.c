#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"
#include "extensible_array.h"

#define USE_RINTERNALS

#include "../include/Rinternals.h"
#include "sexp_info.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

FILE *sexp_inspector_types;
FILE *sexp_inspector_lived_gc_cycles;
FILE *sexp_inspector_debug;

unsigned int active_analyses = 0;
map_t fake_id_dictionary;
unsigned long fake_id_sequence;
unsigned long current_gc_cycle;
ext_unsigned_long_array lived_gc_cycles;

// type counter
unsigned long sexp_counter;
unsigned long type_counters[25+1];

void sexp_inspector_init() {
    // Initialize an output file for SEXP type analysis, if path is provided.
    char *types_path = getenv("SEXP_INSPECTOR_TYPES");
    if (types_path == NULL) {
        sexp_inspector_types = NULL;
    } else {
        active_analyses++;
        sexp_inspector_types = fopen(types_path, "w");
        fprintf(sexp_inspector_types, "type;type_name;count;percent\n");
    }

    // Initialize an output file for SEXP length-of-life analysis, if path is provided.
    char *lives_path = getenv("SEXP_INSPECTOR_LIVES");
    if (lives_path == NULL) {
        sexp_inspector_lived_gc_cycles = NULL;
    } else {
        active_analyses++;
        sexp_inspector_lived_gc_cycles = fopen(lives_path, "w");
        init_unsigned_long(&lived_gc_cycles, 128);
        fprintf(sexp_inspector_lived_gc_cycles, "gc_cycles;count;percent\n");
    }

    if (active_analyses > 0) {
        char *debug_path = getenv("SEXP_INSPECTOR_DEBUG");
        sexp_inspector_debug = fopen(debug_path, "w");

        fake_id_dictionary = hashmap_new();
        fake_id_sequence = 0L;
        current_gc_cycle = 0L;
    }
}

void sexp_inspector_allocation(SEXP sexp) {
    if (active_analyses == 0)
        return;

    sexp_info sexp_info = malloc(sizeof(sexp_info));
    sexp_info->fake_id = ++fake_id_sequence;
    sexp_info->initial_gc_cycle = current_gc_cycle;

    hashmap_put(fake_id_dictionary, (uintptr_t) sexp, sexp_info);

    // SEXP type analysis.
    type_counters[TYPEOF(sexp)]++;
    sexp_counter++;

    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "allocating current_gc_cycle=%lu sexp=%p fake_id=%lu\n",
                current_gc_cycle, (void *) sexp, fake_id_sequence);
}

int sexp_inspector_inspect_one_known(hashmap_key_t sexp, hashmap_val_t sexp_info) {
    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "inspecting current_gc_cycle=%lu sexp=%p fake_id=%lu\n",
                current_gc_cycle, (void *) sexp, sexp_info->fake_id);

    return MAP_OK;
}

void sexp_inspector_inspect_all_known() {
    if (active_analyses == 0)
        return;

    hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known, NULL);
    current_gc_cycle++;
}

void sexp_inspector_gc(SEXP sexp) {
    if (active_analyses == 0)
        return;

    hashmap_ret_t r = hashmap_get(fake_id_dictionary, (uintptr_t) sexp);
    unsigned long fake_id = r.status == MAP_OK ? r.value->fake_id : 0;

    if (sexp_inspector_lived_gc_cycles != NULL) {
        if (r.status == MAP_OK)
            increment_unsigned_long(&lived_gc_cycles, current_gc_cycle - r.value->initial_gc_cycle);
        else
            if (sexp_inspector_debug != NULL)
                fprintf(sexp_inspector_debug, "gc unknown SEXP current_gc_cycle=%lu sexp=%p fake_id=%lu\n",
                        current_gc_cycle, (void *) sexp, fake_id);
    }

    hashmap_remove(fake_id_dictionary, (uintptr_t) sexp);

    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "gc unmark current_gc_cycle=%lu sexp=%p fake_id=%lu\n",
                current_gc_cycle, (void *) sexp, fake_id);
}

void write_out_types() {
    for (int i = 0; i < 25+1; i++)
        fprintf(sexp_inspector_types, "%i;%s;%lu;%f\n", i, sexptype2char(i),
                type_counters[i], 100 * ((double) type_counters[i]) / ((double) sexp_counter));
}

int record_live_objects_length_of_life(hashmap_key_t sexp, hashmap_val_t sexp_info, void *extra) {
    increment_unsigned_long(&lived_gc_cycles, current_gc_cycle - sexp_info->initial_gc_cycle);
    return MAP_OK;
}


void write_out_lives() {
    hashmap_iterate(fake_id_dictionary, record_live_objects_length_of_life, NULL);
    for (unsigned int i = 1; i < current_gc_cycle; i++)
        if (i < lived_gc_cycles.size)
            fprintf(sexp_inspector_lived_gc_cycles, "%i;%lu;%f\n", i, lived_gc_cycles.array[i],
                    100 * ((double) lived_gc_cycles.array[i]) / ((double) sexp_counter));
        else
            fprintf(sexp_inspector_lived_gc_cycles, "%i;%lu;%f\n", i, 0L, 0.);
}

void sexp_inspector_close() {
    if (active_analyses == 0)
        return;

    hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known, NULL);
    current_gc_cycle++;

    if (sexp_inspector_types != NULL) {
        write_out_types();
        fclose(sexp_inspector_types);
    }

    if (sexp_inspector_lived_gc_cycles != NULL) {
        write_out_lives();
        fclose(sexp_inspector_lived_gc_cycles);
    }

    if (sexp_inspector_debug != NULL)
        fclose(sexp_inspector_debug);
}

/*void print_header(SEXP sexp) {
    char *hr_type       = sexptype2char(sexp->sxpinfo.type);
    unsigned int scalar = sexp->sxpinfo.scalar;
    unsigned int obj    = sexp->sxpinfo.obj;
    unsigned int alt    = sexp->sxpinfo.alt;
    unsigned int gp     = sexp->sxpinfo.gp;
    unsigned int mark   = sexp->sxpinfo.mark;
    unsigned int debug  = sexp->sxpinfo.debug;
    unsigned int trace  = sexp->sxpinfo.trace;
    unsigned int spare  = sexp->sxpinfo.spare;
    unsigned int gcgen  = sexp->sxpinfo.gcgen;
    unsigned int gccls  = sexp->sxpinfo.gccls;
    unsigned int named  = sexp->sxpinfo.named;

    fprintf(sexp_inspector_log, "%s;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i",
            hr_type, scalar, obj, alt, gp, mark, debug, trace, spare, gcgen, gccls, named);
}*/

/*void print_fake_id(SEXP sexp) {
    hashmap_ret_t r = hashmap_get(fake_id_dictionary, (uintptr_t) sexp);
    if (r.status == MAP_OK)
        fprintf(sexp_inspector_log, "%ui;");
    else
        fprintf(sexp_inspector_log, ";");
}*/

/*void print_car_tag_and_cdr_fake_ids(SEXP sexp) {
    print_fake_id(sexp->u.listsxp.carval);
    print_fake_id(sexp->u.listsxp.tagval);
    print_fake_id(sexp->u.listsxp.cdrval);
}*/

/*void print_car_tag_and_cdr_addresses(SEXP sexp) {
    fprintf(fake_id_dictionary, "%p;%p;%p;",
            sexp->u.listsxp.carval,
            sexp->u.listsxp.tagval,
            sexp->u.listsxp.cdrval);
}*/

/*void print_body(SEXP sexp) {
    switch(sexp->sxpinfo.type) {
        case NILSXP:
        case SYMSXP:
        case CLOSXP:
        case PROMSXP:
        case EXTPTRSXP:
        case WEAKREFSXP:
            print_car_tag_and_cdr_fake_ids(sexp);
            print_car_tag_and_cdr_addresses(sexp);
            fprintf(sexp_inspector_log, ";;;");       // Nothing in data.
            break;

        case SPECIALSXP:
        case BUILTINSXP:
        case S4SXP:
            break;

        case LISTSXP:
        case LANGSXP:
        case DOTSXP:
            break;

        case ENVSXP:
            break;

        case CHARSXP:
            break;
        case LGLSXP:
            break;
        case INTSXP:
            fprintf(sexp_inspector_log, "%i;%i;", LENGTH(sexp), TRUELENGTH(sexp));
            for (int i = 0; i < XLENGTH(sexp); i++) {
                const int element = INTEGER(sexp)[i];
                const char *format = (i == 0) ? "%p" : ",%p";
                fprintf(sexp_inspector_log, format, element);
            }
            fprintf(sexp_inspector_log, ";");
            break;
        case REALSXP:
            break;
        case CPLXSXP:
            break;
        case STRSXP:
            break;
        case VECSXP:
            break;
        case EXPRSXP:
            break;
        case BCODESXP:
            break;
        case RAWSXP:
            break;
    }
}*/

// TODO investigate write barrier