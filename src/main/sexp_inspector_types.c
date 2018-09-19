#define USE_RINTERNALS

#include "sexp_inspector_shared.h"
#include "sexp_inspector_types.h"

#define NUM_TYPES (25+1)

FILE *sexp_inspector_types;
unsigned long type_counters[NUM_TYPES];

inline int sexp_inspector_types_is_running() {
    return sexp_inspector_types != NULL;
}

void sexp_inspector_types_initialize() {
    char *types_path = getenv("SEXP_INSPECTOR_TYPES");
    if (types_path == NULL) {
        sexp_inspector_types = NULL;
    } else {
        sexp_inspector_bump_analysis_counter();
        sexp_inspector_types = fopen(types_path, "w");
        fprintf(sexp_inspector_types, "type;type_name;count;percent\n");
    }
}

void sexp_inspector_types_close() {
    if (sexp_inspector_types_is_running()) {
        for (int i = 0; i < NUM_TYPES; i++)
            fprintf(sexp_inspector_types, "%i;%s;%lu;%f\n",
                    i,
                    sexptype2char(i),
                    type_counters[i],
                    100 * ((double) type_counters[i]) / ((double) sexp_inspector_read_sexp_counter()));
        fclose(sexp_inspector_types);
    }
}

void sexp_inspector_types_increment(SEXP sexp) {
    type_counters[TYPEOF(sexp)]++;
}