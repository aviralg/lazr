#include "tracer.h"
#include "callbacks.h"
#include "TracingState.h"
#include "utilities.h"
#include <instrumentr/instrumentr.h>
#include <vector>

std::string get_sexp_type(SEXP r_value) {
    if (r_value == R_UnboundValue) {
        return LAZR_NA_STRING;
    } else {
        return type2char(TYPEOF(r_value));
    }
}

bool promise_check_escaped(instrumentr_call_t call,
                           Call& call_data,
                           Argument& argument_data) {
    if (instrumentr_model_is_dead(call)) {
        argument_data.escaped();
        return true;
    }

    return false;
}

void process_parameter(ArgumentTable& argument_table,
                       const std::string& package_name,
                       const std::string& function_name,
                       int call_id,
                       instrumentr_parameter_t parameter) {
    int parameter_id = instrumentr_model_get_id(parameter);
    int parameter_position = instrumentr_parameter_get_position(parameter);
    const std::string parameter_name(instrumentr_parameter_get_name(parameter));
    int argument_count = instrumentr_parameter_get_argument_count(parameter);
    int vararg = instrumentr_parameter_is_vararg(parameter);
    int missing = instrumentr_parameter_is_missing(parameter);

    std::string argument_type = LAZR_NA_STRING;
    std::string expression_type = LAZR_NA_STRING;
    std::string transitive_type = LAZR_NA_STRING;
    std::string value_type = LAZR_NA_STRING;
    int preforced = 0;

    /* if argument is missing */
    if (missing) {
        expression_type = LAZR_NA_STRING;
        transitive_type = LAZR_NA_STRING;
    }
    /* if not missing and is a vararg */
    else if (vararg) {
        for (int index = 0; index < argument_count; ++index) {
            instrumentr_argument_t argument =
                instrumentr_parameter_get_argument_by_position(parameter,
                                                               index);

            if (instrumentr_argument_is_value(argument)) {
                ++preforced;
            }
            /* if not a value, it has to be a promise */
            else {
                instrumentr_promise_t promise =
                    instrumentr_argument_as_promise(argument);
                if (!instrumentr_promise_is_forced(promise)) {
                    ++preforced;
                }
            }
        }
    }
    /* parameter is not vararg and not missing, so it has a single argument */
    else {
        instrumentr_argument_t argument =
            instrumentr_parameter_get_argument_by_position(parameter, 0);

        if (instrumentr_argument_is_value(argument)) {
            instrumentr_value_t value = instrumentr_argument_as_value(argument);
            argument_type = get_sexp_type(instrumentr_value_get_sexp(value));
            ++preforced;
        }
        /* if not a value, it has to be a promise */
        else {
            argument_type = "promise";
            instrumentr_promise_t promise =
                instrumentr_argument_as_promise(argument);
            if (instrumentr_promise_is_forced(promise)) {
                ++preforced;
                value_type =
                    get_sexp_type(instrumentr_promise_get_value(promise));
            }
            expression_type =
                get_sexp_type(instrumentr_promise_get_expression(promise));
        }
    }

    Argument* argument_data = new Argument(parameter_id,
                                           call_id,
                                           package_name,
                                           function_name,
                                           parameter_position,
                                           parameter_name,
                                           argument_count,
                                           vararg,
                                           missing,
                                           argument_type,
                                           expression_type,
                                           transitive_type,
                                           value_type,
                                           preforced);
    argument_table.insert(argument_data);
}

void process_parameters(Call& call_data,
                        ArgumentTable& argument_table,
                        instrumentr_call_t call) {
    int call_id = instrumentr_model_get_id(call);
    int parameter_count = instrumentr_call_get_parameter_count(call);

    for (int index = 0; index < parameter_count; ++index) {
        instrumentr_parameter_t parameter =
            instrumentr_call_get_parameter_by_position(call, index);

        process_parameter(argument_table,
                          call_data.get_package_name(),
                          call_data.get_function_name(),
                          call_id,
                          parameter);
    }
}

void closure_call_entry_callback(instrumentr_tracer_t tracer,
                                 instrumentr_callback_t callback,
                                 instrumentr_state_t state,
                                 instrumentr_application_t application,
                                 instrumentr_package_t package,
                                 instrumentr_function_t function,
                                 instrumentr_call_t call) {
    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    const char* name = instrumentr_package_get_name(package);
    const std::string package_name(name == NULL ? LAZR_NA_STRING : name);
    name = instrumentr_function_get_name(function);
    const std::string function_name(name == NULL ? LAZR_NA_STRING : name);

    int call_id = instrumentr_model_get_id(call);

    Call* call_data = new Call(call_id, package_name, function_name);

    call_table.insert(call_data);

    process_parameters(*call_data, argument_table, call);
}

