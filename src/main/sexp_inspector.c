#include <stdio.h>
#include "hashmap.h"

#define USE_RINTERNALS

#include "../include/Rinternals.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

unsigned int analyses = 0;
FILE *sexp_inspector_types;
FILE *sexp_inspector_debug;
map_t fake_id_dictionary;
unsigned long fake_id_sequence;
unsigned long inspection_clock;

// type counter
unsigned long sexp_counter;
unsigned long type_counters[25+1];

typedef struct {
    // metadata
    SEXP          address;
    unsigned long fake_id;
    unsigned long initial_inspection_clock;
    unsigned long final_inspection_clock;
    // header
    unsigned int  sexp_type;
    unsigned int  scalar;
    unsigned int  obj;
    unsigned int  alt;
    unsigned int  gp;
    unsigned int  mark;
    unsigned int  debug;
    unsigned int  trace;
    unsigned int  spare;
    unsigned int  gcgen;
    unsigned int  gccls;
    unsigned int  named;
} sexp_info;

void sexp_inspector_init() {
    char *types_path = getenv("SEXP_INSPECTOR_TYPES");
    if (types_path == NULL) {
        sexp_inspector_types = NULL;
    } else {
        analyses++;
        sexp_inspector_types = fopen(types_path, "w");
        fprintf(sexp_inspector_types, "type;type_name;count;percent\n");
    }

    //fprintf(sexp_inspector_log,
    //        "inspection_clock;address;fake_id;sexp_type;scalar;obj;alt;gp;mark;debug;trace;spare;gcgen;gccls;named;"
    //                "car_id;tag_id;cdr_id;car_address;tag_address;cdr_address;data_length;data_truelength;data;\n")

    if (analyses > 0) {
        char *debug_path = getenv("SEXP_INSPECTOR_DEBUG");
        sexp_inspector_debug = fopen(debug_path, "w");

        fake_id_dictionary = hashmap_new();
        fake_id_sequence = 0L;
        inspection_clock = 0L;
    }
}

void sexp_inspector_allocation(SEXP sexp) {
    if (analyses == 0)
        return;

    fake_id_sequence++;
    int status = hashmap_put(fake_id_dictionary, (uintptr_t) sexp, fake_id_sequence);

    type_counters[TYPEOF(sexp)]++;
    sexp_counter++;

    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "allocating inspection_clock=%i sexp=%p fake_id=%lu\n",
                inspection_clock, (uintptr_t) sexp, fake_id_sequence);
}

int sexp_inspector_inspect_one_known(hashmap_key_t sexp, hashmap_val_t fake_id) {

    //fprintf(sexp_inspector_log, "%lu;%p;%lu;", inspection_clock, (uintptr_t) sexp, fake_id);
    //print_header((SEXP) sexp);
    //print_body((SEXP) sexp);
    //fprintf(sexp_inspector_log, "\n");

    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "inspecting inspection_clock=%i sexp=%p fake_id=%lu\n",
                inspection_clock, (uintptr_t) sexp, fake_id);

    return MAP_OK;
}

void sexp_inspector_inspect_all_known() {
    if (analyses == 0)
        return;

    hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known);
    inspection_clock++;
}

void sexp_inspector_gc(SEXP sexp) {
    if (analyses == 0)
        return;

    hashmap_ret_t r = hashmap_get(fake_id_dictionary, (uintptr_t) sexp);
    unsigned long fake_id = r.status == MAP_OK ? r.value : 0;

    int status = hashmap_remove(fake_id_dictionary, (uintptr_t) sexp);

    if (sexp_inspector_debug != NULL)
        fprintf(sexp_inspector_debug, "gc unmark inspection_clock=%i sexp=%p fake_id=%lu\n",
                inspection_clock, (uintptr_t) sexp, fake_id);
}

void write_out_types() {
    for (int i = 0; i < 25+1; i++)
        fprintf(sexp_inspector_types, "%i;%s;%u;%f\n", i, sexptype2char(i),
                type_counters[i], 100 * ((double) type_counters[i]) / ((double) sexp_counter));
}

void sexp_inspector_close() {
    if (analyses == 0)
        return;

    hashmap_iterate(fake_id_dictionary, sexp_inspector_inspect_one_known);
    inspection_clock++;

    if (sexp_inspector_types != NULL) {
        write_out_types();
        fclose(sexp_inspector_types);
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