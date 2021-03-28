#ifndef LAZR_TRACING_STATE_H
#define LAZR_TRACING_STATE_H

#include "Rincludes.h"
#include "CallTable.h"
#include "EnvironmentTable.h"
#include "ArgumentTable.h"
#include "FunctionTable.h"
#include "EffectsTable.h"
#include "ArgumentReflectionTable.h"
#include "CallReflectionTable.h"
#include <instrumentr/instrumentr.h>

class TracingState {
  public:
    TracingState() {
    }

    CallTable& get_call_table() {
        return call_table_;
    }

    const CallTable& get_call_table() const {
        return call_table_;
    }

    EnvironmentTable& get_environment_table() {
        return environment_table_;
    }

    const EnvironmentTable& get_environment_table() const {
        return environment_table_;
    }

    ArgumentTable& get_argument_table() {
        return argument_table_;
    }

    const ArgumentTable& get_argument_table() const {
        return argument_table_;
    }

    FunctionTable& get_function_table() {
        return function_table_;
    }

    const FunctionTable& get_function_table() const {
        return function_table_;
    }

    EffectsTable& get_effects_table() {
        return effects_table_;
    }

    const EffectsTable& get_effects_table() const {
        return effects_table_;
    }

    ArgumentReflectionTable& get_arg_ref_tab() {
        return arg_ref_tab_;
    }

    const ArgumentReflectionTable& get_arg_ref_tab() const {
        return arg_ref_tab_;
    }

    CallReflectionTable& get_call_ref_tab() {
        return call_ref_tab_;
    }

    const CallReflectionTable& get_call_ref_tab() const {
        return call_ref_tab_;
    }

    static void initialize(instrumentr_state_t state);

    static void finalize(instrumentr_state_t state);

    static TracingState& lookup(instrumentr_state_t state);

  private:
    CallTable call_table_;
    EnvironmentTable environment_table_;
    ArgumentTable argument_table_;
    FunctionTable function_table_;
    EffectsTable effects_table_;
    ArgumentReflectionTable arg_ref_tab_;
    CallReflectionTable call_ref_tab_;
};

#endif /* LAZR_TRACING_STATE_H */
