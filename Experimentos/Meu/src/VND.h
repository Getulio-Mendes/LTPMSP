#ifndef _VND_H_
#define _VND_H_

#include "jpa_inv_local.h"
#include <string>
#include <vector>
using namespace std;

// Ordens pré-definidas para o VND:
//   "swap_2opt"  -> Swap primeiro, 2-opt depois
//   "2opt_swap"  -> 2-opt primeiro, Swap depois
//   "aleatorio"  -> Ordem aleatória a cada chamada
//
// With OpenMP parallelism, both neighborhoods run simultaneously
// (the ordem parameter is kept for API compatibility).
int VND(int n, vector<int> &s, vector<int> &s_ind, int IterMax, double wall_start,
        const string &ordem, const JpaContext &ctx);

#endif // _VND_H_
