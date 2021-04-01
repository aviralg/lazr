#ifndef LAZR_BACKTRACE_H
#define LAZR_BACKTRACE_H

#include "utilities.h"
#include <instrumentr/instrumentr.h>
#include <string>
#include <vector>

class Backtrace {
  public:
    Backtrace() {
    }

    void pop() {
        backtrace_.pop_back();
    }

    void push(instrumentr_call_t call) {
        std::string frame;

        int call_id = instrumentr_call_get_id(call);
        instrumentr_value_t function = instrumentr_call_get_function(call);

        frame.append("call(call_id = ");
        frame.append(std::to_string(call_id));
        frame.append(", function = ");

        if (instrumentr_value_is_closure(function)) {
            push_closure_(instrumentr_value_as_closure(function), frame);
        }

        else if (instrumentr_value_is_builtin(function)) {
            push_builtin_(instrumentr_value_as_builtin(function), frame);
        }

        else if (instrumentr_value_is_special(function)) {
            push_special_(instrumentr_value_as_special(function), frame);
        }

        frame.append(")");

        backtrace_.push_back(frame);
    }

    void push(instrumentr_frame_t frame) {
        if (instrumentr_frame_is_promise(frame)) {
            instrumentr_promise_t promise = instrumentr_frame_as_promise(frame);
            push_promise_(promise);
        }

        else if (instrumentr_frame_is_call(frame)) {
            instrumentr_call_t call = instrumentr_frame_as_call(frame);
            push(call);
        }
    }

    std::string to_string() const {
        std::string backtrace;

        for (const std::string& frame: backtrace_) {
            backtrace.append(frame);
            backtrace.push_back('\n');
        }

        return backtrace;
    }

  private:
    void push_closure_(instrumentr_closure_t closure, std::string& frame) {
        int fun_id = instrumentr_closure_get_id(closure);
        const char* name = instrumentr_closure_get_name(closure);
        push_function_("closure", fun_id, name, frame);
    }

    void push_builtin_(instrumentr_builtin_t builtin, std::string& frame) {
        int fun_id = instrumentr_builtin_get_id(builtin);
        const char* name = instrumentr_builtin_get_name(builtin);
        push_function_("builtin", fun_id, name, frame);
    }

    void push_special_(instrumentr_special_t special, std::string& frame) {
        int fun_id = instrumentr_special_get_id(special);
        const char* name = instrumentr_special_get_name(special);
        push_function_("special", fun_id, name, frame);
    }

    void push_function_(const char* type,
                        int fun_id,
                        const char* name,
                        std::string& frame) {
        std::string fun_name = name == NULL ? "<NA>" : name;

        frame.append(type);
        frame.append("(fun_id = ");
        frame.append(std::to_string(fun_id));
        frame.append(", fun_name = ");
        frame.append(fun_name);
        frame.append(")");
    }

    void push_promise_(instrumentr_promise_t promise) {
        std::string frame;
        int id = instrumentr_promise_get_id(promise);
        frame.append("promise(arg_id = ");
        frame.append(std::to_string(id));
        frame.append(")");

        backtrace_.push_back(frame);
    }

    std::vector<std::string> backtrace_;
};

#endif /* LAZR_BACKTRACE_H */
