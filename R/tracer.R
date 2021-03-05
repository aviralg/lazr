
#' @export
#' @importFrom instrumentr trace_code get_exec_stats
profile_laziness <- function(code,
                             environment = parent.frame(),
                             quote = TRUE) {
    tracer <- .Call(C_lazr_tracer_create)

    if(quote) {
        code <- substitute(code)
    }

    trace_code(tracer, code, environment = environment, quote = FALSE)

    state <- .Call(C_lazr_tracer_get_tracing_state, tracer)

    state$exec_stats <- get_exec_stats(tracer)

    state
}
