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
                int env_id,
                int source_fun_id,
                int source_call_id,
                int source_arg_id,
                int source_formal_pos,
                int fun_id,
                int call_id,
                int arg_id,
                int formal_pos,
                const std::string& backtrace) {
        type_.push_back(std::string(1, type));
        var_name_.push_back(var_name);
        transitive_.push_back(transitive);
        env_id_.push_back(env_id);
        source_fun_id_.push_back(source_fun_id);
        source_call_id_.push_back(source_call_id);
        source_arg_id_.push_back(source_arg_id);
        source_formal_pos_.push_back(source_formal_pos);
        fun_id_.push_back(fun_id);
        call_id_.push_back(call_id);
        arg_id_.push_back(arg_id);
        formal_pos_.push_back(formal_pos);
        backtrace_.push_back(backtrace);
    }

    SEXP to_sexp() {
        int size = type_.size();

        SEXP r_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_var_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_env_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_backtrace = PROTECT(allocVector(STRSXP, size));

        int index = 0;
        for (int index = 0; index < size; ++index) {
            SET_STRING_ELT(r_type, index, make_char(type_[index]));
            SET_STRING_ELT(r_var_name, index, make_char(var_name_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
            SET_INTEGER_ELT(r_env_id, index, env_id_[index]);
            SET_INTEGER_ELT(r_source_fun_id, index, source_fun_id_[index]);
            SET_INTEGER_ELT(r_source_call_id, index, source_call_id_[index]);
            SET_INTEGER_ELT(r_source_arg_id, index, source_arg_id_[index]);
            SET_INTEGER_ELT(
                r_source_formal_pos, index, source_formal_pos_[index]);
            SET_INTEGER_ELT(r_fun_id, index, fun_id_[index]);
            SET_INTEGER_ELT(r_call_id, index, call_id_[index]);
            SET_INTEGER_ELT(r_arg_id, index, arg_id_[index]);
            SET_INTEGER_ELT(r_formal_pos, index, formal_pos_[index]);
            SET_STRING_ELT(r_backtrace, index, make_char(backtrace_[index]));
        }

        std::vector<SEXP> columns({r_type,
                                   r_var_name,
                                   r_transitive,
                                   r_env_id,
                                   r_source_fun_id,
                                   r_source_call_id,
                                   r_source_arg_id,
                                   r_source_formal_pos,
                                   r_fun_id,
                                   r_call_id,
                                   r_arg_id,
                                   r_formal_pos,
                                   r_backtrace});

        std::vector<std::string> names({"type",
                                        "var_name",
                                        "transitive",
                                        "env_id",
                                        "source_fun_id",
                                        "source_call_id",
                                        "source_arg_id",
                                        "source_formal_pos",
                                        "fun_id",
                                        "call_id",
                                        "arg_id",
                                        "formal_pos",
                                        "backtrace"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(13);

        return df;
    }

  private:
    std::vector<std::string> type_;
    std::vector<std::string> var_name_;
    std::vector<bool> transitive_;
    std::vector<int> env_id_;
    std::vector<int> source_fun_id_;
    std::vector<int> source_call_id_;
    std::vector<int> source_arg_id_;
    std::vector<int> source_formal_pos_;
    std::vector<int> fun_id_;
    std::vector<int> call_id_;
    std::vector<int> arg_id_;
    std::vector<int> formal_pos_;
    std::vector<std::string> backtrace_;
};

#endif /* LAZR_EFFECTS_TABLE_H */
