Soft Bayesian Sum of Trees Models
================

The SoftBart package
--------------------

This package implements the methodology described in the paper

-   Linero, A.R. and Yang, Y. (2017). 
    *Soft Bayesian Additive Regression Trees: Ensembles that Adapt to Smoothness and Sparsity.* 
    (In preapration)

### Installation

The package can be installed with the `devtools` package:

``` r
library(devtools)
github_install("theodds/SoftBART")
```

### Usage

The package is designed to mirror the functionality of the `BayesTree` package. The function `softbart` is the primary function, and is used in essentially the same manner as the `bart` function in `BayesTree`.

The following is a minimal example on "Friedman's example":

``` r
## Load library
library(SoftBart)

## Functions used to generate fake data
set.seed(123)
f_fried <- function(x) 10 * sin(pi * x[,1] * x[,2]) + 20 * (x[,3] - 0.5)^2 + 
                      10 * x[,4] + 5 * x[,5]
    
gen_data <- function(n_train, n_test, P, sigma) {
    X <- matrix(runif(n_train * P), nrow = n_train)
    mu <- f_fried(X)
    X_test <- matrix(runif(n_test * P), nrow = n_test)
    mu_test <- f_fried(X_test)
    Y <- mu + sigma * rnorm(n_train)
    Y_test <- mu_test + sigma * rnorm(n_test)
        
    return(list(X = X, Y = Y, mu = mu, X_test = X_test, Y_test = Y_test, mu_test = mu_test))
}

## Simiulate dataset
sim_data <- gen_data(250, 1000, 50, 1)
    
## Fit the model
fit <- softbart(X = sim_data$X, Y = sim_data$Y, X_test = sim_data$X_test, 
                opts = Opts(num_burn = 5000, num_save = 5000))
    
## Plot the fit (note: interval estimates are not prediction intervals, 
## so they do not cover the predictions at the nominal rate)
plot(fit)

## Look at posterior model inclusion probabilities for each predictor. 

posterior_probs <- function(fit) colMeans(fit$var_counts > 0)
plot(posterior_probs(fit), 
     col = ifelse(posterior_probs(fit) > 0.5, muted("blue"), muted("green")), 
     pch = 20)
```
