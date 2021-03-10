#include "tracer.h"
#include "TracingState.h"
#include "callbacks.h"
#include <instrumentr/instrumentr.h>

SEXP r_lazr_tracer_create() {
    instrumentr_tracer_t tracer = instrumentr_tracer_create();

    instrumentr_callback_t callback =
        instrumentr_callback_create_from_c_function(
            (void*) (closure_call_exit_callback),
            INSTRUMENTR_EVENT_CLOSURE_CALL_EXIT);

    instrumentr_tracer_set_callback(tracer, callback);
    instrumentr_object_release(callback);

    callback = instrumentr_callback_create_from_c_function(
        (void*) (tracing_entry_callback), INSTRUMENTR_EVENT_TRACING_ENTRY);

    instrumentr_tracer_set_callback(tracer, callback);
    instrumentr_object_release(callback);

    callback = instrumentr_callback_create_from_c_function(
        (void*) (tracing_exit_callback), INSTRUMENTR_EVENT_TRACING_EXIT);

    instrumentr_tracer_set_callback(tracer, callback);
    instrumentr_object_release(callback);

    SEXP r_tracer = instrumentr_tracer_wrap(tracer);
    instrumentr_object_release(tracer);
    return r_tracer;
}
