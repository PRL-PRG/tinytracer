#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "sexp_inspector_shared.h"
#include "sexp_inspector_composition.h"
#include "extensible_array.h"

typedef int trie_value_t;
typedef unsigned int trie_payload_t;

struct trie {
    trie_value_t value;
    struct trie *left;
    struct trie *right;
    union {
        struct trie *next_level;
        trie_payload_t counter;
    } payload;
    short int leaf;
};

struct trie *composition = NULL;

struct trie *new_trie(trie_value_t type) {
    struct trie *node = (struct trie *) malloc(sizeof(struct trie));
    if (node == NULL)
        printf("ERROR: new trie cannot allocate memory");
    node->left = NULL;
    node->right = NULL;
    node->value = type;
    return node;
}

struct trie *new_trie_with_value(trie_value_t type, trie_payload_t value) {
    struct trie *node = new_trie(type);
    node->payload.counter = value;
    node->leaf = 1;
    return node;
}

struct trie *new_trie_with_level_ptr(trie_value_t type, struct trie *next_level) {
    struct trie *node = new_trie(type);
    node->payload.next_level = next_level;
    node->leaf = 0;
    return node;
}

// assuming level_root != NULL
struct trie *find_or_create_node_within_level(struct trie *level_root, trie_value_t type) {
    struct trie *current = level_root;
    while(1)
        if (current->value == type)
            return current;
        else
            if (current->value < type) {
                if (current->left == NULL) {
                    current->left = new_trie_with_level_ptr(type, NULL);
                    return current->left;
                } else {
                    current = current->left;
                }
            } else /*current->value > type*/ {
                if (current->right == NULL) {
                    current->right = new_trie_with_level_ptr(type, NULL);
                    return current->right;
                } else {
                    current = current->right;
                }
            }
}

void recursive_traverse(FILE *file, void (*f)(FILE *file, trie_value_t[], int, int), struct trie *elem, trie_value_t values[], int levels) {
    if(elem == NULL)
        return;

    recursive_traverse(file, f, elem->left, values, levels);
    values[levels] = elem->value;
    if(elem->leaf)
        (*f)(file, values, levels, elem->payload.counter);
    else
        recursive_traverse(file, f, elem->payload.next_level, values, levels + 1);
    recursive_traverse(file, f, elem->right, values, levels);
}

struct trie *create_new_simple_tree(trie_value_t values[], int offset, int length) {
    struct trie *tree = NULL;
    for (int i = length - 1; i >= offset; i--) {
        if (tree == NULL)
            tree = new_trie_with_value(values[i], 1);
        else
            tree = new_trie_with_level_ptr(values[i], tree);
    }
    return tree;
}

void traverse_and_increment(struct trie *elem, trie_value_t values[], int offset, int length) {
    if (elem->leaf) {
        elem->payload.counter++;
        return;
    }

    struct trie *leaf = find_or_create_node_within_level(elem, values[offset]);
    if (leaf->payload.next_level == NULL)
        leaf->payload.next_level = create_new_simple_tree(values, offset + 1, length);
    else
        traverse_and_increment(leaf->payload.next_level, values, offset + 1, length);
}


void increment(trie_value_t values[], int levels) {
    if (composition == NULL)
        composition = create_new_simple_tree(values, 0, levels);
    else
        traverse_and_increment(composition, values, 0, levels);
}

unsigned long counts_in_type[26];
int composition_analysis_is_running = 0;
char *composition_path;

inline int sexp_inspector_composition_is_running() {
    return composition_analysis_is_running != 0;
}

void sexp_inspector_composition_initialize() {
    // Initialize an output file for SEXP composition analysis, if path is provided.
    char *composition_path_fmt = getenv("SEXP_INSPECTOR_COMPOSITION");
    if (composition_path_fmt == NULL) {
        composition_analysis_is_running = 0;
    } else {
        sexp_inspector_bump_analysis_counter();
        composition_analysis_is_running = 1;
        composition_path = (char *) calloc(128, sizeof(char));
        pid_t pid = getpid();
        int result = sprintf(composition_path, composition_path_fmt, pid);
        if(result < 0) {
            printf("BELGIUM: (tinytracer) sprintf returned %d, "
                           "turning off analysis", result);
            composition_analysis_is_running = 0;
        }
    }
}

