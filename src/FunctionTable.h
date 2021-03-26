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
        int fun_id = instrumentr_closure_get_id(closure);

        auto iter = table_.find(fun_id);

        if (iter != table_.end()) {
            return iter->second;
        }

        instrumentr_environment_t environment =
            instrumentr_closure_get_environment(closure);

        int fun_env_id = instrumentr_environment_get_id(environment);

        SEXP r_fun_def = instrumentr_closure_get_sexp(closure);

        std::vector<std::string> fun_defs =
            instrumentr_sexp_to_string(r_fun_def, true);
        std::string fun_def = fun_defs.front();

        std::string fun_hash = instrumentr_compute_hash(fun_def);

        Function* function =
            new Function(fun_id, fun_env_id, fun_hash, fun_def);

        table_.insert({fun_id, function});

        function->set_name(instrumentr_closure_get_name(closure));

        if (instrumentr_closure_is_inner(closure)) {
            Function* parent = insert(instrumentr_closure_get_parent(closure));
            function->set_parent_id(parent->get_id());
        }

        return function;
    }

    Function* lookup(int fun_id) {
        auto result = table_.find(fun_id);
        return result == table_.end() ? nullptr : result->second;
    }

    SEXP to_sexp() {
        int size = table_.size();

        SEXP r_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_qual_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_parent_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_env_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_count = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_hash = PROTECT(allocVector(STRSXP, size));
        SEXP r_fun_def = PROTECT(allocVector(STRSXP, size));

        int index = 0;

        for (auto iter = table_.begin(); iter != table_.end();
             ++iter, ++index) {
            Function* function = iter->second;

            function->to_sexp(index,
                              r_fun_id,
                              r_fun_name,
                              r_qual_name,
                              r_parent_fun_id,
                              r_fun_env_id,
                              r_call_count,
                              r_fun_hash,
                              r_fun_def);
        }

        std::vector<SEXP> columns({r_fun_id,
                                   r_fun_name,
                                   r_qual_name,
                                   r_parent_fun_id,
                                   r_fun_env_id,
                                   r_call_count,
                                   r_fun_hash,
                                   r_fun_def});

        std::vector<std::string> names({"fun_id",
                                        "fun_name",
                                        "qual_name",
                                        "parent_fun_id",
                                        "fun_env_id",
                                        "call_count",
                                        "fun_hash",
                                        "fun_def"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(8);

        return df;
    }

    void infer_qualified_names(EnvironmentTable& env_tab) {
        for (auto iter = table_.begin(); iter != table_.end(); ++iter) {
            int id = iter->first;
            Function* fun = iter->second;
            infer_qualified_name_helper_(fun, env_tab);
        }
    }

  private:
    std::unordered_map<int, Function*> table_;

    std::string infer_qualified_name_helper_(Function* fun,
                                             EnvironmentTable& env_tab) {
        /* if function does not have its own name, no use computing
         * qualified parent's name. */
        if (!fun->has_name()) {
            fun->set_qualified_name(LAZR_NA_STRING);
            return LAZR_NA_STRING;
        }

        /* if function already has a qualified name, then don't compute it
         * again  */
        if (fun->has_qualified_name()) {
            return fun->get_qualified_name();
        }

        std::string fun_name = fun->get_name();

        if (fun->has_parent()) {
            int parent_fun_id = fun->get_parent_id();

            Function* parent = lookup(parent_fun_id);

            std::string parent_name =
                infer_qualified_name_helper_(parent, env_tab);

            if (parent_name == LAZR_NA_STRING) {
                fun_name = LAZR_NA_STRING;
            } else {
                fun_name = parent_name + "::" + fun_name;
            }
        }

        else {
            int fun_env_id = fun->get_fun_env_id();

            Environment* env_data = env_tab.lookup(fun_env_id);

            if (env_data->has_name() && (env_data->get_type() == "namespace" ||
                                         env_data->get_type() == "package")) {
                std::string pack_name = env_data->get_name();
                fun_name = pack_name + "::" + fun_name;
            } else {
                fun_name = LAZR_NA_STRING;
            }
        }

        fun->set_qualified_name(fun_name);

        return fun_name;
    }
};

#endif /* LAZR_FUNCTION_TABLE_H */
