#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "sexp_inspector_shared.h"
#include "sexp_inspector_composition.h"
#include "hashmap.h"

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
struct trie *find_or_create_node_within_level(struct trie *level_root, trie_value_t values[], int offset, int length) {
    struct trie *current = level_root;
    while(1)
        if (current->value == values[offset])
            return current;
        else
            if (current->value < values[offset]) {
                if (current->left == NULL) {
                    if (offset == length - 1)
                        current->left = new_trie_with_value(values[offset], 0);
                    else
                        current->left = new_trie_with_level_ptr(values[offset], NULL);
                    return current->left;
                } else {
                    current = current->left;
                }
            } else /*current->value > type*/ {
                if (current->right == NULL) {
                    if (offset == length - 1)
                        current->right = new_trie_with_value(values[offset], 0);
                    else
                        current->right = new_trie_with_level_ptr(values[offset], NULL);
                    return current->right;
                } else {
                    current = current->right;
                }
            }
}

void print_prefix(int level){
    for (int i = 0; i < level; i++)
        printf("    ");
}

void debug_trie(struct trie *elem, short int end_line, int level) {
    if(elem == NULL) {
        printf("null");
        return;
    }

    printf("{value=%i, leaf=%i, \n", elem->value, elem->leaf);

    print_prefix(level); printf("           left=");
    debug_trie(elem->left, 0, level+1);
    printf(",\n");

    print_prefix(level); printf("           right=");
    debug_trie(elem->right, 0, level+1);
    printf(",\n");

    if (elem->leaf) {
        print_prefix(level); printf("           payload.counter=%i}", elem->payload.counter);
    } else {
        print_prefix(level); printf("           payload.next_level=");
        debug_trie(elem->payload.next_level, 0, level+1);
        printf("}");
    }

    if (end_line) {
        printf("\n");
    }
}

void recursive_traverse(FILE *file, void (*f)(FILE *file, trie_value_t[], int, int),
                        struct trie *elem, trie_value_t values[], int levels) {
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
    struct trie *leaf = find_or_create_node_within_level(elem, values, offset, length);
    if (leaf->leaf) {
        //assert(offset == length - 1);
        leaf->payload.counter++;
    } else if (leaf->payload.next_level == NULL) {
        leaf->payload.next_level = create_new_simple_tree(values, offset + 1, length);
    } else {
        traverse_and_increment(leaf->payload.next_level, values, offset + 1, length);
    }
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
            fprintf(stderr, "BELGIUM: (tinytracer) sprintf returned %d, "
                           "turning off analysis", result);
            composition_analysis_is_running = 0;
        }

    }
}

//char *identify_environment(trie_value_t v) {
//    if (v == R_GlobalEnv)
//        return "ENVSXP[R_GlobalEnv]";
//    if (v == R_EmptyEnv)
//        return "ENVSXP[R_EmptyEnv]";
//    if (v == R_BaseEnv)
//        return "ENVSXP[R_BaseEnv]";
//    if (v == R_BaseNamespace)
//        return "ENVSXP[R_BaseNamespace]";
//    if (v == R_NamespaceRegistry)
//        return "ENVSXP[R_NamespaceRegistry]";
//    return "ENVSXP";
//}
//
//char *identify_symbol(trie_value_t v) {
//    if (v == R_UnboundValue)
//        return "SYMSXP[R_UnboundValue]";
//    if (v == R_MissingArg)
//        return "SYMSXP[R_MissingArg]";
//    return "SYMSXP";
//}

