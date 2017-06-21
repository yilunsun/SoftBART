#ifndef SOFT_BART_H
#define SOFT_BART_H

#include <RcppArmadillo.h>
#include "functions.h"

struct Hypers;
struct Node;

struct Hypers {

  double alpha;
  double beta;
  double gamma;
  double sigma;
  double sigma_mu;
  double shape;
  double width;
  double tau_rate;
  double num_tree_prob;
  int num_tree;
  int num_groups;
  arma::vec s;
  arma::vec logs;
  arma::uvec group;

  arma::vec rho_propose;

  std::vector<std::vector<unsigned int> > group_to_vars;

  double sigma_hat;
  double sigma_mu_hat;
  double alpha_scale;
  double alpha_shape_1;
  double alpha_shape_2;

  void UpdateSigma(const arma::vec& r);
  void UpdateSigmaMu(const arma::vec& means);
  void UpdateAlpha();
  void UpdateGamma(std::vector<Node*>& forest);
  void UpdateBeta(std::vector<Node*>& forest);
  void UpdateTauRate(const std::vector<Node*>& forest);

  // For updating tau
  double loglik_tau(double tau,
                    const std::vector<Node*>& forest,
                    const arma::mat& X, const arma::vec& Y);
  void update_tau(std::vector<Node*>& forest,
                  const arma::mat& X, const arma::vec& Y);

  int SampleVar() const;

};

struct Node {

  bool is_leaf;
  bool is_root;
  Node* left;
  Node* right;
  Node* parent;

  // Branch parameters
  int var;
  double val;
  double lower;
  double upper;
  double tau;

  // Leaf parameters
  double mu;

  // Data for computing weights
  double current_weight;

  // Functions
  void Root(const Hypers& hypers);
  void GetLimits();
  void AddLeaves();
  void BirthLeaves(const Hypers& hypers);
  bool is_left();
  void GenTree(const Hypers& hypers);
  void GenBelow(const Hypers& hypers);
  void GetW(const arma::mat& X, int i);
  void DeleteLeaves();
  void UpdateMu(const arma::vec& Y, const arma::mat& X, const Hypers& hypers);
  void UpdateTau(const arma::vec& Y, const arma::mat& X, const Hypers& hypers);
  void SetTau(double tau_new);
  double loglik_tau(double tau_new, const arma::mat& X, const arma::vec& Y, const Hypers& hypers);

  ~Node();

};



struct Opts {
  int num_burn;
  int num_thin;
  int num_save;
  int num_print;

  bool update_sigma_mu;
  bool update_s;
  bool update_alpha;
  bool update_beta;
  bool update_gamma;
  bool update_tau;
  bool update_tau_mean;
  bool update_num_tree;
};


Opts InitOpts(int num_burn, int num_thin, int num_save, int num_print,
              bool update_sigma_mu, bool update_s, bool update_alpha,
              bool update_beta, bool update_gamma, bool update_tau,
              bool update_tau_mean, bool update_num_tree);


Hypers InitHypers(const arma::mat& X, double sigma_hat, double alpha, double beta,
                  double gamma, double k, double width, double shape,
                  int num_tree, double alpha_scale, double alpha_shape_1,
                  double alpha_shape_2, double tau_rate);

void GetSuffStats(Node* n, const arma::vec& y,
                  const arma::mat& X, const Hypers& hypers,
                  arma::vec& mu_hat_out, arma::mat& Omega_inv_out);

double LogLT(Node* n, const arma::vec& Y,
             const arma::mat& X, const Hypers& hypers);

double cauchy_jacobian(double tau, double sigma_hat);

double update_sigma(const arma::vec& r, double sigma_hat, double sigma_old);

arma::vec predict(const std::vector<Node*>& forest,
                  const arma::mat& X,
                  const Hypers& hypers);

arma::vec predict(Node* node,
                  const arma::mat& X,
                  const Hypers& hypers);

bool is_left(Node* n);

double SplitProb(Node* node, const Hypers& hypers);
int depth(Node* node);
void leaves(Node* x, std::vector<Node*>& leafs);
std::vector<Node*> leaves(Node* x);
arma::vec get_means(std::vector<Node*>& forest);
void get_means(Node* node, std::vector<double>& means);
std::vector<Node*> init_forest(const arma::mat& X, const arma::vec& Y,
                               const Hypers& hypers);

Rcpp::List do_soft_bart(const arma::mat& X,
                        const arma::vec& Y,
                        const arma::mat& X_test,
                        const Hypers& hypers,
                        const Opts& opts);

void IterateGibbsWithS(std::vector<Node*>& forest, arma::vec& Y_hat,
                       Hypers& hypers, const arma::mat& X, const arma::vec& Y,
                       const Opts& opts);
void IterateGibbsNoS(std::vector<Node*>& forest, arma::vec& Y_hat,
                     Hypers& hypers, const arma::mat& X, const arma::vec& Y,
                     const Opts& opts);
void TreeBackfit(std::vector<Node*>& forest, arma::vec& Y_hat,
                 const Hypers& hypers, const arma::mat& X, const arma::vec& Y,
                 const Opts& opts);
double activation(double x, double c, double tau);
void birth_death(Node* tree, const arma::mat& X, const arma::vec& Y,
                 const Hypers& hypers);
void node_birth(Node* tree, const arma::mat& X, const arma::vec& Y,
                const Hypers& hypers);
void node_death(Node* tree, const arma::mat& X, const arma::vec& Y,
                const Hypers& hypers);
