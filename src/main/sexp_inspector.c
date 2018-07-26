#include <stdio.h>
#include "hashmap.h"

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

FILE *sexp_inspector_log;
map_t fake_id_dictionary;
unsigned long fake_id_sequence;


void sexp_inspector_init() {
    char *log_path = getenv("SEXP_INSPECTOR_LOG");
    if (log_path == NULL) {
        sexp_inspector_log = NULL;
        return;
    }

    sexp_inspector_log = fopen(log_path, "w");
    fprintf(sexp_inspector_log,
            "hook;address;fake_id;type;scalar;obj;alt;gp;mark;debug;trace;spare;gcgen;gccls;named\n");

    fake_id_dictionary = hashmap_new();
    fake_id_sequence = 0L;
}

void print_header(SEXP sexp) {
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
}

void sexp_inspector_allocation(SEXP sexp) {
    if (sexp_inspector_log == NULL)
        return;

    fake_id_sequence++;

    fprintf(sexp_inspector_log, "A;%p;%l;", sexp, fake_id_sequence);
    print_header(sexp);
    fprintf(sexp_inspector_log, "\n", sexp);

    int status = hashmap_put(fake_id_dictionary, (uintptr_t) sexp, fake_id_sequence);
}

void sexp_inspector_gc(SEXP sexp) {
    if (sexp_inspector_log == NULL)
        return;

    hashmap_ret_t r = hashmap_get(fake_id_dictionary, (uintptr_t) sexp);

    if (sexp->sxpinfo.mark == 1) {
        int status = hashmap_remove(fake_id_dictionary, (uintptr_t) sexp);
    }

    fprintf(sexp_inspector_log, "G;%p;%l;", sexp, r.value);
    print_header(sexp);
    fprintf(sexp_inspector_log, "\n", sexp);
}

void sexp_inspector_close() {
    if (sexp_inspector_log == NULL)
        return;
    fclose(sexp_inspector_log);
}

// TODO investigate write barrier
// NUM_OLD_GENERATIONS
// NUM_NODE_CLASSES
