
#' @export
#' @importFrom instrumentr trace_code get_exec_stats
trace_expr <- function(code,
                       environment = parent.frame(),
                       quote = TRUE) {
    tracer <- .Call(C_lazr_tracer_create)

    if(quote) {
        code <- substitute(code)
    }

    invisible(trace_code(tracer, code, environment = environment, quote = FALSE))
}

#' @export
trace_file <- function(file, environment = parent.frame()) {
    code <- parse(file = file)

    code <- as.call(c(`{`, code))

    invisible(trace_expr(code, quote = FALSE))
}
