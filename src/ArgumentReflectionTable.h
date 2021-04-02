#ifndef LAZR_ARGUMENT_REFLECTION_TABLE_H
#define LAZR_ARGUMENT_REFLECTION_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class ArgumentReflectionTable {
  public:
    ArgumentReflectionTable() {
    }

    ~ArgumentReflectionTable() {
    }

    void insert(int ref_call_id,
                const std::string& ref_type,
                bool transitive,
                int source_fun_id,
                int source_call_id,
                int source_arg_id,
                int source_formal_pos,
                int fun_id,
                int call_id,
                int arg_id,
                int formal_pos,
                const std::string& backtrace) {
        ref_call_id_.push_back(ref_call_id);
        ref_type_.push_back(ref_type);
        transitive_.push_back(transitive);
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
        int size = ref_call_id_.size();

        SEXP r_ref_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_ref_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_source_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_backtrace = PROTECT(allocVector(STRSXP, size));

        for (int index = 0; index < size; ++index) {
            SET_INTEGER_ELT(r_ref_call_id, index, ref_call_id_[index]);
            SET_STRING_ELT(r_ref_type, index, make_char(ref_type_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
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

        std::vector<SEXP> columns({r_ref_call_id,
                                   r_ref_type,
                                   r_transitive,
                                   r_source_fun_id,
                                   r_source_call_id,
                                   r_source_arg_id,
                                   r_source_formal_pos,
                                   r_fun_id,
                                   r_call_id,
                                   r_arg_id,
                                   r_formal_pos,
                                   r_backtrace});

        std::vector<std::string> names({"ref_call_id",
                                        "ref_type",
                                        "transitive",
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

        UNPROTECT(12);

        return df;
    }

  private:
    std::vector<int> ref_call_id_;
    std::vector<std::string> ref_type_;
    std::vector<bool> transitive_;
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

#endif /* LAZR_ARGUMENT_REFLECTION_TABLE_H */
