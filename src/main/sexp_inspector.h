#ifndef SEXP_INSPECTOR_H
#define SEXP_INSPECTOR_H

#include "sexp_inspector_r_connection.h"

void sexp_inspector_init();
void sexp_inspector_allocation(SEXP);
void sexp_inspector_inspect_all_known();
void sexp_inspector_gc(SEXP);
void sexp_inspector_close();

#endif //SEXP_INSPECTOR_H
