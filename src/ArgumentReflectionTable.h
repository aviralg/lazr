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
                int fun_id,
                int call_id,
                int arg_id,
                int formal_pos) {
        ref_call_id_.push_back(ref_call_id);
        ref_type_.push_back(ref_type);
        transitive_.push_back(transitive);
        fun_id_.push_back(fun_id);
        call_id_.push_back(call_id);
        arg_id_.push_back(arg_id);
        formal_pos_.push_back(formal_pos);
    }

    SEXP to_sexp() {
        int size = ref_call_id_.size();

        SEXP r_ref_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_ref_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_formal_pos = PROTECT(allocVector(INTSXP, size));

        for (int index = 0; index < size; ++index) {
            SET_INTEGER_ELT(r_ref_call_id, index, ref_call_id_[index]);
            SET_STRING_ELT(r_ref_type, index, make_char(ref_type_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
            SET_INTEGER_ELT(r_fun_id, index, fun_id_[index]);
            SET_INTEGER_ELT(r_call_id, index, call_id_[index]);
            SET_INTEGER_ELT(r_arg_id, index, arg_id_[index]);
            SET_INTEGER_ELT(r_formal_pos, index, formal_pos_[index]);
        }

        std::vector<SEXP> columns({r_ref_call_id,
                                   r_ref_type,
                                   r_transitive,
                                   r_fun_id,
                                   r_call_id,
                                   r_arg_id,
                                   r_formal_pos});

        std::vector<std::string> names({"ref_call_id",
                                        "ref_type",
                                        "transitive",
                                        "fun_id",
                                        "call_id",
                                        "arg_id",
                                        "formal_pos"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(7);

        return df;
    }

  private:
    std::vector<int> ref_call_id_;
    std::vector<std::string> ref_type_;
    std::vector<bool> transitive_;
    std::vector<int> fun_id_;
    std::vector<int> call_id_;
    std::vector<int> arg_id_;
    std::vector<int> formal_pos_;
};

#endif /* LAZR_ARGUMENT_REFLECTION_TABLE_H */
