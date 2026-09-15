#ifndef _VND_H_
#define _VND_H_

#include <vector>
#include <string>
using namespace std;

// Ordens pré-definidas para o VND:
//   "swap_2opt"  -> Swap primeiro, 2-opt depois
//   "2opt_swap"  -> 2-opt primeiro, Swap depois
//   "aleatorio"  -> Ordem aleatória a cada chamada
int VND(int n, vector<int> &s, vector<int> &s_ind,
        int IterMax, float &t_init,
        const string &ordem = "swap_2opt");

#endif // _VND_H_
