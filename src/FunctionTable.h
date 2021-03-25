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

    Function* insert(instrumentr_closure_t closure) {
        int function_id = instrumentr_closure_get_id(closure);

        auto iter = table_.find(function_id);

        if (iter != table_.end()) {
            return iter->second;
        }

        instrumentr_environment_t environment =
            instrumentr_closure_get_environment(closure);

        int environment_id = instrumentr_environment_get_id(environment);

        SEXP r_definition = instrumentr_closure_get_sexp(closure);

        std::vector<std::string> definitions =
            instrumentr_sexp_to_string(r_definition, true);
        std::string definition = definitions.front();

        std::string hash = instrumentr_compute_hash(definition);

        Function* function =
            new Function(function_id, environment_id, hash, definition);

        table_.insert({function_id, function});

        function->set_name(instrumentr_closure_get_name(closure));

        if (instrumentr_closure_is_inner(closure)) {
            Function* parent = insert(instrumentr_closure_get_parent(closure));
            function->set_parent_id(parent->get_id());
        }

        return function;
    }

    Function* lookup(int function_id) {
        auto result = table_.find(function_id);
        return result == table_.end() ? nullptr : result->second;
    }

    SEXP to_sexp() {
        fix_names_();

        int size = table_.size();

        SEXP r_function_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_function_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_parent_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_environment_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_count = PROTECT(allocVector(INTSXP, size));
        SEXP r_hash = PROTECT(allocVector(STRSXP, size));
        SEXP r_definition = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Function* function = iter->second;

            function->to_sexp(index,
                              r_function_id,
                              r_function_name,
                              r_parent_id,
                              r_environment_id,
                              r_call_count,
                              r_hash,
                              r_definition);
        }

        std::vector<SEXP> columns({r_function_id,
                                   r_function_name,
                                   r_parent_id,
                                   r_environment_id,
                                   r_call_count,
                                   r_hash,
                                   r_definition});

        std::vector<std::string> names({"function_id",
                                        "function_name",
                                        "parent_id",
                                        "environment_id",
                                        "call_count",
                                        "hash",
                                        "definition"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(7);

        return df;
    }

  private:
    std::unordered_map<int, Function*> table_;
};

#endif /* LAZR_FUNCTION_TABLE_H */
