#include "TracingState.h"

void tracing_state_destroy(SEXP r_tracing_state) {
    void* pointer = instrumentr_r_externalptr_to_c_pointer(r_tracing_state);
    if (pointer == NULL) {
        instrumentr_log_error("tracing state is null");
    } else {
        TracingState* state = static_cast<TracingState*>(pointer);
        delete state;
        instrumentr_r_externalptr_clear(r_tracing_state);
    }
}

void TracingState::initialize(instrumentr_state_t state) {
    TracingState* tracing_state = new TracingState();

    SEXP r_tracing_state = PROTECT(instrumentr_c_pointer_to_r_externalptr(
        tracing_state, R_NilValue, R_NilValue, tracing_state_destroy));

    instrumentr_state_insert(state, "tracing_state", r_tracing_state, true);
    UNPROTECT(1);
}

void TracingState::finalize(instrumentr_state_t state) {
    TracingState& tracing_state = TracingState::lookup(state);
    SEXP r_calls = PROTECT(tracing_state.get_call_table().to_sexp());
    SEXP r_arguments = PROTECT(tracing_state.get_argument_table().to_sexp());
    SEXP r_functions = PROTECT(tracing_state.get_function_table().to_sexp());
    SEXP r_environments =
        PROTECT(tracing_state.get_environment_table().to_sexp());
    SEXP r_writes = PROTECT(tracing_state.get_writes_table().to_sexp());
    SEXP r_reflection = PROTECT(tracing_state.get_reflection_table().to_sexp());

    instrumentr_state_erase(state, "tracing_state", true);
    instrumentr_state_insert(state, "calls", r_calls, true);
    instrumentr_state_insert(state, "arguments", r_arguments, true);
    instrumentr_state_insert(state, "functions", r_functions, true);
    instrumentr_state_insert(state, "environments", r_environments, true);
    instrumentr_state_insert(state, "writes", r_writes, true);
    instrumentr_state_insert(state, "reflection", r_reflection, true);

    UNPROTECT(6);
}

TracingState& TracingState::lookup(instrumentr_state_t state) {
    SEXP r_tracing_state =
        instrumentr_state_lookup(state, "tracing_state", R_NilValue);
    TracingState* tracing_state = static_cast<TracingState*>(
        instrumentr_r_externalptr_to_c_pointer(r_tracing_state));
    return *tracing_state;
}
