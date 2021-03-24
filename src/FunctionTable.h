#ifndef LAZR_FUNCTION_TABLE_H
#define LAZR_FUNCTION_TABLE_H

#include "Function.h"
#include "Environment.h"
#include <unordered_map>
#include <instrumentr/instrumentr.h>

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

    Function* insert(instrumentr_closure_t closure, Environment* environment) {
        int function_id = instrumentr_closure_get_id(closure);

        auto iter = table_.find(function_id);

        if (iter != table_.end()) {
            return iter->second;
        }

        const char* name = instrumentr_closure_get_name(closure);
        const std::string function_name(name == NULL ? LAZR_NA_STRING : name);

        SEXP r_definition = instrumentr_closure_get_sexp(closure);

        std::vector<std::string> definitions =
            instrumentr_sexp_to_string(r_definition, true);
        std::string definition = definitions.front();

        std::string hash = instrumentr_compute_hash(definition);

        Function* function_data = new Function(function_id,
                                               environment->get_name(),
                                               function_name,
                                               hash,
                                               definition);

        auto result = table_.insert({function_id, function_data});

        return result.first->second;
    }

    Function* lookup(int function_id) {
        auto result = table_.find(function_id);
        return result == table_.end() ? nullptr : result->second;
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_function_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_environment_name = PROTECT(allocVector(STRSXP, size));
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
                              r_environment_name,
                              r_function_name,
                              r_hash,
                              r_call_count,
                              r_definition);
        }

        std::vector<SEXP> columns({r_function_id,
                                   r_environment_name,
                                   r_function_name,
                                   r_hash,
                                   r_call_count,
                                   r_definition});

        std::vector<std::string> names({"function_id",
                                        "environment_name",
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
