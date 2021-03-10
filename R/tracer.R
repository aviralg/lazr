
#' @export
#' @importFrom instrumentr trace_code get_exec_stats
profile_expr <- function(code,
                         environment = parent.frame(),
                         quote = TRUE) {
    tracer <- .Call(C_lazr_tracer_create)

    if(quote) {
        code <- substitute(code)
    }

    result <- trace_code(tracer, code, environment = environment, quote = FALSE)

    result$state$exec_stats <- get_exec_stats(tracer)

    result
}

#' @export
profile_file <- function(file, environment = parent.frame()) {
    code <- parse(file = file)

    code <- as.call(c(`{`, code))

    profile_expr(code, quote = FALSE)
}
