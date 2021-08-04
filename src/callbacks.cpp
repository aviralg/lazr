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

void mark_promises(int ref_call_id,
                   const std::string& ref_type,
                   ArgumentTable& arg_tab,
                   ArgumentReflectionTable& ref_tab,
                   instrumentr_call_stack_t call_stack,
                   Backtrace& backtrace) {
    bool transitive = false;

    int source_fun_id = NA_INTEGER;
    int source_call_id = NA_INTEGER;
    int source_arg_id = NA_INTEGER;
    int source_formal_pos = NA_INTEGER;

    for (int i = 1; i < instrumentr_call_stack_get_size(call_stack); ++i) {
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

        /* at this point, we are inside an argument promise which does
         * reflective environment operation. */

        int promise_id = instrumentr_promise_get_id(promise);

        const std::vector<Argument*>& args = arg_tab.lookup(promise_id);

        for (auto& arg: args) {
            arg->reflection(ref_type, transitive);

            int arg_id = arg->get_id();
            int fun_id = arg->get_fun_id();
            int call_id = arg->get_call_id();
            int formal_pos = arg->get_formal_pos();
            ref_tab.insert(ref_call_id,
                           ref_type,
                           transitive,
                           source_fun_id,
                           source_call_id,
                           source_arg_id,
                           source_formal_pos,
                           fun_id,
                           call_id,
                           arg_id,
                           formal_pos,
                           backtrace.to_string());

            if (!transitive) {
                source_fun_id = arg->get_fun_id();
                source_call_id = arg->get_call_id();
                source_arg_id = promise_id;
                source_formal_pos = arg->get_formal_pos();
            }
        }

        transitive = true;
    }
}

void mark_escaped_environment_call(int ref_call_id,
                                   const std::string& ref_type,
                                   ArgumentTable& arg_tab,
                                   CallTable& call_tab,
                                   CallReflectionTable& call_ref_tab,
                                   instrumentr_call_stack_t call_stack,
                                   instrumentr_environment_t environment) {
    int depth = 1;
    int source_fun_id = NA_INTEGER;
    int source_call_id = NA_INTEGER;
    int sink_fun_id = NA_INTEGER;
    int sink_call_id = NA_INTEGER;
    int sink_arg_id = NA_INTEGER;
    int sink_formal_pos = NA_INTEGER;

    for (int i = 0; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (sink_fun_id == NA_INTEGER && instrumentr_frame_is_promise(frame)) {
            instrumentr_promise_t promise = instrumentr_frame_as_promise(frame);

            if (instrumentr_promise_get_type(promise) !=
                INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
                continue;
            }

            int promise_id = instrumentr_promise_get_id(promise);

            /* the last argument refers to the topmost call id */
            Argument* arg = arg_tab.lookup(promise_id).back();

            sink_arg_id = arg->get_id();
            sink_fun_id = arg->get_fun_id();
            sink_call_id = arg->get_call_id();
            sink_formal_pos = arg->get_formal_pos();
        }

        if (instrumentr_frame_is_call(frame)) {
            instrumentr_call_t call = instrumentr_frame_as_call(frame);

            instrumentr_value_t function = instrumentr_call_get_function(call);

            if (!instrumentr_value_is_closure(function)) {
                continue;
            }

            instrumentr_closure_t closure =
                instrumentr_value_as_closure(function);

            if (sink_fun_id == NA_INTEGER) {
                sink_call_id = instrumentr_call_get_id(call);
                sink_fun_id = instrumentr_closure_get_id(closure);
            }

            instrumentr_environment_t call_env =
                instrumentr_call_get_environment(call);

            if (environment == NULL || call_env == environment) {
                source_call_id = instrumentr_call_get_id(call);
                source_fun_id = instrumentr_closure_get_id(closure);

                call_ref_tab.insert(ref_call_id,
                                    ref_type,
                                    source_fun_id,
                                    source_call_id,
                                    sink_fun_id,
                                    sink_call_id,
                                    sink_arg_id,
                                    sink_formal_pos,
                                    depth);

                Call* call_data = call_tab.lookup(source_call_id);
                call_data->esc_env();

                return;
            }

            ++depth;
        }
    }
}

