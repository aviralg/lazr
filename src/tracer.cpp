#include "tracer.h"
#include "TracingState.h"
#include "callbacks.h"

void lazr_tracing_state_destroy(SEXP r_tracing_state) {
    void* pointer = instrumentr_r_externalptr_to_c_pointer(r_tracing_state);
    if (pointer == NULL) {
        instrumentr_log_error("tracing state is null");
    } else {
        TracingState* state = static_cast<TracingState*>(pointer);
        delete state;
        instrumentr_r_externalptr_clear(r_tracing_state);
    }
}

SEXP lazr_tracing_state_create() {
    TracingState* state = new TracingState();
    return instrumentr_c_pointer_to_r_externalptr(
        state, R_NilValue, R_NilValue, lazr_tracing_state_destroy);
}

TracingState* lazr_tracer_get_tracing_state(instrumentr_tracer_t tracer) {
    SEXP r_data = instrumentr_object_get_data(tracer);
    void* c_data = instrumentr_r_externalptr_to_c_pointer(r_data);
    return static_cast<TracingState*>(c_data);
}

SEXP r_lazr_tracer_get_tracing_state(SEXP r_tracer) {
    instrumentr_tracer_t tracer = instrumentr_tracer_unwrap(r_tracer);
    TracingState* state = lazr_tracer_get_tracing_state(tracer);
    return state->to_sexp();
}

SEXP r_lazr_tracer_create() {
    instrumentr_tracer_t tracer = instrumentr_tracer_create();

    instrumentr_callback_t callback =
        instrumentr_callback_create_from_c_function(
            (void*) (closure_call_exit_callback),
            INSTRUMENTR_EVENT_CLOSURE_CALL_EXIT);

    instrumentr_tracer_set_callback(tracer, callback);

    instrumentr_object_release(callback);

    instrumentr_object_set_data(tracer, lazr_tracing_state_create());

    SEXP r_tracer = instrumentr_tracer_wrap(tracer);
    instrumentr_object_release(tracer);
    return r_tracer;
}
