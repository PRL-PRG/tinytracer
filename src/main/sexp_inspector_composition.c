#define USE_RINTERNALS

#include "sexp_inspector_shared.h"
#include "sexp_inspector_composition.h"

FILE *sexp_inspector_composition;
unsigned long type_compositions[25+1][25+1][25+1][25+1];

inline int sexp_inspector_composition_is_running() {
    return sexp_inspector_composition != NULL;
}

void sexp_inspector_composition_initialize() {
    // Initialize an output file for SEXP composition analysis, if path is provided.
    char *composition_path = getenv("SEXP_INSPECTOR_COMPOSITION");
    if (composition_path == NULL) {
        sexp_inspector_composition = NULL;
    } else {
        sexp_inspector_bump_analysis_counter();
        sexp_inspector_composition = fopen(composition_path, "w");
        fprintf(sexp_inspector_composition, "type;car_type;tag_type;cdr_type;count;percent\n");
    }
}

void sexp_inspector_composition_close() {
    if (sexp_inspector_composition_is_running()) {
        for (int i = 0; i < 25+1; i++)
            for (int car_i = 0; car_i < 25+1; car_i++)
                for (int tag_i = 0; tag_i < 25+1; tag_i++)
                    for (int cdr_i = 0; cdr_i < 25+1; cdr_i++) {
                        unsigned long count = type_compositions[i][car_i][tag_i][cdr_i];
                        if (count == 0)
                            continue;
                        fprintf(sexp_inspector_composition, "%s;%s;%s;%s;%lu;%f\n",
                                sexptype2char(i),
                                sexptype2char(car_i),
                                sexptype2char(tag_i),
                                sexptype2char(cdr_i),
                                count,
                                100 * ((double) count) / ((double) sexp_inspector_read_sexp_counter()));
                    }
        fclose(sexp_inspector_composition);
    }
}


void sexp_inspector_composition_register(SEXP sexp) {
    switch(TYPEOF(sexp)) {
        case NILSXP:
            type_compositions[TYPEOF(sexp)][11][11][11]++;
            break;

        case SYMSXP:
        case LISTSXP:
        case CLOSXP:
        case ENVSXP:
        case PROMSXP:
        case LANGSXP:
        case DOTSXP:
            type_compositions[TYPEOF(sexp)]
            [(CAR(sexp) != NULL) ? TYPEOF(CAR(sexp)) : 11]
            [(TAG(sexp) != NULL) ? TYPEOF(TAG(sexp)) : 11]
            [(TAG(sexp) != NULL) ? TYPEOF(CDR(sexp)) : 11]++;
            break;

        case LGLSXP:
        case INTSXP:
        case REALSXP:
        case CPLXSXP:
        case STRSXP:
        case VECSXP:
        case EXPRSXP:
        case RAWSXP:

            break;

        case BCODESXP:
            break;

        case ANYSXP:
            break;

        case EXTPTRSXP:
            break;

        case WEAKREFSXP:
            break;

        case S4SXP:
            break;

        case SPECIALSXP:
        case BUILTINSXP:
        case CHARSXP:
            break;
    }
}