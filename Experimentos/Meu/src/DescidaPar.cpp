#include "DescidaPar.h"
#include "jpa_inv_local.h"
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <omp.h>
#include <vector>

using namespace std;

// Parallel random swap descent.
// Each iteration, num_threads threads generate independent random swaps on
// private copies of the solution and evaluate them concurrently.
// The master thread picks the best improving move.
int descida_randomica_t_par(int n, vector<int> &s, vector<int> &s_ind,
                            int IterMax, double wall_start,
                            const JpaContext &ctx, int num_threads) {
  int fo = jpa_inv_local(n, s, s_ind, 1, ctx);
  int iter = 0;
  double duracao = omp_get_wtime() - wall_start;

  while (iter < IterMax && duracao < n) {
    iter++;
    duracao = omp_get_wtime() - wall_start;

    // Each thread produces a candidate solution
    vector<vector<int>> s_candidates(num_threads);
    vector<vector<int>> s_ind_candidates(num_threads);
    vector<int> fo_candidates(num_threads, INT_MAX);

#pragma omp parallel num_threads(num_threads)
    {
      int tid = omp_get_thread_num();
      unsigned int seed = (unsigned int)(rand()) + tid * 1000 + iter;

      // Private copy
      vector<int> s_local = s;
      vector<int> s_ind_local = s_ind;

      // Random swap
      int j = rand_r(&seed) % n;
      int i;
      do {
        i = rand_r(&seed) % n;
      } while (i == j);

      int aux = s_local[i];
      s_local[i] = s_local[j];
      s_local[j] = aux;

      // Evaluate
      int fo_local = jpa_inv_local(n, s_local, s_ind_local, 1, ctx);

      s_candidates[tid] = s_local;
      s_ind_candidates[tid] = s_ind_local;
      fo_candidates[tid] = fo_local;
    }

    // Find the best candidate across all threads
    int best_tid = 0;
    for (int t = 1; t < num_threads; t++) {
      if (fo_candidates[t] < fo_candidates[best_tid]) {
        best_tid = t;
      }
    }

    if (fo_candidates[best_tid] < fo) {
      iter = 0;
      fo = fo_candidates[best_tid];
      s = s_candidates[best_tid];
      s_ind = s_ind_candidates[best_tid];
    }
  }

  return fo;
}

// Parallel random 2-opt descent.
// Each iteration, num_threads threads generate independent random 2-opt
// reversals on private copies and evaluate concurrently.
int descida_2opt_randomica_t_par(int n, vector<int> &s, vector<int> &s_ind,
                                 int IterMax, double wall_start,
                                 const JpaContext &ctx, int num_threads) {
  int fo = jpa_inv_local(n, s, s_ind, 1, ctx);
  int iter = 0;
  double duracao = omp_get_wtime() - wall_start;

  while (iter < IterMax && duracao < n) {
    iter++;
    duracao = omp_get_wtime() - wall_start;

    vector<vector<int>> s_candidates(num_threads);
    vector<vector<int>> s_ind_candidates(num_threads);
    vector<int> fo_candidates(num_threads, INT_MAX);

#pragma omp parallel num_threads(num_threads)
    {
      int tid = omp_get_thread_num();
      unsigned int seed = (unsigned int)(rand()) + tid * 1000 + iter;

      // Private copy
      vector<int> s_local = s;
      vector<int> s_ind_local = s_ind;

      // Random 2-opt: i in [1, n-3], j in [i+1, n-2]
      int i_2opt, j_2opt;
      if (n > 3) {
        i_2opt = 1 + rand_r(&seed) % (n - 3);
        j_2opt = i_2opt + 1 + rand_r(&seed) % (n - i_2opt - 2);
      } else {
        // Fallback for very small n
        i_2opt = 0;
        j_2opt = n - 1;
      }

      reverse(s_local.begin() + i_2opt, s_local.begin() + j_2opt + 1);

      // Evaluate
      int fo_local = jpa_inv_local(n, s_local, s_ind_local, 1, ctx);

      s_candidates[tid] = s_local;
      s_ind_candidates[tid] = s_ind_local;
      fo_candidates[tid] = fo_local;
    }

    // Find the best candidate across all threads
    int best_tid = 0;
    for (int t = 1; t < num_threads; t++) {
      if (fo_candidates[t] < fo_candidates[best_tid]) {
        best_tid = t;
      }
    }

    if (fo_candidates[best_tid] < fo) {
      iter = 0;
      fo = fo_candidates[best_tid];
      s = s_candidates[best_tid];
      s_ind = s_ind_candidates[best_tid];
    }
  }

  return fo;
}
