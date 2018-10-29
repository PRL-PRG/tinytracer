#ifndef TINYTRACER_SEXP_INSPECTOR_COMPOSITION_H
#define TINYTRACER_SEXP_INSPECTOR_COMPOSITION_H

#include "sexp_inspector_r_connection.h"

int sexp_inspector_composition_is_running();
void sexp_inspector_composition_initialize();
void sexp_inspector_composition_close();
void sexp_inspector_composition_register(SEXP);
void sexp_inspector_composition_learn();
void sexp_inspector_composition_forget();

#endif //TINYTRACER_SEXP_INSPECTOR_COMPOSITION_H
