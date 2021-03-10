#ifndef LAZR_CALLBACKS_H
#define LAZR_CALLBACKS_H

#include <instrumentr/instrumentr.h>

void tracing_entry_callback(instrumentr_tracer_t tracer,
                            instrumentr_callback_t callback,
                            instrumentr_state_t state);

void tracing_exit_callback(instrumentr_tracer_t tracer,
                           instrumentr_callback_t callback,
                           instrumentr_state_t state);

void closure_call_exit_callback(instrumentr_tracer_t tracer,
                                instrumentr_callback_t callback,
                                instrumentr_state_t state,
                                instrumentr_application_t application,
                                instrumentr_package_t package,
                                instrumentr_function_t function,
                                instrumentr_call_t call);

#endif /* LAZR_CALLBACKS_H  */
