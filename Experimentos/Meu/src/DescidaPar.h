#ifndef _DESCIDA_PAR_H_
#define _DESCIDA_PAR_H_

#include "jpa_inv_local.h"
#include <vector>
using namespace std;

// Parallel version of descida_randomica_t.
// Multiple random swaps are evaluated in parallel per iteration.
// Uses omp_get_wtime() for time measurement and rand_r() for thread-safe RNG.
// num_threads: how many inner threads to use (controlled by VND to avoid
// oversubscription).
int descida_randomica_t_par(int n, vector<int> &s, vector<int> &s_ind,
                            int IterMax, double wall_start,
                            const JpaContext &ctx, int num_threads);

// Parallel version of descida_2opt_randomica_t.
// Multiple random 2-opt reversals are evaluated in parallel per iteration.
int descida_2opt_randomica_t_par(int n, vector<int> &s, vector<int> &s_ind,
                                 int IterMax, double wall_start,
                                 const JpaContext &ctx, int num_threads);

#endif // _DESCIDA_PAR_H_