void change_decision_rule(Node* tree, const arma::mat& X, const arma::vec& Y,
                          const Hypers& hypers);
double growth_prior(int leaf_depth, const Hypers& hypers);
Node* birth_node(Node* tree, double* leaf_node_probability);
double probability_node_birth(Node* tree);
Node* death_node(Node* tree, double* p_not_grand);
std::vector<Node*> not_grand_branches(Node* tree);
void not_grand_branches(std::vector<Node*>& ngb, Node* node);
arma::uvec get_var_counts(std::vector<Node*>& forest, const Hypers& hypers);
void get_var_counts(arma::uvec& counts, Node* node, const Hypers& hypers);
arma::vec rdirichlet(const arma::vec& shape);
double alpha_to_rho(double alpha, double scale);
double rlgam(double shape);
double rho_to_alpha(double rho, double scale);
double logpdf_beta(double x, double a, double b);
double growth_prior(int node_depth, double gamma, double beta);
double forest_loglik(std::vector<Node*>& forest, double gamma, double beta);
double tree_loglik(Node* node, int node_depth, double gamma, double beta);
Node* rand(std::vector<Node*> ngb);
void UpdateS(std::vector<Node*>& forest, Hypers& hypers);

// For tau
bool do_mh(double loglik_new, double loglik_old,
           double new_to_old, double old_to_new);
double logprior_tau(double tau, double tau_rate);
double tau_proposal(double tau);
double log_tau_trans(double tau_new);
arma::vec get_tau_vec(const std::vector<Node*>& forest);

// RJMCMC for trees
std::vector<Node*> TreeSwap(std::vector<Node*>& forest);
std::vector<Node*> TreeSwapLast(std::vector<Node*>& forest);
std::vector<Node*> AddTree(std::vector<Node*>& forest,
                           const Hypers& hypers);
std::vector<Node*> DeleteTree(std::vector<Node*>& forest);
void update_num_tree(std::vector<Node*>& forest, Hypers& hypers,
                     const arma::vec& Y, const arma::vec& res,
                     const arma::mat& X);
double LogLF(const std::vector<Node*>& forest, const Hypers& hypers,
             const arma::vec& Y, const arma::mat& X);
double loglik_normal(const arma::vec& resid, const double& sigma);
void BirthTree(std::vector<Node*>& forest,
               Hypers& hypers,
               const arma::vec& Y,
               const arma::vec& res,
               const arma::mat& X);
void DeathTree(std::vector<Node*>& forest,
               Hypers& hypers,
               const arma::vec& Y,
               const arma::vec& res,
               const arma::mat& X);
double TPrior(const std::vector<Node*>& forest, const Hypers& hypers);
void RenormAddTree(std::vector<Node*>& forest,
                   std::vector<Node*>& new_forest,
                   Hypers& hypers);
void UnnormAddTree(std::vector<Node*>& forest,
                   std::vector<Node*>& new_forest,
                   Hypers& hypers);
void RenormDeleteTree(std::vector<Node*>& forest,
                      std::vector<Node*>& new_forest,
                      Hypers& hypers);
void UnnormDeleteTree(std::vector<Node*>& forest,
                      std::vector<Node*>& new_forest,
                      Hypers& hypers);

// Slice sampler

struct rho_loglik {
  double mean_log_s;
  double p;
  double alpha_scale;
  double alpha_shape_1;
  double alpha_shape_2;

  double operator() (double rho) {

    double alpha = rho_to_alpha(rho, alpha_scale);

    double loglik = alpha * mean_log_s
      + Rf_lgammafn(alpha)
      - p * Rf_lgammafn(alpha / p)
      + logpdf_beta(rho, alpha_shape_1, alpha_shape_2);

    /* Rcpp::Rcout << "Term 1: " << alpha * mean_log_s << "\n"; */
    /* Rcpp::Rcout << "Term 2:" << Rf_lgammafn(alpha) << "\n"; */
    /* Rcpp::Rcout << "Term 3:" << -p * Rf_lgammafn(alpha / p) << "\n"; */
    /* Rcpp::Rcout << "Term 4:" << logpdf_beta(rho, alpha_shape_1, alpha_shape_2) << "\n"; */

    return loglik;

  }
};

double slice_sampler(double x0, rho_loglik& g, double w,
                     double lower, double upper) {


  /* Find the log density at the initial point, if not already known. */
  double gx0 = g(x0);

  /* Determine the slice level, in log terms. */

  double logy = gx0 - exp_rand();

  /* Find the initial interval to sample from */

  double u = w * unif_rand();
  double L = x0 - u;
  double R = x0 + (w-u);

  /* Expand the interval until its ends are outside the slice, or until the
     limit on steps is reached */

  do {

    if(L <= lower) break;
    if(g(L) <= logy) break;
    L -= w;

  } while(true);

  do {
    if(R >= upper) break;
    if(g(R) <= logy) break;
    R += w;
  } while(true);

  // Shrink interval to lower and upper bounds

  if(L < lower) L = lower;
  if(R > upper) R = upper;

  // Sample from the interval, shrinking it on each rejection

  double x1 = 0.0;

  do {

    double x1 = (R - L) * unif_rand() + L;
    double gx1 = g(x1);

    if(gx1 >= logy) break;

    if(x1 > x0) {
      R = x1;
    }
    else {
      L = x1;
    }

  } while(true);

  return x1;

}


#endif
