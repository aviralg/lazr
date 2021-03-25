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
        , force_order_({})
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
        force_order_.push_back(position);
    }

    int get_force_position() {
        return force_order_.size();
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

        std::string force_order_str = intvec_to_string_(force_order_);
        SET_STRING_ELT(
            r_force_order, position, make_char(force_order_str.c_str()));
    }

  private:
    int call_id_;
    int function_id_;
    const std::string function_name_;
    int environment_id_;
    const std::string environment_name_;
    bool successful_;
    std::string result_type_;
    std::vector<int> force_order_;
    bool exit_;

    std::string intvec_to_string_(const std::vector<int>& vec) {
        std::string str;
        int size = vec.size();

        for (int i = 0; i < size - 1; ++i) {
            str.append(std::to_string(vec[i]));
            str.append("|");
        }

        if (size != 0) {
            str.append(std::to_string(vec[size - 1]));
        }

        return str;
    }
};

#endif /* LAZR_CALL_H */
