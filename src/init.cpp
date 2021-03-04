#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include "tracer.h"
#include <instrumentr/instrumentr.h>

INSTRUMENTR_DEFINE_API()

extern "C" {

static const R_CallMethodDef callMethods[] = {
    {"strictr_tracer_create", (DL_FUNC) &r_strictr_tracer_create, 0},
    {"strictr_tracer_get_tracing_state", (DL_FUNC) &r_strictr_tracer_get_tracing_state, 1},
    {NULL, NULL, 0}
};

void R_init_strictr(DllInfo* dll) {
    R_registerRoutines(dll, NULL, callMethods, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);

    INSTRUMENTR_INITIALIZE_API()
}
}
