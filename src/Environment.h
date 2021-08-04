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

    void add_read_time(const std::string& varname, int time) {
        add_time_(reads_, varname, time);
    }

    void add_write_time(const std::string& varname, int time) {
        add_time_(writes_, varname, time);
    }

    void clear_variable_times() {
        reads_.clear();
        writes_.clear();
    }

    bool
    is_variable_touched(const std::string& varname, int t_left, int t_right) {
        return has_time_(reads_, varname, t_left, t_right) ||
               has_time_(writes_, varname, t_left, t_right);
    }

    bool
    is_variable_modified(const std::string& varname, int t_left, int t_right) {
        return has_time_(writes_, varname, t_left, t_right);
    }

  private:
    void add_time_(std::unordered_map<std::string, std::vector<int>>& table,
                   const std::string& varname,
                   int time) {
        auto iter = table.find(varname);

        if (iter == table.end()) {
            table.insert({varname, {time}});
        }

        else {
            std::vector<int>& seq = iter->second;

            minimize_sequence_(seq);

            seq.push_back(time);
        }
    }

    bool has_time_(std::unordered_map<std::string, std::vector<int>>& table,
                   const std::string& varname,
                   const int t_left,
                   const int t_right) {
        auto iter = table.find(varname);

        if (iter == table.end()) {
            return false;
        }

        const std::vector<int>& seq = iter->second;

        for (int i = seq.size() - 1; i >= 0; --i) {
            int t = seq[i];

            if (t < t_left) {
                return false;
            }

            if (t < t_right && t > t_left) {
                return true;
            }
        }

        return false;
    }

    void minimize_sequence_(std::vector<int>& seq) {
        const int MAX_SIZE = 10000;
        const int BLOCK_SIZE = 1000;
        const int size = seq.size();

        if (size > MAX_SIZE) {
            const int left_boundary = size - BLOCK_SIZE;

            for (int i = 0; i < BLOCK_SIZE; ++i) {
                seq[i] = seq[i + left_boundary];
            }

            seq.erase(seq.begin() + BLOCK_SIZE, seq.end());
        }
    }

    int env_id_;
    std::string env_type_;
    std::string env_name_;
    int call_id_;
    std::unordered_map<std::string, std::vector<int>> reads_;
    std::unordered_map<std::string, std::vector<int>> writes_;
};

#endif /* LAZR_ENVIRONMENT_H */
