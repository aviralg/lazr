#ifndef LAZR_FUNCTION_TABLE_H
#define LAZR_FUNCTION_TABLE_H

#include "Function.h"
#include <unordered_map>

class FunctionTable {
  public:
    FunctionTable() {
    }

    ~FunctionTable() {
        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            delete iter->second;
        }
        table_.clear();
    }

    void insert(Function* function) {
        int function_id = function->get_function_id();
        auto result = table_.insert({function_id, function});
        if (!result.second) {
            Rf_error("function with id %d is already present in table",
                     function_id);
        }
    }

    Function* lookup(int function_id) {
        auto result = table_.find(function_id);
        return result == table_.end() ? nullptr : result->second;
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_function_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_package_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_hash = PROTECT(allocVector(STRSXP, size));
        SEXP r_call_count = PROTECT(allocVector(INTSXP, size));
        SEXP r_definition = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Function* function = iter->second;

            function->to_sexp(index,
                              r_function_id,
                              r_package_name,
                              r_function_name,
                              r_hash,
                              r_call_count,
                              r_definition);
        }

        std::vector<SEXP> columns({r_function_id,
                                   r_package_name,
                                   r_function_name,
                                   r_hash,
                                   r_call_count,
                                   r_definition});

        std::vector<std::string> names({"function_id",
                                        "package_name",
                                        "function_name",
                                        "hash",
                                        "call_count",
                                        "definition"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(6);

        return df;
    }

  private:
    std::unordered_map<int, Function*> table_;
};

#endif /* LAZR_FUNCTION_TABLE_H */
