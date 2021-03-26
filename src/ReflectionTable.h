#ifndef LAZR_REFLECTION_TABLE_H
#define LAZR_REFLECTION_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class ReflectionTable {
  public:
    ReflectionTable() {
    }

    ~ReflectionTable() {
    }

    void insert(const std::string& type,
                bool transitive,
                instrumentr_promise_t promise) {
        int arg_id = instrumentr_promise_get_id(promise);

        type_.push_back(type);
        transitive_.push_back(transitive);
        arg_id_.push_back(arg_id);
    }

    SEXP to_sexp() {
        int size = type_.size();

        SEXP r_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_arg_id = PROTECT(allocVector(INTSXP, size));

        int index = 0;
        for (int index = 0; index < size; ++index) {
            SET_STRING_ELT(r_type, index, make_char(type_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
            SET_INTEGER_ELT(r_arg_id, index, arg_id_[index]);
        }

        std::vector<SEXP> columns({r_type, r_transitive, r_arg_id});

        std::vector<std::string> names({"type", "transitive", "arg_id"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(3);

        return df;
    }

  private:
    std::vector<std::string> type_;
    std::vector<bool> transitive_;
    std::vector<int> arg_id_;
};

#endif /* LAZR_REFLECTION_TABLE_H */
