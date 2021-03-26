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

void builtin_call_entry_callback(instrumentr_tracer_t tracer,
                                 instrumentr_callback_t callback,
                                 instrumentr_state_t state,
                                 instrumentr_application_t application,
                                 instrumentr_builtin_t builtin,
                                 instrumentr_call_t call) {
    TracingState& tracing_state = TracingState::lookup(state);

    ArgumentTable& arg_table = tracing_state.get_argument_table();

    ReflectionTable& ref_table = tracing_state.get_reflection_table();

    std::string name = instrumentr_builtin_get_name(builtin);

    /* NOTE: sys.status calls 3 of these functions so it is not added to the
     * list  */
    if (name != "sys.parent" && name != "sys.call" && name != "sys.frame" &&
        name != "sys.nframe" && name != "sys.calls" && name != "sys.frames" &&
        name != "sys.parents" && name != "sys.function" &&
        name != "parent.frame" && name != "sys.on.exit" &&
        name != "as.environment" && name != "pos.to.env") {
        return;
    }

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    bool transitive = false;
    for (int i = 0; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (!instrumentr_frame_is_promise(frame)) {
            continue;
        }

        instrumentr_promise_t promise = instrumentr_frame_as_promise(frame);

        if (instrumentr_promise_get_type(promise) !=
            INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
            continue;
        }

        int promise_id = instrumentr_promise_get_id(promise);

        const std::vector<Argument*>& args = arg_table.lookup(promise_id);

        for (auto& arg: args) {
            arg->reflection(name);
        }

        ref_table.insert(name, transitive, promise);

        transitive = true;
    }
}

void process_arguments(ArgumentTable& argument_table,
                       instrumentr_call_t call,
                       instrumentr_closure_t closure,
                       Call* call_data,
                       Function* function_data,
                       Environment* environment_data) {
    instrumentr_environment_t call_env = instrumentr_call_get_environment(call);

    instrumentr_value_t formals = instrumentr_closure_get_formals(closure);

    int position = 0;

    while (instrumentr_value_is_pairlist(formals)) {
        instrumentr_pairlist_t pairlist =
            instrumentr_value_as_pairlist(formals);

        instrumentr_value_t tagval = instrumentr_pairlist_get_tag(pairlist);

        instrumentr_symbol_t nameval = instrumentr_value_as_symbol(tagval);

        const char* argument_name = instrumentr_char_get_element(
            instrumentr_symbol_get_element(nameval));

        instrumentr_value_t argval =
            instrumentr_environment_lookup(call_env, nameval);

        argument_table.insert(argval,
                              position,
                              argument_name,
                              call_data,
                              function_data,
                              environment_data);

        ++position;
        formals = instrumentr_pairlist_get_cdr(pairlist);
    }
}

void process_actuals(ArgumentTable& argument_table, instrumentr_call_t call) {
    int call_id = instrumentr_call_get_id(call);

    instrumentr_value_t arguments = instrumentr_call_get_arguments(call);

    int index = 0;

    while (!instrumentr_value_is_null(arguments)) {
        instrumentr_pairlist_t pairlist =
            instrumentr_value_as_pairlist(arguments);

        instrumentr_value_t value = instrumentr_pairlist_get_car(pairlist);

        if (instrumentr_value_is_promise(value)) {
            instrumentr_promise_t promise = instrumentr_value_as_promise(value);

            int promise_id = instrumentr_promise_get_id(promise);

            Argument* argument =
                argument_table.lookup_permissive(promise_id, call_id);

            if (argument != nullptr) {
                argument->set_actual_position(index);
            }
        }

        ++index;
        arguments = instrumentr_pairlist_get_cdr(pairlist);
    }
}

void closure_call_entry_callback(instrumentr_tracer_t tracer,
                                 instrumentr_callback_t callback,
                                 instrumentr_state_t state,
                                 instrumentr_application_t application,
                                 instrumentr_closure_t closure,
                                 instrumentr_call_t call) {
    TracingState& tracing_state = TracingState::lookup(state);

    /* handle environments */

    EnvironmentTable& env_table = tracing_state.get_environment_table();

    Environment* fun_env_data =
        env_table.insert(instrumentr_closure_get_environment(closure));

    Environment* call_env_data =
        env_table.insert(instrumentr_call_get_environment(call));

    /* handle closure */

    FunctionTable& function_table = tracing_state.get_function_table();

    Function* function_data = function_table.insert(closure);

    function_data->call();

    /* handle call */

    CallTable& call_table = tracing_state.get_call_table();

    Call* call_data = call_table.insert(call, function_data);

    /* handle arguments */

    ArgumentTable& argument_table = tracing_state.get_argument_table();

    process_arguments(
        argument_table, call, closure, call_data, function_data, call_env_data);

    process_actuals(argument_table, call);
}