bool has_minus_one_argument(instrumentr_call_t call) {
    instrumentr_value_t arguments = instrumentr_call_get_arguments(call);
    SEXP r_arguments = instrumentr_value_get_sexp(arguments);
    SEXP r_argument = CAR(r_arguments);
    bool valid = false;

    if (TYPEOF(r_argument) == REALSXP) {
        for (int i = 0; i < Rf_length(r_argument); ++i) {
            double pos = REAL_ELT(r_argument, i);
            if (pos == -1) {
                valid = true;
                break;
            }
        }
    }

    else if (TYPEOF(r_argument) == INTSXP) {
        for (int i = 0; i < Rf_length(r_argument); ++i) {
            int pos = INTEGER_ELT(r_argument, i);
            if (pos == -1) {
                valid = true;
                break;
            }
        }
    }

    return valid;
}

void builtin_call_exit_callback(instrumentr_tracer_t tracer,
                                instrumentr_callback_t callback,
                                instrumentr_state_t state,
                                instrumentr_application_t application,
                                instrumentr_builtin_t builtin,
                                instrumentr_call_t call) {
    std::string ref_type = instrumentr_builtin_get_name(builtin);
    int ref_call_id = instrumentr_call_get_id(call);

    /* NOTE: sys.status calls 3 of these functions so it is not added to the
     * list. */
    if (ref_type != "as.environment" && ref_type != "pos.to.env") {
        return;
    }

    if (!has_minus_one_argument(call)) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);

    ArgumentTable& arg_tab = tracing_state.get_argument_table();
    CallTable& call_tab = tracing_state.get_call_table();
    ArgumentReflectionTable& arg_ref_tab = tracing_state.get_arg_ref_tab();
    CallReflectionTable& call_ref_tab = tracing_state.get_call_ref_tab();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    mark_promises(
        ref_call_id, ref_type, arg_tab, arg_ref_tab, call_stack, backtrace);

    if (!instrumentr_call_has_result(call)) {
        return;
    }

    /* NOTE: sys.status calls 3 of these functions so it is not added to the
     * list. */
    if (ref_type == "as.environment" || ref_type == "pos.to.env") {
        instrumentr_value_t result = instrumentr_call_get_result(call);

        if (!instrumentr_value_is_environment(result)) {
            return;
        }

        instrumentr_environment_t environment =
            instrumentr_value_as_environment(result);

        mark_escaped_environment_call(ref_call_id,
                                      ref_type,
                                      arg_tab,
                                      call_tab,
                                      call_ref_tab,
                                      call_stack,
                                      environment);
    }

    /* NOTE: this case does not get affected by argument's evaluation position.
      this means that environments of all functions on the stack escape
    else if (ref_type == "sys.frames") {
        mark_escaped_environment_call(ref_call_id,
                                      ref_type,
                                      arg_tab,
                                      call_tab,
                                      call_ref_tab,
                                      call_stack,
                                      NULL);
    }
    */
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

            if (argument != NULL) {
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

    /* handle backtrace */
    Backtrace& backtrace = tracing_state.get_backtrace();

    backtrace.push(call);
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
        instrumentr_value_t value = instrumentr_call_get_result(call);
        instrumentr_value_type_t val_type = instrumentr_value_get_type(value);
        result_type = instrumentr_value_type_get_name(val_type);
    }

    Call* call_data = call_table.lookup(call_id);

    call_data->exit(result_type);

    /* handle backtrace */
    Backtrace& backtrace = tracing_state.get_backtrace();

    backtrace.pop();
}

std::string get_native_function_name(instrumentr_value_t argument) {
    std::string fun_name = "NA";

    if (instrumentr_value_is_list(argument)) {
        argument = instrumentr_list_get_element(
            instrumentr_value_as_list(argument), 0);
    }

    if (instrumentr_value_is_character(argument)) {
        fun_name =
            instrumentr_char_get_element(instrumentr_character_get_element(
                instrumentr_value_as_character(argument), 0));
    }

    return fun_name;
}

