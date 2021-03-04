#ifndef STRICTR_TRACING_STATE_H
#define STRICTR_TRACING_STATE_H

#include "Rincludes.h"
#include "CallData.h"
#include "ArgumentData.h"

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

    SEXP to_sexp() {
        SEXP r_data = PROTECT(allocVector(VECSXP, 2));
        SET_VECTOR_ELT(r_data, 0, call_data_.to_sexp());
        SET_VECTOR_ELT(r_data, 1, argument_data_.to_sexp());


        SEXP r_names = PROTECT(allocVector(STRSXP, 2));
        SET_STRING_ELT(r_names, 0, mkChar("calls"));
        SET_STRING_ELT(r_names, 1, mkChar("arguments"));

        setAttrib(r_data, R_NamesSymbol, r_names);

        UNPROTECT(2);
        return r_data;
    }


  private:
    CallData call_data_;
    ArgumentData argument_data_;
};

SEXP wrap_tracing_state(TracingState* tracing_state);

TracingState* unwrap_tracing_state(SEXP r_tracing_state);

#endif /* STRICTR_TRACING_STATE_H */
