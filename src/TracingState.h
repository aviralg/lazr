#ifndef LAZR_TRACING_STATE_H
#define LAZR_TRACING_STATE_H

#include "Rincludes.h"
#include "CallData.h"
#include "ArgumentData.h"
#include <instrumentr/instrumentr.h>

class TracingState {
  public:
    TracingState() {
    }

    CallData& get_call_data() {
        return call_data_;
    }

    const CallData& get_call_data() const {
        return call_data_;
    }

    ArgumentData& get_argument_data() {
        return argument_data_;
    }

    const ArgumentData& get_argument_data() const {
        return argument_data_;
    }

    static void initialize(instrumentr_state_t state);

    static void finalize(instrumentr_state_t state);

    static TracingState& lookup(instrumentr_state_t state);

  private:
    CallData call_data_;
    ArgumentData argument_data_;
};



#endif /* LAZR_TRACING_STATE_H */
