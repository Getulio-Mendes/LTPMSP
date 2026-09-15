#include "VND.h"
#include "Descida.h"
#include "Util.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <random>
#include <string>
#include <vector>

using namespace std;

// VND (Variable Neighborhood Descent)
// Alterna entre vizinhanças (Swap randomico e 2-opt randomico).
// A ordem das vizinhanças é configurável via parâmetro 'ordem':
//   "swap_2opt"  -> Swap primeiro, 2-opt depois
//   "2opt_swap"  -> 2-opt primeiro, Swap depois
//   "aleatorio"  -> Ordem aleatória a cada chamada
//
// Lógica: começa pela primeira vizinhança. Se melhorar, volta para a
// primeira vizinhança. Se não melhorar, passa para a próxima.
// Quando todas foram tentadas sem melhora, retorna.

int VND(int n, vector<int> &s, vector<int> &s_ind, int IterMax, float &t_init,
        const string &ordem) {
  // define as vizinhanças como funções (lambdas)
  auto vizinhanca_swap = [&]() {
    return descida_randomica_t(n, s, s_ind, IterMax, t_init);
  };

  auto vizinhanca_2opt = [&]() {
    return descida_2opt_randomica_t(n, s, s_ind, IterMax, t_init);
  };

  // monta o vetor de vizinhanças na ordem desejada
  vector<function<int()>> vizinhancas;

  if (ordem == "2opt_swap") {
    vizinhancas.push_back(vizinhanca_2opt);
    vizinhancas.push_back(vizinhanca_swap);
  } else if (ordem == "aleatorio") {
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
    float duracao = ((clock() - t_init) / CLOCKS_PER_SEC);
    if (duracao >= 3600)
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