void compute_meta_depth(instrumentr_state_t state,
                        const std::string& meta_type,
                        MetaprogrammingTable& meta_table,
                        Argument* argument,
                        int call_id) {
    int meta_depth = 0;
    int sink_fun_id = NA_INTEGER;
    int sink_call_id = NA_INTEGER;

    std::string intervening_fun(LAZR_NA_STRING);

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    /* topmost frame is substitute call */
    for (int i = 0; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (!instrumentr_frame_is_call(frame)) {
            continue;
        }

        instrumentr_call_t call = instrumentr_frame_as_call(frame);

        instrumentr_value_t function = instrumentr_call_get_function(call);

        if (intervening_fun == LAZR_NA_STRING &&
            instrumentr_value_is_special(function)) {
            intervening_fun = instrumentr_special_get_name(
                instrumentr_value_as_special(function));
        }

        if (intervening_fun == LAZR_NA_STRING &&
            instrumentr_value_is_builtin(function)) {
            std::string name = intervening_fun = instrumentr_builtin_get_name(
                instrumentr_value_as_builtin(function));

            if (name == ".C" || name == ".Fortran" || name == ".External" ||
                name == ".External2" || name == ".Call" ||
                name == ".External.graphics" || name == ".Call.graphics") {
                instrumentr_value_t args = instrumentr_call_get_arguments(call);

                instrumentr_value_t first_arg =
                    instrumentr_pairlist_get_element(
                        instrumentr_value_as_pairlist(args), 0);

                intervening_fun +=
                    "(" + get_native_function_name(first_arg) + ")";
            }
        }

        if (!instrumentr_value_is_closure(function)) {
            continue;
        }

        int current_call_id = instrumentr_call_get_id(call);

        if (sink_fun_id == NA_INTEGER) {
            sink_call_id = current_call_id;
            sink_fun_id = instrumentr_value_get_id(function);
        }

        if (current_call_id == call_id) {
            meta_table.insert(meta_type,
                              intervening_fun,
                              argument->get_fun_id(),
                              argument->get_call_id(),
                              argument->get_id(),
                              argument->get_formal_pos(),
                              sink_fun_id,
                              sink_call_id,
                              meta_depth);

            /* NOTE: expression metaprogramming happens even when substitute is
               called. calling this for substitute will double count
               metaprogramming. */
            if (meta_type == "expression") {
                argument->metaprogram(meta_depth);
            }

            break;
        }

        ++meta_depth;
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
    MetaprogrammingTable& meta_table =
        tracing_state.get_metaprogramming_table();

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

        compute_meta_depth(state, "substitute", meta_table, argument, call_id);
    }
}

void promise_expression_lookup_callback(instrumentr_tracer_t tracer,
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
    MetaprogrammingTable& meta_table =
        tracing_state.get_metaprogramming_table();

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

        compute_meta_depth(state, "expression", meta_table, argument, call_id);
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
            return promise_argument->get_formal_pos();
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

        if (!argument->has_escaped() && instrumentr_frame_is_call(frame)) {
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

    argument->force(argument->has_escaped() ? NA_INTEGER : force_depth,
                    companion_position);
}

void compute_parent_argument(instrumentr_state_t state,
                             ArgumentTable& argument_table,
                             instrumentr_promise_t promise) {
    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    int promise_id = instrumentr_promise_get_id(promise);
    int birth_time = instrumentr_promise_get_birth_time(promise);

    for (int i = 1; i < instrumentr_call_stack_get_size(call_stack); ++i) {
        instrumentr_frame_t frame =
            instrumentr_call_stack_peek_frame(call_stack, i);

        if (!instrumentr_frame_is_promise(frame)) {
            continue;
        }

        instrumentr_promise_t parent_promise =
            instrumentr_frame_as_promise(frame);

        if (instrumentr_promise_get_type(parent_promise) !=
            INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
            continue;
        }

        int force_time =
            instrumentr_promise_get_force_entry_time(parent_promise);

        /* this means the promise was born while this promise was being
           evaluated. this is not interesting. */
        if (force_time < birth_time) {
            continue;
        }

        int parent_promise_id = instrumentr_promise_get_id(parent_promise);

        const std::vector<Argument*>& parent_arguments =
            argument_table.lookup(parent_promise_id);

        /* we take last argument because it refers to the closest call. */
        Argument* parent_argument = parent_arguments.back();

        const std::vector<Argument*>& arguments =
            argument_table.lookup(promise_id);

        for (Argument* argument: arguments) {
            argument->set_parent(parent_argument);
        }

        return;
    }
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

        call_data->force_argument(argument->get_formal_pos());

        /* NOTE: first check escaped */
        if (call_data->has_exited()) {
            argument->escaped();
        }

        compute_depth_and_companion(state, argument_table, call_data, argument);
    }

    compute_parent_argument(state, argument_table, promise);
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
    TracingState& tracing_state = TracingState::lookup(state);
    EnvironmentTable& env_table = tracing_state.get_environment_table();
    FunctionTable& fun_table = tracing_state.get_function_table();

    fun_table.infer_qualified_names(env_table);

    TracingState::finalize(state);
}

