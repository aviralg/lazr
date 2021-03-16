#ifndef LAZR_CALL_TABLE_H
#define LAZR_CALL_TABLE_H

#include "Call.h"
#include <unordered_map>

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

    void insert(Call* call) {
        int call_id = call->get_call_id();
        auto result = table_.insert({call_id, call});
        if (!result.second) {
            Rf_error("call with id %d is already present in table", call_id);
        }
    }

    Call& lookup(int call_id) {
        auto result = table_.find(call_id);
        if (result == table_.end()) {
            Rf_error("cannot find call with id %d", call_id);
        }
        return *(result->second);
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_package_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_successful = PROTECT(allocVector(LGLSXP, size));
        SEXP r_result_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_force_order = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Call* call = iter->second;

            call->to_sexp(index,
                          r_call_id,
                          r_package_name,
                          r_function_name,
                          r_successful,
                          r_result_type,
                          r_force_order);
        }

        std::vector<SEXP> columns({r_call_id,
                                   r_package_name,
                                   r_function_name,
                                   r_successful,
                                   r_result_type,
                                   r_force_order});

        std::vector<std::string> names({"call_id",
                                        "package_name",
                                        "function_name",
                                        "successful",
                                        "result_type",
                                        "force_order"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(6);

        return df;
    }

  private:
    std::unordered_map<int, Call*> table_;
};

#endif /* LAZR_CALL_TABLE_H */
