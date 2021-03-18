#ifndef LAZR_CALL_H
#define LAZR_CALL_H

#include <string>
#include <vector>
#include "utilities.h"

class Call {
  public:
    Call(int call_id,
         int function_id,
         const std::string& package_name,
         const std::string& function_name)
        : call_id_(call_id)
        , function_id_(function_id)
        , package_name_(package_name)
        , function_name_(function_name)
        , successful_(false)
        , result_type_(LAZR_NA_STRING)
        , force_order_("") {
    }

    int get_call_id() {
        return call_id_;
    }

    void set_result(const std::string& result_type) {
        result_type_ = result_type;
        successful_ = result_type != LAZR_NA_STRING;
    }

    void force_argument(int position) {
        if (!force_order_.empty()) {
            force_order_.append(",");
        }

        force_order_.append(std::to_string(position));
    }

    const std::string& get_package_name() const {
        return package_name_;
    }

    const std::string& get_function_name() const {
        return function_name_;
    }

    void to_sexp(int position,
                 SEXP r_call_id,
                 SEXP r_function_id,
                 SEXP r_package_name,
                 SEXP r_function_name,
                 SEXP r_successful,
                 SEXP r_result_type,
                 SEXP r_force_order) {
        INTEGER(r_call_id)[position] = call_id_;
        INTEGER(r_function_id)[position] = function_id_;
        SET_STRING_ELT(r_package_name, position, make_char(package_name_));
        SET_STRING_ELT(r_function_name, position, make_char(function_name_));
        LOGICAL(r_successful)[position] = successful_;
        SET_STRING_ELT(r_result_type, position, make_char(result_type_));
        SET_STRING_ELT(r_force_order, position, make_char(force_order_));
    }

  private:
    int call_id_;
    int function_id_;
    const std::string package_name_;
    const std::string function_name_;
    bool successful_;
    std::string result_type_;
    std::string force_order_;
};

#endif /* LAZR_CALL_H */