char *trie_value_to_string(trie_value_t v) {
    switch(v) {
        case NILSXP:     return "NILSXP";     /* 0 */
        case -2:         return "NILSXP[R_NilValue]";
        case SYMSXP:     return "SYMSXP";     /* 1 */
        case -10:        return "SYMSXP[R_UnboundValue]";
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
    SEXP_EXTERNAL, SEXP_OTHER, SEXP_OFFSET, SEXP_S4,
    //SEXP_SYMSXP, SEXP_CHARSXP,
    SEXP_BCODE
} sexp_classification_t;

sexp_classification_t classify_sexp(SEXPTYPE type) {
    switch(type) {
        case -2:
        case NILSXP:
            return SEXP_EMPTY;

        case S4SXP:/* TODO attrib analysis */
            return SEXP_S4;

        case -10:       /* SYMSXP[R_UnboundValue] */
        case SYMSXP:
        case LISTSXP:
        case CLOSXP:
        case ENVSXP:
        case PROMSXP:
        case LANGSXP:
        case DOTSXP:
            return SEXP_TRIPLE;

        case BCODESXP:/* TODO something fun */
            return SEXP_BCODE;

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
            fprintf(file, "%s,NA,NA,NA,NA,%i\n",
                    trie_value_to_string(type),
                    payload);
            break;

        case SEXP_S4:
            fprintf(file, "%s,%s,NA,NA,NA,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    payload);
            break;

        case SEXP_TRIPLE:
            fprintf(file, "%s,%s,%s,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[4]), // ATTRIB 1 -> 1, TAG 2 -> 3, CDR 3 -> 4, CAR 4 -> 2
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[3]),
                    payload);
            break;

        case SEXP_BCODE:
            fprintf(file, "%s,%s,%s,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[4]), // ATTRIB 1 -> 1, TAG 2 -> 3, CDR 3 -> 4, CAR 4 -> 2
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[3]),
                    payload);
            break;

        case SEXP_VECTOR:
            fprintf(file, "%s,%s,%i,%i,%i,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    values[3],
                    values[4],
                    values[2],                       // ALT bit
                    payload);
            break;

        case SEXP_EXTERNAL:
            fprintf(file, "%s,%s,NA,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[3]),
                    payload);
            break;

        case SEXP_WEAKREF:
            fprintf(file, "%s,%s,%s,%s,%s,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    trie_value_to_string(values[2]),
                    trie_value_to_string(values[3]),
                    trie_value_to_string(values[4]),
                    payload);
            break;

        case SEXP_OFFSET:
            fprintf(file, "%s,%s,%i,NA,NA,%i\n",
                    trie_value_to_string(type),
                    trie_value_to_string(values[1]),
                    values[2],
                    payload);
            break;

        case SEXP_OTHER:
            fprintf(file, "%s,", trie_value_to_string(type));
            for (int i = levels; i > 0; i--)
                fprintf(file, "%s,", trie_value_to_string(values[i]));
            for (int i = 5 - 1; i > levels; i--)
                fprintf(file, ",");
            fprintf(file, "%i\n", payload);
            break;
    }
}

int register_just_the_one_straggler(SEXP straggler, void *result) {
    sexp_inspector_composition_register(straggler);
    return 0;
}

void register_stragglers() {
    sexp_inspector_iterate_over_tracked_sexps(register_just_the_one_straggler, NULL);
}

void write_out_data() {
    trie_value_t values[4];
    FILE *file = fopen(composition_path, "w");
    if (file == NULL) {
        perror(NULL);
        exit(1);
    }
    fprintf(file, "type,attrib_type,car_type,tag_type,cdr_type,count\n");
    recursive_traverse(file, print_composition, composition, values, 0);
    fclose(file);
}

void sexp_inspector_composition_close() {
    if (!sexp_inspector_composition_is_running())
        return;

    sexp_inspector_composition_learn();
    register_stragglers();
    sexp_inspector_composition_forget();
    write_out_data();
}

trie_value_t  sexp_to_trie_value(SEXP sexp){
    if (sexp == NULL)
        return -1;
    if (sexp == R_NilValue)
        return -2;
    if (sexp == R_UnboundValue)
        return -10;
    return TYPEOF(sexp);
}

trie_value_t to_log(int v) {
    return (v == 0) ? 0 : (8 * sizeof(int) - __builtin_clz(v));
}

struct sexp_memory_cell_t{
    int type;
    int attrib;
    union {
        struct {
            int car;
            int cdr;
            int tag;
        } triple;
        struct {
            int length;
            int true_length;
            int alt_bit;
        } vector;
        struct {
            int key;
            int value;
            int finalizer;
        } weakref;
        struct {
            int offset;
        } prim;
    } body;
    int count_this;
};

//struct sexp_memory_cell_t *memory;
map_t memory;

void sexp_inspector_composition_register(SEXP sexp) {
    counts_in_type[TYPEOF(sexp)]++;

    hashmap_ret_t r = hashmap_get(memory, (uintptr_t) sexp);

    if (r.status != MAP_OK) {
        /*
         * This happens because the SEXP that we see in the GC loop is a technically uninitialized area of memory that
         * was allocated inside GetNewPage as part of CLASS_GET_FREE_NODE. GetNewPage allocates a whole new empty page
         * and splits it into SEXP-sized regions according to some specific size class. Then, the SEXPs are given empty
         * headers from a template and snapped into a GC ring for the appropriate size class. Then, when
         * CLASS_GET_FREE_NODE is called it quickly returns one of these pre-prepared ur-SEXPs.
         *
         * Since these ur-SEXPs are a part of a GC ring, they show up as free memory when we loop through GC rings. But
         * since they have not yet gone through allocation, we have not assigned them a fake id nor do they show up in
         * our "memory" hashmap. Thus, we get a map miss here.
         *
         * The appropriate course of action is to ignore these, since they are not actually SEXPs.
         */
        return;
    }

    struct sexp_memory_cell_t *memory_cell = (struct sexp_memory_cell_t *) r.value;

    switch(classify_sexp(TYPEOF(sexp))) {
        case SEXP_EMPTY:
            increment((trie_value_t[]){memory_cell->type}, 1);
            break;

        case SEXP_S4:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib}, 2);
            break;

        case SEXP_TRIPLE:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.triple.tag,
                                       memory_cell->body.triple.cdr,
                                       memory_cell->body.triple.car}, 5);
            break;

        case SEXP_BCODE:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.triple.tag,
                                       memory_cell->body.triple.cdr,
                                       memory_cell->body.triple.car}, 5);
            break;

        case SEXP_VECTOR:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.vector.alt_bit,
                                       memory_cell->body.vector.length,
                                       memory_cell->body.vector.true_length}, 5);
            break;

        case SEXP_EXTERNAL:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.triple.tag,
                                       memory_cell->body.triple.cdr}, 4);
            break;

        case SEXP_WEAKREF:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.weakref.key,
                                       memory_cell->body.weakref.value,
                                       memory_cell->body.weakref.finalizer}, 5);
            break;

        case SEXP_OFFSET:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.prim.offset}, 3);
            break;

        case SEXP_OTHER:
            increment((trie_value_t[]){memory_cell->type,
                                       memory_cell->attrib,
                                       memory_cell->body.triple.car,
                                       memory_cell->body.triple.tag,
                                       memory_cell->body.triple.cdr}, 5);
            break;

        default:
            fprintf(stderr, "BELGIUM: unknown type %i/%i\n",
                    TYPEOF(sexp), classify_sexp(TYPEOF(sexp)));
    }
}


