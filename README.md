
# lazr

<!-- badges: start -->
<!-- badges: end -->

The goal of lazr is to analyze function strictness.

## Installation

You can install the released version of lazr from [CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("lazr")
```

## Example

This is a basic example which shows you how to solve a common problem:

``` r
library(lazr)

trace_strictness({
    library(stringr)
    str_c("a")
})
```
