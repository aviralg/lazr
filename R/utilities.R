
#' @export
run_length_encoding <- function(input) {
    .Call(C_lazr_run_length_encoding, input)
}
