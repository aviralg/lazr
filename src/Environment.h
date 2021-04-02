#ifndef LAZR_ENVIRONMENT_H
#define LAZR_ENVIRONMENT_H

#include <string>
#include "utilities.h"

class Environment {
  public:
    Environment(int env_id, int call_id)
        : env_id_(env_id)
        , env_type_(LAZR_NA_STRING)
        , env_name_(LAZR_NA_STRING)
        , call_id_(call_id) {
    }

    int get_id() {
        return env_id_;
    }

    bool has_name() const {
        return env_name_ != LAZR_NA_STRING;
    }
    const std::string& get_name() const {
        return env_name_;
    }

    const std::string& get_type() const {
        return env_type_;
    }

    void set_name(const char* env_name) {
        env_name_ = charptr_to_string(env_name);
    }

    void set_type(const char* env_type) {
        env_type_ = charptr_to_string(env_type);
    }

    int get_call_id() const {
        return call_id_;
    }

    void set_call_id(int call_id) {
        call_id_ = call_id;
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
    std::string env_type_;
    std::string env_name_;
    int call_id_;
};

#endif /* LAZR_ENVIRONMENT_H */
