#ifndef LAZR_EFFECTS_TABLE_H
#define LAZR_EFFECTS_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class EffectsTable {
  public:
    EffectsTable() {
    }

    ~EffectsTable() {
    }

    void insert(const char type,
                const std::string& var_name,
                bool transitive,
                instrumentr_promise_t promise,
                Environment* environment) {
        int arg_id = instrumentr_promise_get_id(promise);

        type_.push_back(std::string(1, type));
        var_name_.push_back(var_name);
        transitive_.push_back(transitive);
        arg_id_.push_back(arg_id);
        env_id_.push_back(environment->get_id());
    }

    SEXP to_sexp() {
        int size = type_.size();

        SEXP r_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_var_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_env_id = PROTECT(allocVector(INTSXP, size));

        int index = 0;
        for (int index = 0; index < size; ++index) {
            SET_STRING_ELT(r_type, index, make_char(type_[index]));
            SET_STRING_ELT(r_var_name, index, make_char(var_name_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
            SET_INTEGER_ELT(r_arg_id, index, arg_id_[index]);
            SET_INTEGER_ELT(r_env_id, index, env_id_[index]);
        }

        std::vector<SEXP> columns(
            {r_type, r_var_name, r_transitive, r_arg_id, r_env_id});

        std::vector<std::string> names(
            {"type", "var_name", "transitive", "arg_id", "env_id"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(5);

        return df;
    }

  private:
    std::vector<std::string> type_;
    std::vector<std::string> var_name_;
    std::vector<bool> transitive_;
    std::vector<int> arg_id_;
    std::vector<int> env_id_;
};

#endif /* LAZR_EFFECTS_TABLE_H */
