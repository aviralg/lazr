#ifndef LAZR_FUNCTION_H
#define LAZR_FUNCTION_H

#include <string>
#include <vector>
#include "utilities.h"

class Function {
  public:
    Function(int function_id,
             int environment_id,
             const std::string& hash,
             const std::string& definition)
        : function_id_(function_id)
        , function_name_(LAZR_NA_STRING)
        , environment_id_(environment_id)
        , call_count_(0)
        , hash_(hash)
        , definition_(definition) {
    }

    int get_id() {
        return function_id_;
    }

    const std::string& get_name() const {
        return function_name_;
    }

    void set_name(const char* name) {
        function_name_ = charptr_to_string(name);
    }

    void call() {
        ++call_count_;
    }

    void to_sexp(int index,
                 SEXP r_function_id,
                 SEXP r_function_name,
                 SEXP r_environment_id,
                 SEXP r_call_count,
                 SEXP r_hash,
                 SEXP r_definition) {
        SET_INTEGER_ELT(r_function_id, index, function_id_);
        SET_STRING_ELT(r_function_name, index, make_char(function_name_));
        SET_INTEGER_ELT(r_environment_id, index, environment_id_);
        SET_INTEGER_ELT(r_call_count, index, call_count_);
        SET_STRING_ELT(r_hash, index, make_char(hash_));
        SET_STRING_ELT(r_definition, index, make_char(definition_));
    }

  private:
    int function_id_;
    std::string function_name_;
    int environment_id_;
    int call_count_;
    std::string hash_;
    std::string definition_;
};

#endif /* LAZR_FUNCTION_H */