void process_reads(instrumentr_state_t state,
                   instrumentr_environment_t environment,
                   const char type,
                   const std::string& varname,
                   ArgumentTable& argument_table,
                   EnvironmentTable& environment_table,
                   EffectsTable& effects_table,
                   Backtrace& backtrace) {
    /* don't process *tmp* as it is an implementation variable */
    if (varname == "*tmp*") {
        return;
    }

    Environment* env = environment_table.insert(environment);

    int t_env_birth = instrumentr_environment_get_birth_time(environment);

    int env_id = instrumentr_environment_get_id(environment);

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    bool transitive = false;

    int source_fun_id = NA_INTEGER;
    int source_call_id = NA_INTEGER;
    int source_arg_id = NA_INTEGER;
    int source_formal_pos = NA_INTEGER;

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

        int t_prom_birth = instrumentr_promise_get_birth_time(promise);
        int t_prom_entry = instrumentr_promise_get_force_entry_time(promise);

        /* if environment is born inside this promise's evaluation, then
           we stop the analysis here because the effect is contained inside
           this promise. */
        if (t_env_birth > t_prom_entry) {
            break;
        }

        bool modified =
            env->is_variable_modified(varname, t_prom_birth, t_prom_entry);

        // This means the environment has been modified after the promise is
        // created but before it is forced and while forcing, this promise is
        // reading from the environment. Since this read can potentially be from
        // the modified location, we should mark it as non local and make the
        // argument lazy.
        if (!transitive && modified) {
            const std::vector<Argument*>& args =
                argument_table.lookup(promise_id);

            for (auto& arg: args) {
                arg->side_effect(type, transitive);
            }

            Argument* arg = args.back();

            /* NOTE: source_ids are NA because effect originates from this
             * promise. */

            effects_table.insert(type,
                                 varname,
                                 transitive,
                                 env_id,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 arg->get_fun_id(),
                                 arg->get_call_id(),
                                 promise_id,
                                 arg->get_formal_pos(),
                                 backtrace.to_string());

            source_fun_id = arg->get_fun_id();
            source_call_id = arg->get_call_id();
            source_arg_id = promise_id;
            source_formal_pos = arg->get_formal_pos();
            transitive = true;
        }

        /* if transitive is set, then the promise directly responsible for the
         * effect has already been found and handled. All other promises on the
         * stack are transitively responsible for forcing it and have to be made
         * lazy. */
        if (transitive && modified) {
            const std::vector<Argument*>& args =
                argument_table.lookup(promise_id);

            for (auto& arg: args) {
                arg->side_effect(type, transitive);
            }

            // Take the most recent argument
            Argument* arg = args.back();

            effects_table.insert(type,
                                 varname,
                                 transitive,
                                 env_id,
                                 source_fun_id,
                                 source_call_id,
                                 source_arg_id,
                                 source_formal_pos,
                                 arg->get_fun_id(),
                                 arg->get_call_id(),
                                 promise_id,
                                 arg->get_formal_pos(),
                                 LAZR_NA_STRING);

            /* loop back to next promise on the stack so it can be made
             * responsible for transitive read. */
            continue;
        }
    }

    env->add_read_time(varname, instrumentr_state_get_time(state));
}

