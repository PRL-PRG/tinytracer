//
// Created by kondziu on 23.7.18.
//

#ifndef SEXP_INSPECTOR_H
#define SEXP_INSPECTOR_H

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

void sexp_inspector_init();
void sexp_inspector_allocation(SEXP);
void sexp_inspector_gc(SEXP);
void sexp_inspector_close();

#endif //SEXP_INSPECTOR_H
