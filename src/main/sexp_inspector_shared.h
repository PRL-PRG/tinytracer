#ifndef TINYTRACER_SEXP_INSPECTOR_SHARED_H
#define TINYTRACER_SEXP_INSPECTOR_SHARED_H

#include "sexp_inspector_r_connection.h"

typedef struct {
    unsigned long fake_id;
    unsigned int  initial_gc_cycle;
} *t_sexp_info;

void          sexp_inspector_initialize_fake_ids();
int           sexp_inspector_register_fake_id(SEXP);
t_sexp_info   sexp_inspector_retrieve_fake_id(SEXP);
int           sexp_inspector_remove_fake_id(SEXP);

void *hack_get_fake_id_dictionary_do_not_use(); // FIXME

int sexp_inspector_are_there_analyses();

void sexp_inspector_bump_analysis_counter();
void sexp_inspector_bump_sexp_counter();
void sexp_inspector_bump_gc_counter();

unsigned long sexp_inspector_read_sexp_counter();
unsigned int  sexp_inspector_read_gc_counter();
unsigned long sexp_inspector_read_fake_id_sequence();

#endif //TINYTRACER_SEXP_INSPECTOR_SHARED_H