void closure_call_exit_callback(instrumentr_tracer_t tracer,
                                instrumentr_callback_t callback,
                                instrumentr_state_t state,
                                instrumentr_application_t application,
                                instrumentr_closure_t closure,
                                instrumentr_call_t call) {
    TracingState& tracing_state = TracingState::lookup(state);

    ArgumentTable& argument_table = tracing_state.get_argument_table();

    /* handle calls */
    CallTable& call_table = tracing_state.get_call_table();

    int call_id = instrumentr_call_get_id(call);

    bool has_result = instrumentr_call_has_result(call);

    std::string result_type = LAZR_NA_STRING;
    if (has_result) {
        result_type = get_type_as_string(instrumentr_call_get_result(call));
    }

    Call* call_data = call_table.lookup(call_id);

    call_data->exit(result_type);
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

    int promise_id = instrumentr_promise_get_id(promise);

    const std::vector<Argument*>& arguments = argument_table.lookup(promise_id);
    std::vector<instrumentr_call_t> calls =
        instrumentr_promise_get_calls(promise);

    for (Argument* argument: arguments) {
        int call_id = argument->get_call_id();

        Call* call_data = call_table.lookup(call_id);

        /* NOTE: first check escaped then lookup */
        if (call_data->has_exited()) {
            argument->escaped();
        }

        argument->metaprogram();
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

    int promise_id = instrumentr_promise_get_id(promise);
    const std::vector<Argument*>& arguments = argument_table.lookup(promise_id);

    for (Argument* argument: arguments) {
        int call_id = argument->get_call_id();

        Call* call_data = call_table.lookup(call_id);

        /* NOTE: first check escaped then lookup */
        if (call_data->has_exited()) {
            argument->escaped();
        }

        argument->lookup();
    }
}

int compute_companion_position(ArgumentTable& argument_table,
                               int call_id,
                               instrumentr_frame_t frame) {
    instrumentr_promise_t frame_promise = instrumentr_frame_as_promise(frame);
    int frame_promise_id = instrumentr_promise_get_id(frame_promise);

    if (instrumentr_promise_get_type(frame_promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return NA_INTEGER;
    }

    const std::vector<instrumentr_call_t>& calls =
        instrumentr_promise_get_calls(frame_promise);

    for (int i = 0; i < calls.size(); ++i) {
        instrumentr_call_t promise_call = calls[i];
        int promise_call_id = instrumentr_call_get_id(promise_call);

        if (promise_call_id == call_id) {
            Argument* promise_argument =
                argument_table.lookup(frame_promise_id, call_id);
            return promise_argument->get_position();
        }
    }

    return NA_INTEGER;
}

void compute_depth_and_companion(instrumentr_state_t state,
                                 ArgumentTable& argument_table,
                                 Call* call_data,
                                 Argument* argument) {
    int call_id = call_data->get_id();

    int argument_id = argument->get_id();

    int force_depth = 0;
    int companion_position = NA_INTEGER;

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    for (int i = 1; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (companion_position == NA_INTEGER &&
            instrumentr_frame_is_promise(frame)) {
            companion_position =
                compute_companion_position(argument_table, call_id, frame);
        }

        if (instrumentr_frame_is_call(frame)) {
            instrumentr_call_t frame_call = instrumentr_frame_as_call(frame);

            int frame_call_id = instrumentr_call_get_id(frame_call);

            instrumentr_value_t function =
                instrumentr_call_get_function(frame_call);

            if (instrumentr_value_is_closure(function)) {
                ++force_depth;
            }

            if (frame_call_id == call_id) {
                break;
            }
        }
    }

    argument->force(force_depth, companion_position);
}

void promise_force_entry_callback(instrumentr_tracer_t tracer,
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

    int promise_id = instrumentr_promise_get_id(promise);
    const std::vector<Argument*>& arguments = argument_table.lookup(promise_id);

    for (Argument* argument: arguments) {
        int call_id = argument->get_call_id();

        Call* call_data = call_table.lookup(call_id);

        /* NOTE: the order of these statements is important.
         force position changes after adding to call*/
        int force_position = call_data->get_force_position();

        argument->set_force_position(force_position);

        call_data->force_argument(argument->get_position());

        /* NOTE: first check escaped */
        if (call_data->has_exited()) {
            argument->escaped();
        } else {
            compute_depth_and_companion(
                state, argument_table, call_data, argument);
        }
    }
}

void promise_force_exit_callback(instrumentr_tracer_t tracer,
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

    int promise_id = instrumentr_promise_get_id(promise);
    const std::vector<Argument*>& arguments = argument_table.lookup(promise_id);

    for (Argument* argument: arguments) {
        int call_id = argument->get_call_id();

        Call* call_data = call_table.lookup(call_id);

        /* NOTE: first check escaped */
        if (call_data->has_exited()) {
            argument->escaped();
        }

        std::string value_type = LAZR_NA_STRING;
        if (instrumentr_promise_is_forced(promise)) {
            instrumentr_value_t value = instrumentr_promise_get_value(promise);
            value_type = instrumentr_value_type_get_name(
                instrumentr_value_get_type(value));
        }

        argument->set_value_type(value_type);
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

void process_side_effects(instrumentr_state_t state,
                          instrumentr_environment_t environment,
                          const std::string& type,
                          const std::string& varname,
                          ArgumentTable& argument_table,
                          EnvironmentTable& environment_table,
                          SideEffectsTable& se_table) {
    Environment* env = environment_table.insert(environment);

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    bool transitive = false;

    for (int i = 0; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (!instrumentr_frame_is_promise(frame)) {
            continue;
        }

        instrumentr_promise_t promise = instrumentr_frame_as_promise(frame);

        if (instrumentr_promise_get_type(promise) !=
            INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
            continue;
        }

        int t1 = instrumentr_promise_get_birth_time(promise);
        int t2 = instrumentr_environment_get_birth_time(environment);

        // if environment was born before the promise then writing to it is
        // a side effect side effect
        if (t1 > t2) {
            int promise_id = instrumentr_promise_get_id(promise);

            int env_id = instrumentr_environment_get_id(environment);

            se_table.insert(type, varname, transitive, promise, env);

            const std::vector<Argument*>& args =
                argument_table.lookup(promise_id);

            for (auto& arg: args) {
                arg->side_effect(type);
            }
        }

        /* TODO: should it be set iff there is a SE by the topmost promise.
         */
        transitive = true;
    }
}

void variable_assign(instrumentr_tracer_t tracer,
                     instrumentr_callback_t callback,
                     instrumentr_state_t state,
                     instrumentr_application_t application,
                     instrumentr_symbol_t symbol,
                     instrumentr_value_t value,
                     instrumentr_environment_t environment) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_table = tracing_state.get_argument_table();
    SideEffectsTable& se_table = tracing_state.get_side_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_side_effects(
        state, environment, "asn", varname, arg_table, env_table, se_table);
}

void variable_define(instrumentr_tracer_t tracer,
                     instrumentr_callback_t callback,
                     instrumentr_state_t state,
                     instrumentr_application_t application,
                     instrumentr_symbol_t symbol,
                     instrumentr_value_t value,
                     instrumentr_environment_t environment) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_table = tracing_state.get_argument_table();
    SideEffectsTable& se_table = tracing_state.get_side_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_side_effects(
        state, environment, "def", varname, arg_table, env_table, se_table);
}

void variable_remove(instrumentr_tracer_t tracer,
                     instrumentr_callback_t callback,
                     instrumentr_state_t state,
                     instrumentr_application_t application,
                     instrumentr_symbol_t symbol,
                     instrumentr_environment_t environment) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_table = tracing_state.get_argument_table();
    SideEffectsTable& se_table = tracing_state.get_side_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();

    std::string varname = LAZR_NA_STRING;

    process_side_effects(
        state, environment, "rem", varname, arg_table, env_table, se_table);
}

void value_finalize(instrumentr_tracer_t tracer,
                    instrumentr_callback_t callback,
                    instrumentr_state_t state,
                    instrumentr_application_t application,
                    instrumentr_value_t value) {
    TracingState& tracing_state = TracingState::lookup(state);

    if (instrumentr_value_is_closure(value)) {
        instrumentr_closure_t closure = instrumentr_value_as_closure(value);

        FunctionTable& function_table = tracing_state.get_function_table();

        int id = instrumentr_closure_get_id(closure);
        Function* fun = function_table.lookup(id);

        if (fun != NULL) {
            fun->set_name(instrumentr_closure_get_name(closure));
        }
    }

    else if (instrumentr_value_is_environment(value)) {
        instrumentr_environment_t environment =
            instrumentr_value_as_environment(value);

        EnvironmentTable& environment_table =
            tracing_state.get_environment_table();

        int id = instrumentr_environment_get_id(environment);
        Environment* env = environment_table.lookup(id);

        if (env != NULL) {
            env->set_name(instrumentr_environment_get_name(environment));

            instrumentr_environment_type_t type =
                instrumentr_environment_get_type(environment);
            const char* env_type = instrumentr_environment_type_to_string(type);
            env->set_type(env_type);

            const char* env_name =
                instrumentr_environment_get_name(environment);
            env->set_name(env_name);
        }
    }
}
