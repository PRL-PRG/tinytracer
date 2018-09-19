#include <stdio.h>
#include <stdlib.h>

#include "sexp_inspector_shared.h"
#include "sexp_inspector_debug.h"

FILE *sexp_inspector_debug;

void sexp_inspector_debug_initialize() {
    char *debug_path = getenv("SEXP_INSPECTOR_DEBUG");
    if (debug_path != NULL)
        sexp_inspector_debug = fopen(debug_path, "w");
}

inline int sexp_inspector_debug_is_running() {
    return (sexp_inspector_debug != NULL);
}

void sexp_inspector_debug_note_allocation(SEXP sexp) {
    fprintf(sexp_inspector_debug,
            "allocating sexp=%p fake_id=%lu\n",
            (void *) sexp,
            sexp_inspector_read_fake_id_sequence());
}

void sexp_inspector_debug_note_gc_unmark(SEXP sexp, unsigned long fake_id) {
    fprintf(sexp_inspector_debug,
            "gc unmark sexp=%p fake_id=%lu\n",
            (void *) sexp,
            fake_id);
}

void sexp_inspector_debug_note_unknown_SEXP(SEXP sexp) {
    fprintf(sexp_inspector_debug,
            "gc unknown SEXP sexp=%p \n",
            (void *) sexp);
}

void sexp_inspector_debug_close() {
    if (sexp_inspector_debug != NULL)
        fclose(sexp_inspector_debug);
}