#ifndef LAZR_CALL_H
#define LAZR_CALL_H

#include <string>
#include <vector>
#include "utilities.h"

class Call {
  public:
    Call(int call_id,
         int function_id,
         const std::string& function_name,
         int environment_id,
         const std::string& environment_name)
        : call_id_(call_id)
        , function_id_(function_id)
        , function_name_(function_name)
        , environment_id_(environment_id)
        , environment_name_(environment_name)
        , successful_(false)
        , result_type_(LAZR_NA_STRING)
        , force_order_("")
        , exit_(false) {
    }

    int get_id() {
        return call_id_;
    }

    void exit(const std::string& result_type) {
        exit_ = true;
        result_type_ = result_type;
        successful_ = result_type != LAZR_NA_STRING;
    }

    bool has_exited() const {
        return exit_;
    }

    void force_argument(int position) {
        if (!force_order_.empty()) {
            force_order_.append(",");
        }

        force_order_.append(std::to_string(position));
    }

    const std::string& get_environment_name() const {
        return environment_name_;
    }

    const std::string& get_function_name() const {
        return function_name_;
    }

    void to_sexp(int position,
                 SEXP r_call_id,
                 SEXP r_function_id,
                 SEXP r_function_name,
                 SEXP r_environment_id,
                 SEXP r_environment_name,
                 SEXP r_successful,
                 SEXP r_result_type,
                 SEXP r_force_order) {
        SET_INTEGER_ELT(r_call_id, position, call_id_);
        SET_INTEGER_ELT(r_function_id, position, function_id_);
        SET_STRING_ELT(r_function_name, position, make_char(function_name_));
        SET_INTEGER_ELT(r_environment_id, position, environment_id_);
        SET_STRING_ELT(
            r_environment_name, position, make_char(environment_name_));
        LOGICAL(r_successful)[position] = successful_;
        SET_STRING_ELT(r_result_type, position, make_char(result_type_));
        SET_STRING_ELT(r_force_order, position, make_char(force_order_));
    }

  private:
    int call_id_;
    int function_id_;
    const std::string function_name_;
    int environment_id_;
    const std::string environment_name_;
    bool successful_;
    std::string result_type_;
    std::string force_order_;
    bool exit_;
};

#endif /* LAZR_CALL_H */
