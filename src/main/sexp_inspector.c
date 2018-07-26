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
    sexp_inspector_log = fopen(log_path, "w");
    fprintf(sexp_inspector_log, "hook;address;type;scalar;obj;alt;gp;mark;debug;trace;spare;gcgen;gccls;named\n");
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

    fprintf(sexp_inspector_log, "A;%p;", sexp);
    print_header(sexp);
    fprintf(sexp_inspector_log, "\n", sexp);
}

void sexp_inspector_gc(SEXP sexp) {
    if (sexp_inspector_log == NULL)
        return;

    fprintf(sexp_inspector_log, "G;%p;", sexp);
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
