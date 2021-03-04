
#' @importFrom instrumentr get_data
get_tracing_state <- function(tracer) {
    .Call(C_strictr_tracer_get_tracing_state, tracer)
}

#' @export
#' @importFrom purrr imap_chr
#' @importFrom fst write_fst
write_tracing_data <- function(data, dirpath = getwd()) {
    writer <- function(df, filename) {
        filepath <- file.path(dirpath, paste0(filename, ".fst"))
        write_fst(df, filepath)
        filepath
    }
    imap_chr(data, writer)
}
