#ifndef LAZR_CALL_TABLE_H
#define LAZR_CALL_TABLE_H

#include "Call.h"
#include <unordered_map>
#include "Function.h"
#include "Environment.h"
#include <instrumentr/instrumentr.h>

class CallTable {
  public:
    CallTable() {
    }

    ~CallTable() {
        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            delete iter->second;
        }
        table_.clear();
    }

    Call* insert(instrumentr_call_t call, Function* function) {
        int call_id = instrumentr_call_get_id(call);

        instrumentr_environment_t environment =
            instrumentr_call_get_environment(call);

        int env_id = instrumentr_environment_get_id(environment);

        auto iter = table_.find(call_id);

        if (iter != table_.end()) {
            return iter->second;
        }

        instrumentr_language_t call_expr =
            instrumentr_call_get_expression(call);

        SEXP r_call_expr = instrumentr_language_get_sexp(call_expr);

        std::vector<std::string> call_exprs =
            instrumentr_sexp_to_string(r_call_expr, true);

        Call* call_data =
            new Call(call_id, function->get_id(), env_id, call_exprs.front());

        auto result = table_.insert({call_id, call_data});
        return result.first->second;
    }

    Call* lookup(int call_id) {
        auto result = table_.find(call_id);
        if (result == table_.end()) {
            Rf_error("cannot find call with id %d", call_id);
        }
        return result->second;
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_env_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_successful = PROTECT(allocVector(LGLSXP, size));
        SEXP r_result_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_force_order = PROTECT(allocVector(STRSXP, size));
        SEXP r_esc_env = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_expr = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Call* call = iter->second;

            call->to_sexp(index,
                          r_call_id,
                          r_fun_id,
                          r_env_id,
                          r_successful,
                          r_result_type,
                          r_force_order,
                          r_esc_env,
                          r_call_expr);
        }

        std::vector<SEXP> columns({r_call_id,
                                   r_fun_id,
                                   r_env_id,
                                   r_successful,
                                   r_result_type,
                                   r_force_order,
                                   r_esc_env,
                                   r_call_expr});

        std::vector<std::string> names({"call_id",
                                        "fun_id",
                                        "env_id",
                                        "successful",
                                        "result_type",
                                        "force_order",
                                        "esc_env",
                                        "call_expr"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(8);

        return df;
    }

  private:
    std::unordered_map<int, Call*> table_;
};

#endif /* LAZR_CALL_TABLE_H */
