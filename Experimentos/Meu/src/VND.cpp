#include "VND.h"
#include "Descida.h"
#include "DescidaPar.h"
#include "Util.h"
#include "jpa_inv_local.h"
#include <algorithm>
#include <chrono>
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
int VND_par(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
            double wall_start, const JpaContext &ctx) {

  // Split threads: 2 for VND sections, remaining split for inner descent
  int total_threads = omp_get_max_threads();
  int inner_threads = max(1, total_threads / 2);

  int fo = jpa_inv_local(n, s, s_ind, 1, ctx);
  bool improved = true;

  while (improved) {
    improved = false;

    // verifica limite de tempo global (wall-clock) baseado em n ordens
    double wall_now = omp_get_wtime();
    if ((wall_now - wall_start) >= n)
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

// VND (Variable Neighborhood Descent) Sequencial — Fiel ao commit c2c5e39b
// Alterna entre vizinhanças (Swap randomico e 2-opt randomico).
// A ordem das vizinhanças é configurável via parâmetro 'ordem':
//   "swap_2opt"  -> Swap primeiro, 2-opt depois
//   "2opt_swap"  -> 2-opt primeiro, Swap depois
//   "aleatorio"  -> Ordem aleatória a cada chamada
//
// Lógica: começa pela primeira vizinhança. Se melhorar, volta para a
// primeira vizinhança. Se não melhorar, passa para a próxima.
// Quando todas foram tentadas sem melhora, retorna.
int VND_seq(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
            double wall_start, const string &ordem) {
  // define as vizinhanças como funções (lambdas)
  auto vizinhanca_swap = [&]() {
    return descida_randomica_t(n, s, s_ind, IterMax, wall_start);
  };

  auto vizinhanca_2opt = [&]() {
    return descida_2opt_randomica_t(n, s, s_ind, IterMax, wall_start);
  };

  // monta o vetor de vizinhanças na ordem desejada
  vector<function<int()>> vizinhancas;

  if (ordem == "2opt_swap" || ordem == "vnd_2opt_swap") {
    vizinhancas.push_back(vizinhanca_2opt);
    vizinhancas.push_back(vizinhanca_swap);
  } else if (ordem == "aleatorio" || ordem == "vnd_aleatorio") {
    if (rand() % 2 == 0) {
      vizinhancas.push_back(vizinhanca_swap);
      vizinhancas.push_back(vizinhanca_2opt);
    } else {
      vizinhancas.push_back(vizinhanca_2opt);
      vizinhancas.push_back(vizinhanca_swap);
    }
  } else {
    // swap_2opt -> Swap primeiro, 2-opt depois
    vizinhancas.push_back(vizinhanca_swap);
    vizinhancas.push_back(vizinhanca_2opt);
  }

  int fo = jpa_inv(n, s, s_ind, 1);
  int k = 0;

  while (k < (int)vizinhancas.size()) {
    // verifica limite de tempo
    double duracao = omp_get_wtime() - wall_start;
    if (duracao >= n)
      break;

    // chama função da vizinhança
    int fo_novo = vizinhancas[k]();

    if (fo_novo < fo) {
      // melhorou: volta para a primeira vizinhança
      fo = fo_novo;
      k = 0;
    } else {
      // não melhorou: tenta a próxima vizinhança
      k++;
    }
  }

  return fo;
}

DescidaTipo parseDescidaTipo(const string &descida_tipo) {
  if (descida_tipo == "vnd_swap_2opt" || descida_tipo == "swap_2opt")
    return DescidaTipo::VND_SWAP_2OPT;
  if (descida_tipo == "vnd_2opt_swap" || descida_tipo == "2opt_swap")
    return DescidaTipo::VND_2OPT_SWAP;
  if (descida_tipo == "vnd_aleatorio" || descida_tipo == "aleatorio")
    return DescidaTipo::VND_ALEATORIO;
  if (descida_tipo == "swap_randomico")
    return DescidaTipo::SWAP_RANDOMICO;
  if (descida_tipo == "2opt_classico" || descida_tipo == "completa")
    return DescidaTipo::OPT2_CLASSICO;
  if (descida_tipo == "2opt_randomico" || descida_tipo == "randomica")
    return DescidaTipo::OPT2_RANDOMICO;
  if (descida_tipo == "2opt_segmento" || descida_tipo == "segmento_randomica")
    return DescidaTipo::OPT2_SEGMENTO;
  return DescidaTipo::VND_PAR;
}

int busca_local(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
                double wall_start, DescidaTipo tipo, const JpaContext &ctx) {
  switch (tipo) {
  case DescidaTipo::VND_PAR:
    return VND_par(n, s, s_ind, IterMax, wall_start, ctx);
  case DescidaTipo::VND_SWAP_2OPT:
    return VND_seq(n, s, s_ind, IterMax, wall_start, "swap_2opt");
  case DescidaTipo::VND_2OPT_SWAP:
    return VND_seq(n, s, s_ind, IterMax, wall_start, "2opt_swap");
  case DescidaTipo::VND_ALEATORIO:
    return VND_seq(n, s, s_ind, IterMax, wall_start, "aleatorio");
  case DescidaTipo::SWAP_RANDOMICO:
    return descida_randomica_t(n, s, s_ind, IterMax, wall_start);
  case DescidaTipo::OPT2_CLASSICO:
    return descida_2opt(n, s, s_ind, wall_start);
  case DescidaTipo::OPT2_RANDOMICO:
    return descida_2opt_randomica_t(n, s, s_ind, IterMax, wall_start);
  case DescidaTipo::OPT2_SEGMENTO:
    return descida_2opt_segmento_randomica_t(n, s, s_ind, IterMax, wall_start);
  default:
    return VND_par(n, s, s_ind, IterMax, wall_start, ctx);
  }
}

int busca_local(int n, vector<int> &s, vector<int> &s_ind, int IterMax,
                double wall_start, const string &descida_tipo,
                const JpaContext &ctx) {
  return busca_local(n, s, s_ind, IterMax, wall_start,
                     parseDescidaTipo(descida_tipo), ctx);
}
