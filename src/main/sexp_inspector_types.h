#ifndef TINYTRACER_SEXP_INSPECTOR_TYPES_H
#define TINYTRACER_SEXP_INSPECTOR_TYPES_H

#include "sexp_inspector_r_connection.h"

int sexp_inspector_types_is_running();
void sexp_inspector_types_initialize();
void sexp_inspector_types_close();
void sexp_inspector_types_increment(SEXP);

#endif //TINYTRACER_SEXP_INSPECTOR_TYPES_H
