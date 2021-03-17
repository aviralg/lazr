#ifndef LAZR_ARGUMENT_TABLE_H
#define LAZR_ARGUMENT_TABLE_H

#include "Argument.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "utilities.h"

class ArgumentTable {
  public:
    ArgumentTable() {
    }

    ~ArgumentTable() {
        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            delete iter->second;
        }
        table_.clear();
    }

    void insert(Argument* argument) {
        int parameter_id = argument->get_parameter_id();

        auto result = table_.insert({parameter_id, argument});
        if (!result.second) {
            Rf_error("argument with id %d is already present in table",
                     parameter_id);
        }
    }

    Argument& lookup(int argument_id) {
        auto result = table_.find(argument_id);
        if (result == table_.end()) {
            Rf_error("cannot find argument with id %d", argument_id);
        }
        return *(result->second);
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_parameter_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_package_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_parameter_position = PROTECT(allocVector(INTSXP, size));
        SEXP r_parameter_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_parameter_count = PROTECT(allocVector(INTSXP, size));
        SEXP r_vararg = PROTECT(allocVector(LGLSXP, size));
        SEXP r_missing = PROTECT(allocVector(LGLSXP, size));
        SEXP r_argument_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_expression_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_value_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_preforced = PROTECT(allocVector(INTSXP, size));
        SEXP r_cap_force = PROTECT(allocVector(INTSXP, size));
        SEXP r_cap_meta = PROTECT(allocVector(INTSXP, size));
        SEXP r_cap_lookup = PROTECT(allocVector(INTSXP, size));
        SEXP r_escaped = PROTECT(allocVector(LGLSXP, size));
        SEXP r_esc_force = PROTECT(allocVector(INTSXP, size));
        SEXP r_esc_meta = PROTECT(allocVector(INTSXP, size));
        SEXP r_esc_lookup = PROTECT(allocVector(INTSXP, size));
        SEXP r_force_depth = PROTECT(allocVector(INTSXP, size));
        SEXP r_force_source = PROTECT(allocVector(STRSXP, size));
        SEXP r_companion_position = PROTECT(allocVector(INTSXP, size));
        SEXP r_event_sequence = PROTECT(allocVector(STRSXP, size));

        int index = 0;

        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Argument* argument = iter->second;

            argument->to_sexp(index,
                              r_parameter_id,
                              r_call_id,
                              r_package_name,
                              r_function_name,
                              r_parameter_position,
                              r_parameter_name,
                              r_parameter_count,
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
                              r_event_sequence);
        }

        std::vector<SEXP> columns({r_parameter_id,
                                   r_call_id,
                                   r_package_name,
                                   r_function_name,
                                   r_parameter_position,
                                   r_parameter_name,
                                   r_parameter_count,
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
                                   r_event_sequence});

        std::vector<std::string> names({"parameter_id",
                                        "call_id",
                                        "package_name",
                                        "function_name",
                                        "parameter_position",
                                        "parameter_name",
                                        "parameter_count",
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
                                        "event_sequence"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(25);

        return df;
    }

  private:
    std::unordered_map<int, Argument*> table_;
};

#endif /* LAZR_ARGUMENT_TABLE_H */
