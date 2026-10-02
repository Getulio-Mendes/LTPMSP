#include "VND.h"
#include "DescidaPar.h"
#include "jpa_inv_local.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <omp.h>
#include <random>
#include <string>
#include <vector>

using namespace std;

// VND (Variable Neighborhood Descent) — Parallel version with OpenMP
//
// Runs both neighborhoods (random swap and random 2-opt) simultaneously
// using OpenMP parallel sections. Each neighborhood operates on a private
// copy of the solution. The best improving result is accepted.
//
// The 'ordem' parameter is kept for API compatibility but both neighborhoods
// always run in parallel every iteration.
//
// Lógica: em cada iteração, ambas as vizinhanças são executadas em paralelo.
// Se alguma melhorar, aceita o melhor resultado e repete.
// Se nenhuma melhorar, retorna.

int VND(int n, vector<int> &s, vector<int> &s_ind, int IterMax, float &t_init,
        const string &ordem, const JpaContext &ctx) {

  double wall_start = omp_get_wtime();

  // Split threads: 2 for VND sections, remaining split for inner descent
  int total_threads = omp_get_max_threads();
  int inner_threads = max(1, total_threads / 2);

  int fo = jpa_inv_local(n, s, s_ind, 1, ctx);
  bool improved = true;

  while (improved) {
    improved = false;

    // verifica limite de tempo (wall-clock)
    double wall_now = omp_get_wtime();
    if ((wall_now - wall_start) >= 3600)
      break;

    // Private copies for each neighborhood
    vector<int> s_swap = s, s_2opt = s;
    vector<int> s_ind_swap = s_ind, s_ind_2opt = s_ind;
    int fo_swap, fo_2opt;

#pragma omp parallel sections
    {
#pragma omp section
      {
        fo_swap = descida_randomica_t_par(n, s_swap, s_ind_swap, IterMax,
                                          wall_start, ctx, inner_threads);
      }

#pragma omp section
      {
        fo_2opt = descida_2opt_randomica_t_par(n, s_2opt, s_ind_2opt, IterMax,
                                               wall_start, ctx, inner_threads);
      }
    }

    // Pick the best result
    int fo_best = min(fo_swap, fo_2opt);
    if (fo_best < fo) {
      fo = fo_best;
      if (fo_swap <= fo_2opt) {
        s = s_swap;
        s_ind = s_ind_swap;
      } else {
        s = s_2opt;
        s_ind = s_ind_2opt;
      }
      improved = true;
    }
  }

  return fo;
}
