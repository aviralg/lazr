
# strictr

<!-- badges: start -->
<!-- badges: end -->

The goal of strictr is to analyze function strictness.

## Installation

You can install the released version of strictr from [CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("strictr")
```

## Example

This is a basic example which shows you how to solve a common problem:

``` r
library(strictr)

trace_strictness({
    library(stringr)
    str_c("a")
})
```
