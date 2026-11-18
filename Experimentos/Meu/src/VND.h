#ifndef _VND_H_
#define _VND_H_

#include "jpa_inv_local.h"
#include <string>
#include <vector>
using namespace std;

// VND Paralelo Síncrono com OpenMP
int VND_par(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
            double wall_start, const JpaContext &ctx);

// VND Sequencial (fiel ao commit c2c5e39b)
// Ordens pré-definidas para o VND sequencial:
//   "swap_2opt"  -> Swap primeiro, 2-opt depois
//   "2opt_swap"  -> 2-opt primeiro, Swap depois
//   "aleatorio"  -> Ordem aleatória a cada chamada
int VND_seq(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
            double wall_start, const string &ordem = "swap_2opt");

// Tipos de busca local suportados
enum class DescidaTipo {
  VND_PAR,
  VND_SWAP_2OPT,
  VND_2OPT_SWAP,
  VND_ALEATORIO,
  SWAP_RANDOMICO,
  OPT2_CLASSICO,
  OPT2_RANDOMICO,
  OPT2_SEGMENTO
};

DescidaTipo parseDescidaTipo(const string &descida_tipo);

// Dispatcher modular de busca local
int busca_local(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
                double wall_start, DescidaTipo tipo, const JpaContext &ctx);

int busca_local(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
                double wall_start, const string &descida_tipo,
                const JpaContext &ctx);

#endif // _VND_H_
