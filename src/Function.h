#ifndef LAZR_FUNCTION_H
#define LAZR_FUNCTION_H

#include <string>
#include <vector>
#include "utilities.h"

class Function {
  public:
    Function(int function_id,
             const std::string& environment_name,
             const std::string& function_name,
             const std::string& hash,
             const std::string& definition)
        : function_id_(function_id)
        , environment_name_(environment_name)
        , function_name_(function_name)
        , hash_(hash)
        , call_count_(0)
        , definition_(definition) {
    }

    int get_id() {
        return function_id_;
    }

    const std::string& get_name() const {
        return function_name_;
    }

    void call() {
        ++call_count_;
    }

    void to_sexp(int index,
                 SEXP r_function_id,
                 SEXP r_environment_name,
                 SEXP r_function_name,
                 SEXP r_hash,
                 SEXP r_call_count,
                 SEXP r_definition) {
        INTEGER(r_function_id)[index] = function_id_;
        SET_STRING_ELT(r_environment_name, index, make_char(environment_name_));
        SET_STRING_ELT(r_function_name, index, make_char(function_name_));
        SET_STRING_ELT(r_hash, index, make_char(hash_));
        INTEGER(r_call_count)[index] = call_count_;
        SET_STRING_ELT(r_definition, index, make_char(definition_));
    }

  private:
    int function_id_;
    const std::string environment_name_;
    const std::string function_name_;
    std::string hash_;
    int call_count_;
    std::string definition_;
};

#endif /* LAZR_FUNCTION_H */
