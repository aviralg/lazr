#ifndef STRICTR_TRACER_H
#define STRICTR_TRACER_H

#include "Rincludes.h"
#include "TracingState.h"
#include <instrumentr/instrumentr.h>

extern "C" {
SEXP r_lazr_tracer_create();
SEXP r_lazr_tracer_get_tracing_state(SEXP r_tracer);

TracingState* lazr_tracer_get_tracing_state(instrumentr_tracer_t tracer);
}

#endif /* STRICTR_TRACER_H */
