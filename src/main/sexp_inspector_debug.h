#ifndef TINYTRACER_SEXP_INSPECTOR_DEBUG_H
#define TINYTRACER_SEXP_INSPECTOR_DEBUG_H

#include "sexp_inspector_r_connection.h"

void sexp_inspector_debug_initialize();
int sexp_inspector_debug_is_running();
void sexp_inspector_debug_note_allocation(SEXP);
void sexp_inspector_debug_note_gc_unmark(SEXP, unsigned long fake_id);
void sexp_inspector_debug_note_unknown_SEXP(SEXP);
void sexp_inspector_debug_close();

#endif //TINYTRACER_SEXP_INSPECTOR_DEBUG_H