void variable_lookup(instrumentr_tracer_t tracer,
                     instrumentr_callback_t callback,
                     instrumentr_state_t state,
                     instrumentr_application_t application,
                     instrumentr_symbol_t symbol,
                     instrumentr_value_t value,
                     instrumentr_environment_t environment) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_table = tracing_state.get_argument_table();
    EffectsTable& effects_table = tracing_state.get_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_reads(state,
                  environment,
                  'L',
                  varname,
                  arg_table,
                  env_table,
                  effects_table,
                  backtrace);
}

void function_context_lookup(instrumentr_tracer_t tracer,
                             instrumentr_callback_t callback,
                             instrumentr_state_t state,
                             instrumentr_application_t application,
                             instrumentr_symbol_t symbol,
                             instrumentr_value_t value,
                             instrumentr_environment_t environment) {
    if (!instrumentr_value_is_promise(value)) {
        return;
    }

    instrumentr_promise_t promise = instrumentr_value_as_promise(value);

    if (instrumentr_promise_get_type(promise) !=
        INSTRUMENTR_PROMISE_TYPE_ARGUMENT) {
        return;
    }

    TracingState& tracing_state = TracingState::lookup(state);

    ArgumentTable& arg_tab = tracing_state.get_argument_table();

    int promise_id = instrumentr_promise_get_id(promise);

    bool forced = instrumentr_promise_is_forced(promise);

    const std::vector<Argument*>& args = arg_tab.lookup(promise_id);

    for (auto arg: args) {
        arg->set_context_lookup();

        if (!forced) {
            arg->set_context_force();
        }
    }
}

void process_writes(instrumentr_state_t state,
                    instrumentr_environment_t environment,
                    const char type,
                    const std::string& varname,
                    ArgumentTable& argument_table,
                    EnvironmentTable& environment_table,
                    EffectsTable& effects_table,
                    Backtrace& backtrace) {
    /* don't process *tmp* as it is an implementation variable */
    if (varname == "*tmp*") {
        return;
    }

    Environment* env = environment_table.insert(environment);

    int t_env_birth = instrumentr_environment_get_birth_time(environment);
    int env_id = instrumentr_environment_get_id(environment);

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    bool transitive = false;

    int source_fun_id = NA_INTEGER;
    int source_call_id = NA_INTEGER;
    int source_arg_id = NA_INTEGER;
    int source_formal_pos = NA_INTEGER;

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

        int t_prom_birth = instrumentr_promise_get_birth_time(promise);
        int t_prom_entry = instrumentr_promise_get_force_entry_time(promise);

        /* if environment is born inside the the currently forced promise, then
         * effect is contained inside the promise and we can exit */
        if (t_env_birth > t_prom_entry) {
            break;
        }

        // if promise is forced after the environment it is writing to is
        // born then the write is a non-local side effect

        bool touched =
            env->is_variable_touched(varname, t_prom_birth, t_prom_entry);

        if (!transitive && touched) {
            const std::vector<Argument*>& args =
                argument_table.lookup(promise_id);

            for (auto& arg: args) {
                arg->side_effect(type, transitive);
            }

            Argument* arg = args.back();

            /* NOTE: source_ids are NA because effect originates from this
             * promise. */

            effects_table.insert(type,
                                 varname,
                                 transitive,
                                 env_id,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 NA_INTEGER,
                                 arg->get_fun_id(),
                                 arg->get_call_id(),
                                 promise_id,
                                 arg->get_formal_pos(),
                                 backtrace.to_string());

            source_fun_id = arg->get_fun_id();
            source_call_id = arg->get_call_id();
            source_arg_id = promise_id;
            source_formal_pos = arg->get_formal_pos();
            transitive = true;
        }

        /* if transitive is set, then the promise directly responsible for
         * the effect has already been found and handled. All other promises
         * on the stack satisfying this condition are transitively
         * responsible for forcing it and have to be made lazy. */
        if (transitive && touched) {
            const std::vector<Argument*>& args =
                argument_table.lookup(promise_id);

            for (auto& arg: args) {
                arg->side_effect(type, transitive);
            }

            // Take the most recent argument
            Argument* arg = args.back();

            effects_table.insert(type,
                                 varname,
                                 transitive,
                                 env_id,
                                 source_fun_id,
                                 source_call_id,
                                 source_arg_id,
                                 source_formal_pos,
                                 arg->get_fun_id(),
                                 arg->get_call_id(),
                                 promise_id,
                                 arg->get_formal_pos(),
                                 LAZR_NA_STRING);

            /* loop back to next promise on the stack so it can be made
             * responsible for transitive write. */
            continue;
        }
    }

    env->add_write_time(varname, instrumentr_state_get_time(state));
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
    EffectsTable& effects_table = tracing_state.get_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_writes(state,
                   environment,
                   'A',
                   varname,
                   arg_table,
                   env_table,
                   effects_table,
                   backtrace);
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
    EffectsTable& effects_table = tracing_state.get_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_writes(state,
                   environment,
                   'D',
                   varname,
                   arg_table,
                   env_table,
                   effects_table,
                   backtrace);
}

