#ifndef LAZR_SIDE_EFFECTS_TABLE_H
#define LAZR_SIDE_EFFECTS_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class SideEffectsTable {
  public:
    SideEffectsTable() {
    }

    ~SideEffectsTable() {
    }

    void insert(const std::string& type,
                const std::string& varname,
                bool transitive,
                instrumentr_promise_t promise,
                Environment* environment) {
        int promise_id = instrumentr_promise_get_id(promise);

        type_.push_back(type);
        varname_.push_back(varname);
        transitive_.push_back(transitive);
        promise_id_.push_back(promise_id);
        environment_id_.push_back(environment->get_id());
        environment_type_.push_back(environment->get_type());
        environment_name_.push_back(environment->get_name());
        environment_call_id_.push_back(environment->get_call_id());
    }

    SEXP to_sexp() {
        int size = type_.size();

        SEXP r_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_varname = PROTECT(allocVector(STRSXP, size));
        SEXP r_transitive = PROTECT(allocVector(LGLSXP, size));
        SEXP r_promise_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_environment_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_environment_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_environment_name = PROTECT(allocVector(STRSXP, size));
        SEXP r_environment_call_id = PROTECT(allocVector(INTSXP, size));

        int index = 0;
        for (int index = 0; index < size; ++index) {
            SET_STRING_ELT(r_type, index, make_char(type_[index]));
            SET_STRING_ELT(r_varname, index, make_char(varname_[index]));
            SET_LOGICAL_ELT(r_transitive, index, transitive_[index]);
            SET_INTEGER_ELT(r_promise_id, index, promise_id_[index]);
            SET_INTEGER_ELT(r_environment_id, index, environment_id_[index]);
            SET_STRING_ELT(
                r_environment_type, index, make_char(environment_type_[index]));
            SET_STRING_ELT(
                r_environment_name, index, make_char(environment_name_[index]));
            SET_INTEGER_ELT(
                r_environment_call_id, index, environment_call_id_[index]);
        }

        std::vector<SEXP> columns({r_type,
                                   r_varname,
                                   r_transitive,
                                   r_promise_id,
                                   r_environment_id,
                                   r_environment_type,
                                   r_environment_name,
                                   r_environment_call_id});

        std::vector<std::string> names({"type",
                                        "varname",
                                        "transitive",
                                        "promise_id",
                                        "environment_id",
                                        "environment_type",
                                        "environment_name",
                                        "environment_call_id"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(8);

        return df;
    }

  private:
    std::vector<std::string> type_;
    std::vector<std::string> varname_;
    std::vector<bool> transitive_;
    std::vector<int> promise_id_;
    std::vector<int> environment_id_;
    std::vector<std::string> environment_type_;
    std::vector<std::string> environment_name_;
    std::vector<int> environment_call_id_;
};

#endif /* LAZR_SIDE_EFFECTS_TABLE_H */