char *trie_value_to_string(trie_value_t v) {
    switch(v) {
        case NILSXP:     return "NILSXP";     /* 0 */
        case SYMSXP:     return "SYMSXP";     /* 1 */
        case LISTSXP:    return "LISTSXP";    /* 2 */
        case CLOSXP:     return "CLOSXP";     /* 3 */
        case ENVSXP:     return "ENVSXP";     /* 4 */
        case PROMSXP:    return "PROMSXP";    /* 5 */
        case LANGSXP:    return "LANGSXP";    /* 6 */
        case SPECIALSXP: return "SPECIALSXP"; /* 7 */
        case BUILTINSXP: return "BUILTINSXP"; /* 8 */
        case CHARSXP:    return "CHARSXP";    /* 9 */
        case LGLSXP:     return "LGLSXP";     /* 10 */
        case 11:         return "<11>";       /* defunct */
        case 12:         return "<12>";       /* defunct */
        case INTSXP:     return "INTSXP";     /* 13 */
        case REALSXP:    return "REALSXP";    /* 14 */
        case CPLXSXP:    return "CPLXSXP";    /* 15 */
        case STRSXP:     return "STRSXP";     /* 16 */
        case DOTSXP:     return "DOTSXP";     /* 17 */
        case ANYSXP:     return "ANYSXP";     /* 18 */
        case VECSXP:     return "VECSXP";     /* 19 */
        case EXPRSXP:    return "EXPRSXP";    /* 20 */
        case BCODESXP:   return "BCODESXP";   /* 21 */
        case EXTPTRSXP:  return "EXTPTRSXP";  /* 22 */
        case WEAKREFSXP: return "WEAKREFSXP"; /* 23 */
        case RAWSXP:     return "RAWSXP";     /* 24 */
        case S4SXP:      return "S4SXP";      /* 25 */
        case 26:         return "<26>";       /* unknown */
        case 27:         return "<27>";       /* unknown */
        case 28:         return "<28>";       /* unknown */
        case 29:         return "<29>";       /* unknown */
        case NEWSXP:     return "NEWSXP";     /* 30 */
        case FREESXP:    return "FREESXP";    /* 31 */
        case -1:         return "<missing>";
        default:         return "<unknown>";
    }
}

typedef enum {
    SEXP_EMPTY, SEXP_TRIPLE, SEXP_VECTOR, SEXP_WEAKREF,
    SEXP_EXTERNAL, SEXP_OTHER, SEXP_OFFSET
} sexp_classification_t;

sexp_classification_t classify_sexp(SEXPTYPE type) {
    switch(type) {
        case NILSXP:
        case S4SXP:/* TODO attrib analysis */
            return SEXP_EMPTY;

        case SYMSXP:
        case LISTSXP:
        case CLOSXP:
        case ENVSXP:
        case PROMSXP:
        case LANGSXP:
        case DOTSXP:
        case BCODESXP:/* TODO something fun */
            return SEXP_TRIPLE;

        case LGLSXP:
        case INTSXP:
        case REALSXP:
        case CPLXSXP:
        case STRSXP:
        case VECSXP:
        case EXPRSXP:
        case RAWSXP:
        case CHARSXP:
            return SEXP_VECTOR;

        case EXTPTRSXP:
            return SEXP_EXTERNAL;

        case WEAKREFSXP:
            return SEXP_WEAKREF;

        case ANYSXP:
            return SEXP_OTHER;

        case SPECIALSXP:
        case BUILTINSXP:
            return SEXP_OFFSET;
    }
}

