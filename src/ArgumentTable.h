#ifndef LAZR_ARGUMENT_TABLE_H
#define LAZR_ARGUMENT_TABLE_H

#include "Argument.h"
#include "Environment.h"
#include "Function.h"
#include "Call.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "utilities.h"
#include <instrumentr/instrumentr.h>

class ArgumentTable {
  public:
    ArgumentTable(): size_(0) {
    }

    ~ArgumentTable() {
        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            for (auto& argument: iter->second) {
                delete argument;
            }
        }
        table_.clear();
    }

    void insert(instrumentr_value_t argument,
                int argument_position,
                const std::string& argument_name,
                Call* call_data,
                Function* function_data,
                Environment* environment_data) {
        if (instrumentr_value_is_dot(argument)) {
            insert_dot_(instrumentr_value_as_dot(argument),
                        argument_position,
                        argument_name,
                        call_data,
                        function_data,
                        environment_data);
        }

        else if (instrumentr_value_is_missing(argument)) {
            insert_missing_(instrumentr_value_as_missing(argument),
                            argument_position,
                            argument_name,
                            call_data,
                            function_data,
                            environment_data);
        }

        else if (instrumentr_value_is_promise(argument)) {
            insert_promise_(instrumentr_value_as_promise(argument),
                            argument_position,
                            argument_name,
                            call_data,
                            function_data,
                            environment_data);
        }

        else {
            insert_value_(argument,
                          argument_position,
                          argument_name,
                          call_data,
                          function_data,
                          environment_data);
        }
    }

    const std::vector<Argument*>& lookup(int argument_id) {
        auto result = table_.find(argument_id);

        if (result == table_.end()) {
            Rf_error("cannot find argument with id %d", argument_id);
        }
        return result->second;
    }

    Argument* lookup(int argument_id, int call_id) {
        const std::vector<Argument*>& arguments = lookup(argument_id);

        for (auto argument: arguments) {
            if (argument->get_call_id() == call_id) {
                return argument;
            }
        }

        Rf_error("cannot find argument with id %d and call id %d",
                 argument_id,
                 call_id);

        return NULL;
    }

    Argument* lookup_permissive(int argument_id, int call_id) {
        auto result = table_.find(argument_id);

        if (result == table_.end()) {
            return NULL;
        }

        const std::vector<Argument*>& arguments = result->second;

        for (auto argument: arguments) {
            if (argument->get_call_id() == call_id) {
                return argument;
            }
        }
        return NULL;
    }

    SEXP to_sexp() {
        SEXP r_argument_id = PROTECT(allocVector(INTSXP, size_));
        SEXP r_call_id = PROTECT(allocVector(INTSXP, size_));
        SEXP r_function_id = PROTECT(allocVector(INTSXP, size_));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size_));
        SEXP r_environment_id = PROTECT(allocVector(INTSXP, size_));
        SEXP r_environment_name = PROTECT(allocVector(STRSXP, size_));
        SEXP r_argument_position = PROTECT(allocVector(INTSXP, size_));
        SEXP r_force_position = PROTECT(allocVector(INTSXP, size_));
        SEXP r_actual_position = PROTECT(allocVector(INTSXP, size_));
        SEXP r_argument_name = PROTECT(allocVector(STRSXP, size_));
        SEXP r_argument_count = PROTECT(allocVector(INTSXP, size_));
        SEXP r_vararg = PROTECT(allocVector(LGLSXP, size_));
        SEXP r_missing = PROTECT(allocVector(LGLSXP, size_));
        SEXP r_argument_type = PROTECT(allocVector(STRSXP, size_));
        SEXP r_expression_type = PROTECT(allocVector(STRSXP, size_));
        SEXP r_transitive_type = PROTECT(allocVector(STRSXP, size_));
        SEXP r_value_type = PROTECT(allocVector(STRSXP, size_));
        SEXP r_preforced = PROTECT(allocVector(INTSXP, size_));
        SEXP r_cap_force = PROTECT(allocVector(INTSXP, size_));
        SEXP r_cap_meta = PROTECT(allocVector(INTSXP, size_));
        SEXP r_cap_lookup = PROTECT(allocVector(INTSXP, size_));
        SEXP r_escaped = PROTECT(allocVector(LGLSXP, size_));
        SEXP r_esc_force = PROTECT(allocVector(INTSXP, size_));
        SEXP r_esc_meta = PROTECT(allocVector(INTSXP, size_));
        SEXP r_esc_lookup = PROTECT(allocVector(INTSXP, size_));
        SEXP r_force_depth = PROTECT(allocVector(INTSXP, size_));
        SEXP r_force_source = PROTECT(allocVector(STRSXP, size_));
        SEXP r_companion_position = PROTECT(allocVector(INTSXP, size_));
        SEXP r_event_sequence = PROTECT(allocVector(STRSXP, size_));
        SEXP r_effect_sequence = PROTECT(allocVector(STRSXP, size_));
        SEXP r_reflection_sequence = PROTECT(allocVector(STRSXP, size_));

        int index = 0;

        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            auto& arguments = iter->second;

            for (Argument* argument: arguments) {
                argument->to_sexp(index,
                                  r_argument_id,
                                  r_call_id,
                                  r_function_id,
                                  r_function_name,
                                  r_environment_id,
                                  r_environment_name,
                                  r_argument_position,
                                  r_force_position,
                                  r_actual_position,
                                  r_argument_name,
                                  r_argument_count,
                                  r_vararg,
                                  r_missing,
                                  r_argument_type,
                                  r_expression_type,
                                  r_transitive_type,
                                  r_value_type,
                                  r_preforced,
                                  r_cap_force,
                                  r_cap_meta,
                                  r_cap_lookup,
                                  r_escaped,
                                  r_esc_force,
                                  r_esc_meta,
                                  r_esc_lookup,
                                  r_force_depth,
                                  r_force_source,
                                  r_companion_position,
                                  r_event_sequence,
                                  r_effect_sequence,
                                  r_reflection_sequence);
                ++index;
            }
        }

        std::vector<SEXP> columns({r_argument_id,
                                   r_call_id,
                                   r_function_id,
                                   r_function_name,
                                   r_environment_id,
                                   r_environment_name,
                                   r_argument_position,
                                   r_force_position,
                                   r_actual_position,
                                   r_argument_name,
                                   r_argument_count,
                                   r_vararg,
                                   r_missing,
                                   r_argument_type,
                                   r_expression_type,
                                   r_transitive_type,
                                   r_value_type,
                                   r_preforced,
                                   r_cap_force,
                                   r_cap_meta,
                                   r_cap_lookup,
                                   r_escaped,
                                   r_esc_force,
                                   r_esc_meta,
                                   r_esc_lookup,
                                   r_force_depth,
                                   r_force_source,
                                   r_companion_position,
                                   r_event_sequence,
                                   r_effect_sequence,
                                   r_reflection_sequence});

        std::vector<std::string> names({"argument_id",
                                        "call_id",
                                        "function_id",
                                        "function_name",
                                        "environment_id",
                                        "environment_name",
                                        "argument_position",
                                        "force_position",
                                        "actual_position",
                                        "argument_name",
                                        "argument_count",
                                        "vararg",
                                        "missing",
                                        "argument_type",
                                        "expression_type",
                                        "transitive_type",
                                        "value_type",
                                        "preforced",
                                        "cap_force",
                                        "cap_meta",
                                        "cap_lookup",
                                        "escaped",
                                        "esc_force",
                                        "esc_meta",
                                        "esc_lookup",
                                        "force_depth",
                                        "force_source",
                                        "companion_position",
                                        "event_sequence",
                                        "effect_sequence",
                                        "reflection_sequence"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(31);

        return df;
    }

  private:
    std::unordered_map<int, std::vector<Argument*>> table_;
    int size_;

    void insert_dot_(instrumentr_dot_t dot,
                     int argument_position,
                     const std::string& argument_name,
                     Call* call_data,
                     Function* function_data,
                     Environment* environment_data) {
        instrumentr_value_t ptr = instrumentr_dot_as_value(dot);

        int preforced = 0;
        int argument_count = 0;

        while (1) {
            if (instrumentr_value_is_null(ptr)) {
                break;
            }

            else if (instrumentr_value_is_dot(ptr)) {
                instrumentr_dot_t dot_ptr = instrumentr_value_as_dot(ptr);

                instrumentr_value_t value = instrumentr_dot_get_car(dot_ptr);

                ptr = instrumentr_dot_get_cdr(dot_ptr);

                ++argument_count;

                if (instrumentr_value_is_promise(value)) {
                    preforced += instrumentr_promise_is_forced(
                        instrumentr_value_as_promise(value));
                }

                else {
                    ++preforced;
                }
            }

            else if (instrumentr_value_is_pairlist(ptr)) {
                instrumentr_pairlist_t pairlist_ptr =
                    instrumentr_value_as_pairlist(ptr);

                instrumentr_value_t value =
                    instrumentr_pairlist_get_car(pairlist_ptr);

                ptr = instrumentr_pairlist_get_cdr(pairlist_ptr);

                ++argument_count;

                if (instrumentr_value_is_promise(value)) {
                    preforced += instrumentr_promise_is_forced(
                        instrumentr_value_as_promise(value));
                }

                else {
                    ++preforced;
                }
            }
        }

        int argument_id = instrumentr_dot_get_id(dot);
        int call_id = call_data->get_id();
        int function_id = function_data->get_id();
        std::string function_name = function_data->get_name();
        int environment_id = environment_data->get_id();
        std::string environment_name = environment_data->get_name();
        int vararg = 1;
        int missing = 0;
        std::string argument_type = "dot";
        std::string expression_type = LAZR_NA_STRING;
        std::string transitive_type = LAZR_NA_STRING;
        std::string value_type = LAZR_NA_STRING;

        Argument* argument_data = new Argument(argument_id,
                                               call_id,
                                               function_id,
                                               function_name,
                                               environment_id,
                                               environment_name,
                                               argument_position,
                                               argument_name,
                                               argument_count,
                                               vararg,
                                               missing,
                                               argument_type,
                                               expression_type,
                                               transitive_type,
                                               value_type,
                                               preforced);

        insert_(argument_data);
    }

    void insert_missing_(instrumentr_missing_t missing_val,
                         int argument_position,
                         const std::string& argument_name,
                         Call* call_data,
                         Function* function_data,
                         Environment* environment_data) {
        int argument_id = instrumentr_missing_get_id(missing_val);
        int call_id = call_data->get_id();
        int function_id = function_data->get_id();
        std::string function_name = function_data->get_name();
        int environment_id = environment_data->get_id();
        std::string environment_name = environment_data->get_name();
        int argument_count = 0;
        int vararg = 0;
        int missing = 1;
        std::string argument_type = "missing";
        std::string expression_type = LAZR_NA_STRING;
        std::string transitive_type = LAZR_NA_STRING;
        std::string value_type = LAZR_NA_STRING;
        int preforced = NA_INTEGER;

        Argument* argument_data = new Argument(argument_id,
                                               call_id,
                                               function_id,
                                               function_name,
                                               environment_id,
                                               environment_name,
                                               argument_position,
                                               argument_name,
                                               argument_count,
                                               vararg,
                                               missing,
                                               argument_type,
                                               expression_type,
                                               transitive_type,
                                               value_type,
                                               preforced);

        insert_(argument_data);
    }

    void insert_promise_(instrumentr_promise_t promise,
                         int argument_position,
                         const std::string& argument_name,
                         Call* call_data,
                         Function* function_data,
                         Environment* environment_data) {
        instrumentr_value_type_t prom_expr_type = instrumentr_value_get_type(
            instrumentr_promise_get_expression(promise));

        instrumentr_value_type_t prom_val_type =
            instrumentr_value_get_type(instrumentr_promise_get_value(promise));

        int argument_id = instrumentr_promise_get_id(promise);
        int call_id = call_data->get_id();
        int function_id = function_data->get_id();
        std::string function_name = function_data->get_name();
        int environment_id = environment_data->get_id();
        std::string environment_name = environment_data->get_name();
        int argument_count = 1;
        int vararg = 0;
        int missing = 0;
        std::string argument_type = "promise";
        std::string expression_type =
            instrumentr_value_type_get_name(prom_expr_type);
        std::string transitive_type = LAZR_NA_STRING;
        std::string value_type = instrumentr_value_type_get_name(prom_val_type);
        int preforced = instrumentr_promise_is_forced(promise);

        Argument* argument_data = new Argument(argument_id,
                                               call_id,
                                               function_id,
                                               function_name,
                                               environment_id,
                                               environment_name,
                                               argument_position,
                                               argument_name,
                                               argument_count,
                                               vararg,
                                               missing,
                                               argument_type,
                                               expression_type,
                                               transitive_type,
                                               value_type,
                                               preforced);

        insert_(argument_data);
    }

    void insert_value_(instrumentr_value_t value,
                       int argument_position,
                       const std::string& argument_name,
                       Call* call_data,
                       Function* function_data,
                       Environment* environment_data) {
        instrumentr_value_type_t val_type = instrumentr_value_get_type(value);

        int argument_id = instrumentr_value_get_id(value);
        int call_id = call_data->get_id();
        int function_id = function_data->get_id();
        std::string function_name = function_data->get_name();
        int environment_id = environment_data->get_id();
        std::string environment_name = environment_data->get_name();
        int argument_count = 1;
        int vararg = 0;
        int missing = 0;
        std::string argument_type = instrumentr_value_type_get_name(val_type);
        std::string expression_type = LAZR_NA_STRING;
        std::string transitive_type = LAZR_NA_STRING;
        std::string value_type = LAZR_NA_STRING;
        int preforced = NA_INTEGER;

        Argument* argument_data = new Argument(argument_id,
                                               call_id,
                                               function_id,
                                               function_name,
                                               environment_id,
                                               environment_name,
                                               argument_position,
                                               argument_name,
                                               argument_count,
                                               vararg,
                                               missing,
                                               argument_type,
                                               expression_type,
                                               transitive_type,
                                               value_type,
                                               preforced);

        insert_(argument_data);
    }

    void insert_(Argument* argument) {
        ++size_;
        int argument_id = argument->get_id();

        auto result = table_.insert({argument_id, {argument}});
        if (!result.second) {
            result.first->second.push_back(argument);
        }
    }
};

#endif /* LAZR_ARGUMENT_TABLE_H */
