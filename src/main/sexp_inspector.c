#include <stdio.h>

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

FILE *sexp_inspector_log;

void sexp_inspector_init() {
    char *log_path = getenv("SEXP_INSPECTOR_LOG");
    if (log_path == NULL) {
        sexp_inspector_log = NULL;
        return;
    }
    sexp_inspector_log = fopen(log_path, "a");

}

SEXP sexp_inspector_allocation(SEXP s) {
    if (sexp_inspector_log == NULL)
        return;
    SEXPTYPE type = TYPEOF(s);
    char *hr_type = sexptype2char(type);
    fprintf(sexp_inspector_log, "A;%p;%s\n", s, hr_type);
    return s;
}

void sexp_inspector_gc(SEXP sexp) {
    if (sexp_inspector_log == NULL)
        return;
    SEXPTYPE type = TYPEOF(sexp);
    char *hr_type = sexptype2char(type);
    fprintf(sexp_inspector_log, "G;%p;%s\n", sexp, hr_type);
}

void sexp_inspector_close() {
    if (sexp_inspector_log == NULL)
        return;
    fclose(sexp_inspector_log);
}

// TODO investigate write barrier
// NUM_OLD_GENERATIONS
// NUM_NODE_CLASSES
