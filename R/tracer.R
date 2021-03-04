
#' @importFrom instrumentr trace_packages
create_strictness_tracer <- function(packages) {
    exclusions <- c("base", "injectr", "strictr")

    tracer <- .Call(C_strictr_tracer_create)

    trace_packages(tracer, setdiff(packages, exclusions))

    tracer
}

#' @export
#' @importFrom instrumentr trace_code get_exec_stats
trace_strictness <- function(code,
                             environment = parent.frame(),
                             quote = TRUE,
                             packages = get_installed_packages()) {
    tracer <- create_strictness_tracer(packages)

    if(quote) {
        code <- substitute(code)
    }

    trace_code(tracer, code, environment = environment, quote = FALSE)

    state <- .Call(C_strictr_tracer_get_tracing_state, tracer)

    state$exec_stats <- get_exec_stats(tracer)

    state
}
