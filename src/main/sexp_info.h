//
// Created by kondziu on 23.7.18.
//

#ifndef SEXP_INFO_H
#define SEXP_INFO_H

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

typedef struct {
    // metadata
    //SEXP          address;
    unsigned long fake_id;
    unsigned int  initial_gc_cycle;
//    unsigned long final_inspection_clock;
//    // header
//    unsigned int  sexp_type;
//    unsigned int  scalar;
//    unsigned int  obj;
//    unsigned int  alt;
//    unsigned int  gp;
//    unsigned int  mark;
//    unsigned int  debug;
//    unsigned int  trace;
//    unsigned int  spare;
//    unsigned int  gcgen;
//    unsigned int  gccls;
//    unsigned int  named;
} *sexp_info;

#endif //SEXP_INFO_H
