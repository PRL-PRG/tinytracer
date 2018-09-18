#ifndef TINYTRACER_SEXP_INSPECTOR_LIVES_H
#define TINYTRACER_SEXP_INSPECTOR_LIVES_H

//#include "sexp_inspector_r_connection.h"

int sexp_inspector_lives_is_running();
void sexp_inspector_lives_initialize();
void sexp_inspector_lives_increment(unsigned int initial_gc_cycle);
void sexp_inspector_lives_close();

#endif //TINYTRACER_SEXP_INSPECTOR_LIVES_H