//double percent_of(int count, int total) {
//    return 100 * (double) count / (double) total;
//}

void print_composition(FILE *file, trie_value_t values[], int levels, int payload) {
    trie_value_t type = values[0];
    switch(classify_sexp(type)) {
        case SEXP_EMPTY:
            fprintf(file, "%s,NA,NA,NA,%i\n",
                    trie_value_to_string(type),
                    payload);
            break;

        case SEXP_TRIPLE:
            fprintf(file, "%s,%s,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[3]),
                    payload);
            break;

        case SEXP_VECTOR:
            fprintf(file, "%s,%i,%i,NA,%i\n",
                    trie_value_to_string(type),
                    values[1],
                    values[2],
                    payload);
            break;

        case SEXP_EXTERNAL:
            fprintf(file, "%s,NA,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[2]),
                    payload);
            break;

        case SEXP_WEAKREF:
            fprintf(file, "%s,%s,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[3]),
                    payload);
            break;

        case SEXP_OFFSET:
            fprintf(file, "%s,%i,NA,NA,%i\n",
                    trie_value_to_string(type),
                    values[1],
                    payload);
            break;

        case SEXP_OTHER:
            fprintf(file, "%s,", trie_value_to_string(type));
            for (int i = levels; i > 0; i--)
                fprintf(file, "%s,", trie_value_to_string(values[i]));
            for (int i = 4 - 1; i > levels; i--)
                fprintf(file, ",");
            fprintf(file, "%i\n", payload);
            break;
    }
}

void sexp_inspector_composition_close() {
    if (!sexp_inspector_composition_is_running())
        return;

    trie_value_t values[4];
    FILE *file = fopen(composition_path, "w");
    fprintf(file, "type,car_type,tag_type,cdr_type,count\n");
    recursive_traverse(file, print_composition, composition, values, 0);
    fclose(file);
}

trie_value_t  sexp_to_trie_value(SEXP sexp){
    //trie_value_t r = (sexp == NULL) ? -1 : TYPEOF(sexp);
    //if (r > 25)
        //printf("##### %d\n", r);
    return (sexp == NULL) ? -1 : TYPEOF(sexp);
}

trie_value_t to_log(int v) {
    return (v == 0) ? 0 : (8 * sizeof(int) - __builtin_clz(v));
}

void sexp_inspector_composition_register(SEXP sexp) {
    counts_in_type[TYPEOF(sexp)]++;
    switch(classify_sexp(TYPEOF(sexp))) {
        case SEXP_EMPTY:
            increment((trie_value_t[]){sexp_to_trie_value(sexp)}, 1);
            break;

        case SEXP_TRIPLE:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       sexp_to_trie_value(TAG(sexp)),
                                       sexp_to_trie_value(CDR(sexp)),
                                       sexp_to_trie_value(CAR(sexp))}, 4);
            break;

        case SEXP_VECTOR:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       to_log(((VECSEXP) sexp)->vecsxp.length),
                                       to_log(((VECSEXP) sexp)->vecsxp.truelength)}, 3);
            break;

        case SEXP_EXTERNAL:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       sexp_to_trie_value(TAG(sexp)),
                                       sexp_to_trie_value(CDR(sexp))}, 3);
            break;

        case SEXP_WEAKREF:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       sexp_to_trie_value(VECTOR_ELT(sexp, 0)),
                                       sexp_to_trie_value(VECTOR_ELT(sexp, 1)),
                                       sexp_to_trie_value(VECTOR_ELT(sexp, 2))}, 4);
            break;

        case SEXP_OFFSET:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       PRIMOFFSET(sexp)}, 3);
            break;

        case SEXP_OTHER:
            increment((trie_value_t[]){sexp_to_trie_value(sexp),
                                       sexp_to_trie_value(CAR(sexp)),
                                       sexp_to_trie_value(TAG(sexp)),
                                       sexp_to_trie_value(CDR(sexp))}, 4);
            break;
    }
}