void closure_call_exit_callback(instrumentr_tracer_t tracer,
                                instrumentr_callback_t callback,
                                instrumentr_state_t state,
                                instrumentr_application_t application,
                                instrumentr_package_t package,
                                instrumentr_function_t function,
                                instrumentr_call_t call) {
    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    int call_id = instrumentr_model_get_id(call);

    bool has_result = instrumentr_call_has_result(call);
    std::string result_type = LAZR_NA_STRING;
    if (has_result) {
        result_type = get_type_as_string(instrumentr_call_get_result(call));
    }

    Call& call_data = call_table.lookup(call_id);

    call_data.set_result(result_type);
}

int compute_companion_position(int call_id, instrumentr_frame_t frame) {
    instrumentr_promise_t frame_promise = instrumentr_frame_as_promise(frame);

    // TODO: remove this after fixing bug
    if (instrumentr_model_is_dead(frame_promise)) {
        return NA_INTEGER;
    }

    if (instrumentr_promise_get_type(frame_promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return NA_INTEGER;
    }

    const std::vector<instrumentr_promise_call_info_t>& call_info_seq =
        instrumentr_promise_get_call_info(frame_promise);

    for (int i = 0; i < call_info_seq.size(); ++i) {
        instrumentr_promise_call_info_t call_info = call_info_seq[i];

        instrumentr_call_t promise_call = call_info.call;
        int promise_call_id = instrumentr_model_get_id(promise_call);

        instrumentr_parameter_t promise_parameter = call_info.parameter;
        int promise_parameter_position =
            instrumentr_parameter_get_position(promise_parameter);

        if (promise_call_id == call_id) {
            return promise_parameter_position;
        }
    }

    return NA_INTEGER;
}

void handle_promise_force_entry_callback(TracingState& tracing_state,
                                         CallTable& call_table,
                                         ArgumentTable& argument_table,
                                         instrumentr_state_t state,
                                         instrumentr_promise_t promise,
                                         instrumentr_call_t call,
                                         instrumentr_parameter_t parameter,
                                         instrumentr_argument_t argument) {
    int call_id = instrumentr_model_get_id(call);
    Call& call_data = call_table.lookup(call_id);

    int parameter_id = instrumentr_model_get_id(parameter);
    Argument& argument_data = argument_table.lookup(parameter_id);

    bool escaped = promise_check_escaped(call, call_data, argument_data);

    /* TODO: why should promise not be alive at this point?  */
    bool has_value = instrumentr_promise_is_forced(promise);
    std::string value_type = LAZR_NA_STRING;
    if (has_value) {
        value_type = get_type_as_string(instrumentr_promise_get_value(promise));
    }

    int force_depth = NA_INTEGER;
    int companion_position = NA_INTEGER;

    int closure_count = 0;
    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    for (int i = 1; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (companion_position == NA_INTEGER &&
            instrumentr_frame_is_promise(frame)) {
            companion_position = compute_companion_position(call_id, frame);
        }

        if (instrumentr_frame_is_call(frame)) {
            instrumentr_call_t frame_call = instrumentr_frame_as_call(frame);

            int frame_call_id = instrumentr_model_get_id(frame_call);

            instrumentr_function_t function =
                instrumentr_call_get_function(call);

            if (instrumentr_function_get_type(function) ==
                INSTRUMENTR_FUNCTION_CLOSURE) {
                ++closure_count;
            }

            if (frame_call_id == call_id) {
                force_depth = closure_count;
                break;
            }
        }
    }

    argument_data.force(force_depth, companion_position);

    call_data.force_argument(instrumentr_parameter_get_position(parameter));
}

void promise_force_entry_callback(instrumentr_tracer_t tracer,
                                  instrumentr_callback_t callback,
                                  instrumentr_state_t state,
                                  instrumentr_application_t application,
                                  instrumentr_promise_t promise) {
    //// TODO: remove this check after fixing the bug.
    // if (instrumentr_model_is_dead(promise)) {
    //    return;
    //}

    if (instrumentr_promise_get_type(promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    const std::vector<instrumentr_promise_call_info_t>& call_info_seq =
        instrumentr_promise_get_call_info(promise);

    for (int i = 0; i < call_info_seq.size(); ++i) {
        instrumentr_promise_call_info_t call_info = call_info_seq[i];
        handle_promise_force_entry_callback(tracing_state,
                                            call_table,
                                            argument_table,
                                            state,
                                            promise,
                                            call_info.call,
                                            call_info.parameter,
                                            call_info.argument);
    }
}

void promise_force_exit_callback(instrumentr_tracer_t tracer,
                                 instrumentr_callback_t callback,
                                 instrumentr_state_t state,
                                 instrumentr_application_t application,
                                 instrumentr_promise_t promise) {
    // TODO: remove this check after fixing the bug.
    if (instrumentr_model_is_dead(promise)) {
        return;
    }

    if (instrumentr_promise_get_type(promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    const std::vector<instrumentr_promise_call_info_t>& call_info_seq =
        instrumentr_promise_get_call_info(promise);

    for (int i = 0; i < call_info_seq.size(); ++i) {
        instrumentr_promise_call_info_t call_info = call_info_seq[i];
        instrumentr_call_t call = call_info.call;
        instrumentr_parameter_t parameter = call_info.parameter;
        instrumentr_argument_t argument = call_info.argument;

        int call_id = instrumentr_model_get_id(call);
        Call& call_data = call_table.lookup(call_id);

        int parameter_id = instrumentr_model_get_id(parameter);
        Argument& argument_data = argument_table.lookup(parameter_id);

        bool escaped = promise_check_escaped(call, call_data, argument_data);

        bool has_value = instrumentr_promise_is_forced(promise);
        std::string value_type = LAZR_NA_STRING;
        if (has_value) {
            value_type =
                get_type_as_string(instrumentr_promise_get_value(promise));
        }

        argument_data.set_value_type(value_type);
    }
}

void promise_value_lookup_callback(instrumentr_tracer_t tracer,
                                   instrumentr_callback_t callback,
                                   instrumentr_state_t state,
                                   instrumentr_application_t application,
                                   instrumentr_promise_t promise) {
    if (instrumentr_promise_get_type(promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    const std::vector<instrumentr_promise_call_info_t>& call_info_seq =
        instrumentr_promise_get_call_info(promise);

    for (int i = 0; i < call_info_seq.size(); ++i) {
        instrumentr_promise_call_info_t call_info = call_info_seq[i];

        instrumentr_call_t call = call_info.call;
        instrumentr_parameter_t parameter = call_info.parameter;
        instrumentr_argument_t argument = call_info.argument;

        int call_id = instrumentr_model_get_id(call);
        Call& call_data = call_table.lookup(call_id);

        int parameter_id = instrumentr_model_get_id(parameter);
        Argument& argument_data = argument_table.lookup(parameter_id);

        promise_check_escaped(call, call_data, argument_data);

        argument_data.lookup();
    }
}

void promise_substitute_callback(instrumentr_tracer_t tracer,
                                 instrumentr_callback_t callback,
                                 instrumentr_state_t state,
                                 instrumentr_application_t application,
                                 instrumentr_promise_t promise) {
    if (instrumentr_promise_get_type(promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);
    CallTable& call_table = tracing_state.get_call_table();
    ArgumentTable& argument_table = tracing_state.get_argument_table();

    const std::vector<instrumentr_promise_call_info_t>& call_info_seq =
        instrumentr_promise_get_call_info(promise);

    for (int i = 0; i < call_info_seq.size(); ++i) {
        instrumentr_promise_call_info_t call_info = call_info_seq[i];

        instrumentr_call_t call = call_info.call;
        instrumentr_parameter_t parameter = call_info.parameter;
        instrumentr_argument_t argument = call_info.argument;

        int call_id = instrumentr_model_get_id(call);
        Call& call_data = call_table.lookup(call_id);

        int parameter_id = instrumentr_model_get_id(parameter);
        Argument& argument_data = argument_table.lookup(parameter_id);

        promise_check_escaped(call, call_data, argument_data);

        argument_data.metaprogram();
    }
}

void tracing_entry_callback(instrumentr_tracer_t tracer,
                            instrumentr_callback_t callback,
                            instrumentr_state_t state) {
    TracingState::initialize(state);
}

void tracing_exit_callback(instrumentr_tracer_t tracer,
                           instrumentr_callback_t callback,
                           instrumentr_state_t state) {
    TracingState::finalize(state);
}
