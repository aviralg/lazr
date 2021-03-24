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

    Call* insert(instrumentr_call_t call,
                 Function* function,
                 Environment* environment) {
        int call_id = instrumentr_call_get_id(call);

        auto iter = table_.find(call_id);

        if (iter != table_.end()) {
            return iter->second;
        }

        Call* call_data = new Call(call_id,
                                   function->get_id(),
                                   function->get_name(),
                                   environment->get_id(),
                                   environment->get_name());

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
        SEXP r_function_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_environment_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_environment_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_successful = PROTECT(allocVector(LGLSXP, size));
        SEXP r_result_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_force_order = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Call* call = iter->second;

            call->to_sexp(index,
                          r_call_id,
                          r_function_id,
                          r_function_name,
                          r_environment_id,
                          r_environment_name,
                          r_successful,
                          r_result_type,
                          r_force_order);
        }

        std::vector<SEXP> columns({r_call_id,
                                   r_function_id,
                                   r_function_name,
                                   r_environment_id,
                                   r_environment_name,
                                   r_successful,
                                   r_result_type,
                                   r_force_order});

        std::vector<std::string> names({"call_id",
                                        "function_id",
                                        "function_name",
                                        "environment_id",
                                        "environment_name",
                                        "successful",
                                        "result_type",
                                        "force_order"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(8);

        return df;
    }

  private:
    std::unordered_map<int, Call*> table_;
};

#endif /* LAZR_CALL_TABLE_H */
