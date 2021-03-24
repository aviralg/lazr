#ifndef LAZR_ENVIRONMENT_H
#define LAZR_ENVIRONMENT_H

#include <string>
#include "utilities.h"

class Environment {
  public:
    Environment(int env_id,
                const std::string& env_type,
                const std::string& env_name,
                int call_id)
        : env_id_(env_id)
        , env_type_(env_type)
        , env_name_(env_name)
        , call_id_(call_id) {
    }

    int get_id() {
        return env_id_;
    }

    const std::string& get_name() const {
        return env_name_;
    }

    void to_sexp(int position,
                 SEXP r_env_id,
                 SEXP r_env_type,
                 SEXP r_env_name,
                 SEXP r_call_id) {
        SET_INTEGER_ELT(r_env_id, position, env_id_);
        SET_STRING_ELT(r_env_type, position, make_char(env_type_));
        SET_STRING_ELT(r_env_name, position, make_char(env_name_));
        SET_INTEGER_ELT(r_call_id, position, call_id_);
    }

  private:
    int env_id_;
    const std::string env_type_;
    const std::string env_name_;
    int call_id_;
};

#endif /* LAZR_ENVIRONMENT_H */
