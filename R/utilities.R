
hash_table <- function(...) {
    object <- list2env(list(...), parent = emptyenv())
    structure(object, class = c("hash_table", class(object)))
}

#' @importFrom utils installed.packages
get_installed_packages <- function() {
    unname(installed.packages()[, 1])
}


to_data_frame <- function(object) {
    UseMethod("to_data_frame")
}

to_data_frame.hash_table <- function(object) {
    do.call(rbind, as.list(object))
}