int learn_about_just_the_one_sexp(SEXP sexp, void *extra) {
    //int *pointer = (int *) extra;

    struct sexp_memory_cell_t *memory_cell =
            (struct sexp_memory_cell_t *) malloc(sizeof(struct sexp_memory_cell_t));

    if (memory_cell == NULL)
        fprintf(stderr, "BELGIUM: did not allocate memory cell for SEXP data\n");

    memory_cell->type = sexp_to_trie_value(sexp);
    memory_cell->count_this = 0;
    switch(classify_sexp(TYPEOF(sexp))) {
        case SEXP_EMPTY:
            memory_cell->attrib = -1;
            memory_cell->body.triple.car = -1;
            memory_cell->body.triple.cdr = -1;
            memory_cell->body.triple.tag = -1;
            break;

        case SEXP_S4:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.triple.car = -1;
            memory_cell->body.triple.cdr = -1;
            memory_cell->body.triple.tag = -1;
            break;

        case SEXP_TRIPLE:
        case SEXP_BCODE:
        case SEXP_OTHER:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.triple.car = sexp_to_trie_value(CAR(sexp));
            memory_cell->body.triple.cdr = sexp_to_trie_value(CDR(sexp));
            memory_cell->body.triple.tag = sexp_to_trie_value(TAG(sexp));
            break;

        case SEXP_VECTOR:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.vector.length = to_log(((VECSEXP) sexp)->vecsxp.length);
            memory_cell->body.vector.true_length = to_log(((VECSEXP) sexp)->vecsxp.truelength);
            memory_cell->body.vector.alt_bit = ((VECSEXP) sexp)->sxpinfo.alt;
            break;

        case SEXP_EXTERNAL:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.triple.car = -1;
            memory_cell->body.triple.cdr = sexp_to_trie_value(CDR(sexp));
            memory_cell->body.triple.tag = sexp_to_trie_value(TAG(sexp));
            break;

        case SEXP_WEAKREF:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.weakref.key = sexp_to_trie_value(VECTOR_ELT(sexp, 0));
            memory_cell->body.weakref.value = sexp_to_trie_value(VECTOR_ELT(sexp, 1));
            memory_cell->body.weakref.finalizer = sexp_to_trie_value(VECTOR_ELT(sexp, 2));
            //memory_cell->body.weakref.next = sexp_to_trie_value(VECTOR_ELT(sexp, 3));
            break;

        case SEXP_OFFSET:
            memory_cell->attrib = sexp_to_trie_value(ATTRIB(sexp));
            memory_cell->body.prim.offset = PRIMOFFSET(sexp);
            break;

        default:
            fprintf(stderr, "BELGIUM: unknown type %i/%i\n",
                    TYPEOF(sexp), classify_sexp(TYPEOF(sexp)));
    }

    hashmap_put(memory, (uintptr_t) sexp, memory_cell);
    //(*pointer)++;
    return 0;
}

void sexp_inspector_composition_learn() {
    memory = hashmap_new("memory");
    //int memory_pointer = 0;
    if (memory == NULL)
        fprintf(stderr, "BELGIUM: did not allocate memory for SEXP data\n");

    sexp_inspector_iterate_over_tracked_sexps(learn_about_just_the_one_sexp, NULL); //&memory_pointer);

    // FIXME duplicates ?
}

//void forget_about_just_the_sexp(SEXP sexp, void *extra) {
//    hashmap_remove(memory, (uintptr_t) sexp, 1);
//}

void sexp_inspector_composition_forget() {
    //sexp_inspector_iterate_over_tracked_sexps(forget_about_just_the_sexp, NULL);
    hashmap_clear(memory);
    hashmap_free(memory);
    memory = NULL;
}

void sexp_inspector_composition_note_allocation(SEXP sexp) {

}