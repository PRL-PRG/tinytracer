#ifndef TINYTRACER_SEXP_INSPECTOR_SHARED_H
#define TINYTRACER_SEXP_INSPECTOR_SHARED_H

#include "sexp_inspector_r_connection.h"
void           sexp_inspector_initialize_fake_ids();
int            sexp_inspector_register_fake_id(SEXP);
unsigned long *sexp_inspector_retrieve_fake_id(SEXP);
int            sexp_inspector_remove_fake_id(SEXP);
unsigned long  sexp_inspector_count_fake_ids();

int sexp_inspector_are_there_analyses();

void sexp_inspector_bump_analysis_counter();
void sexp_inspector_bump_sexp_counter();

unsigned long sexp_inspector_read_sexp_counter();
unsigned long sexp_inspector_read_fake_id_sequence();
unsigned long sexp_inspector_count_registered_sexps();

typedef int (*sexp_iter)(SEXP, void *result);
void sexp_inspector_iterate_over_tracked_sexps(sexp_iter, void *result);

#endif //TINYTRACER_SEXP_INSPECTOR_SHARED_H
