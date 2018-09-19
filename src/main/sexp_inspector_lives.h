#ifndef TINYTRACER_SEXP_INSPECTOR_LIVES_H
#define TINYTRACER_SEXP_INSPECTOR_LIVES_H

#include "sexp_inspector_r_connection.h"

int  sexp_inspector_lives_is_running();
void sexp_inspector_lives_initialize();
int  sexp_inspector_lives_register(SEXP);
void sexp_inspector_lives_new_gc_cycle();
void sexp_inspector_lives_gc_unmark(SEXP);
void sexp_inspector_lives_close();

#endif //TINYTRACER_SEXP_INSPECTOR_LIVES_H