void variable_remove(instrumentr_tracer_t tracer,
                     instrumentr_callback_t callback,
                     instrumentr_state_t state,
                     instrumentr_application_t application,
                     instrumentr_symbol_t symbol,
                     instrumentr_environment_t environment) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_table = tracing_state.get_argument_table();
    EffectsTable& effects_table = tracing_state.get_effects_table();
    EnvironmentTable& env_table = tracing_state.get_environment_table();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_char_t charval = instrumentr_symbol_get_element(symbol);
    std::string varname = instrumentr_char_get_element(charval);

    process_writes(state,
                   environment,
                   'R',
                   varname,
                   arg_table,
                   env_table,
                   effects_table,
                   backtrace);
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

            if (type == INSTRUMENTR_ENVIRONMENT_TYPE_CALL) {
                instrumentr_call_t call =
                    instrumentr_environment_get_call(environment);
                int call_id = instrumentr_call_get_id(call);

                env->set_call_id(call_id);
            }

            const char* env_name =
                instrumentr_environment_get_name(environment);
            env->set_name(env_name);

            env->clear_variable_times();
        }
    }
}

void trace_error(instrumentr_tracer_t tracer,
                 instrumentr_callback_t callback,
                 instrumentr_state_t state,
                 instrumentr_application_t application,
                 instrumentr_value_t call_expr) {
    TracingState& tracing_state = TracingState::lookup(state);
    ArgumentTable& arg_tab = tracing_state.get_argument_table();
    EffectsTable& effects_tab = tracing_state.get_effects_table();
    Backtrace& backtrace = tracing_state.get_backtrace();

    instrumentr_call_stack_t call_stack =
        instrumentr_state_get_call_stack(state);

    bool transitive = false;
    int source_fun_id = NA_INTEGER;
    int source_call_id = NA_INTEGER;
    int source_arg_id = NA_INTEGER;
    int source_formal_pos = NA_INTEGER;

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

        const std::vector<Argument*>& args = arg_tab.lookup(promise_id);

        Argument* arg = args.back();

        arg->side_effect('E', transitive);

        int arg_id = arg->get_id();
        int fun_id = arg->get_fun_id();
        int call_id = arg->get_call_id();
        int formal_pos = arg->get_formal_pos();

        effects_tab.insert('E',
                           LAZR_NA_STRING,
                           transitive,
                           NA_INTEGER,
                           source_fun_id,
                           source_call_id,
                           source_arg_id,
                           source_formal_pos,
                           fun_id,
                           call_id,
                           arg_id,
                           formal_pos,
                           backtrace.to_string());

        if (!transitive) {
            source_fun_id = arg->get_fun_id();
            source_call_id = arg->get_call_id();
            source_arg_id = promise_id;
            source_formal_pos = arg->get_formal_pos();
        }
    }
}
