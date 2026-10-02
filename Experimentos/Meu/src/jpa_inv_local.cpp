#include "jpa_inv_local.h"
#include "Construcao.h"
#include <algorithm>
#include <omp.h>
#include <vector>

using namespace std;

// Global evaluation counter (defined in main.cpp)
extern int contador;

// Thread-safe version of jpa_inv.
// All mutable scheduling state is local to this function.
int jpa_inv_local(int n, vector<int> &s, vector<int> &s_ind, int cont,
                  const JpaContext &ctx) {
  int iatv = 0, imaq = 0, iteam = 0;
  int fo = 0;
  int valid_t = 0, valid_m = 0, valid_tt = 0;
  int tam_m = 0, tam_t = 0;
  int busca_m = 0, busca_t = 0;
  float interv_fim = 0, interv_init = 0;

  // Local mutable state (was global in original jpa_inv)
  int qteam_local = 0;
  int qatv_local = 0;
  vector<vector<float>> t_team_local;
  vector<vector<float>> t_maq_local;
  vector<vector<float>> s_atv_local;
  vector<int> uni_maq_local;

  // Local copies of read-only dimensions
  int natv_l = ctx.natv;
  int nteam_l = ctx.nteam;
  int nmaq_l = ctx.nmaq;
  float max_tempo_l = ctx.max_tempo;

  // Clear and resize local matrices
  s_atv_local.resize(natv_l);
  s_ind.clear();
  s_ind.resize(natv_l, 0);

  // Create unique machines vector (local)
  // Need non-const copy of atv for maquinas_unicas (it takes non-const ref)
  vector<vector<float>> atv_copy = ctx.atv;
  maquinas_unicas(uni_maq_local, atv_copy, natv_l);

  // Initialize team time matrix (local)
  t_team_local.resize(nteam_l);
  for (int i = 0; i < nteam_l; i++) {
    t_team_local[i].push_back(ctx.team[i][0]); // codigo da equipe
    t_team_local[i].push_back(ctx.team[i][1]); // especialidade da equipe
    t_team_local[i].push_back(
        ctx.team[i][3]);           // quant de unidade de tempo disponivel
    t_team_local[i].push_back(0);  // quant tempo ja alocada para equipe
    t_team_local[i].push_back(0);  // tempo inicial dos trabalhos
    t_team_local[i].push_back(-1); // indicador de intervalo disponivel
    t_team_local[i].push_back(ctx.team[i][3]); // tempo final dos trabalhos
  }

  // Initialize machine time matrix (local)
  t_maq_local.resize(nmaq_l);
  for (int i = 0; i < nmaq_l; i++) {
    t_maq_local[i].push_back(uni_maq_local[i]); // codigo da maquina
    t_maq_local[i].push_back(
        max_tempo_l);             // quant de unidade de tempo disponivel
    t_maq_local[i].push_back(0);  // quant tempo ja alocada para maquina
    t_maq_local[i].push_back(0);  // tempo inicial dos trabalhos
    t_maq_local[i].push_back(-1); // indicador de intervalo disponivel
    t_maq_local[i].push_back(max_tempo_l); // tempo final dos trabalhos
  }

  for (int i = 0; i < n; i++) {

    // encontrar dados das atividades
    while (atv_copy[iatv][0] != s[i]) {
      iatv++;
    }

    // procurar maquina correspondente
    while (atv_copy[iatv][2] != t_maq_local[imaq][0]) {
      imaq++;
    }

    // procurar intervalo de tempo na maquina selecionada
    tam_m = t_maq_local[imaq].size();
    while (valid_m != 1 && busca_m < tam_m) {
      busca_m = 4;
      busca_intervalo_maquina(valid_m, imaq, t_maq_local, iatv, atv_copy,
                              busca_m);
      if (valid_m == 1) {

        // procurar equipe
        iteam = 0;
        while (valid_t != 1 && iteam < nteam_l) {
          busca_team(valid_t, nteam_l, iteam, t_team_local, iatv, atv_copy);
          if (valid_t == 1) {

            // procurar intervalo da equipe
            tam_t = t_team_local[iteam].size();
            busca_t = 5;
            while (valid_tt != 1 && busca_t < tam_t) {
              busca_intervalo_team(valid_tt, iteam, t_team_local, iatv,
                                   atv_copy, busca_t);
              if (valid_tt == 1) {
                // verificar intervalos encontrados
                interv_init =
                    max({atv_copy[iatv][3], t_maq_local[imaq][busca_m - 2],
                         t_team_local[iteam][busca_t - 2]});
                interv_fim = min({atv_copy[iatv][4], t_maq_local[imaq][busca_m],
                                  t_team_local[iteam][busca_t]});
                if ((interv_fim - interv_init) >= atv_copy[iatv][5]) {
                  // atualiza vetor de tempo da maquina
                  atualiza_tempo_inv(imaq, busca_m, t_maq_local, iatv, atv_copy,
                                     interv_fim);
                  // atualiza vetor de tempo da equipe
                  atualiza_tempo_inv(iteam, busca_t, t_team_local, iatv,
                                     atv_copy, interv_fim);
                  // atualiza programacao nos vetores de maquina e equipe
                  t_maq_local[imaq][1] =
                      t_maq_local[imaq][1] - atv_copy[iatv][5];
                  t_maq_local[imaq][2] =
                      t_maq_local[imaq][2] + atv_copy[iatv][5];
                  t_team_local[iteam][2] =
                      t_team_local[iteam][2] - atv_copy[iatv][5];
                  t_team_local[iteam][3] =
                      t_team_local[iteam][3] + atv_copy[iatv][5];
                  // atualizar vetor de solucao indireta
                  s_ind[i] = 1;
                  qatv_local++;
                } else
                  valid_tt = 0;

              } else {
                valid_t = 0;
                iteam++;
              }
            }
          } else {
            valid_t = 1;
          }
        }
      }
    }
    // atualiza valor de fo em relacao as atividades
    fo = fo + (atv_copy[iatv][6] * (1 - s_ind[i]));

    // elaborando matriz de solucao (local)
    s_atv_local[i].push_back(s[i]);
    s_atv_local[i].push_back(s_ind[i]);
    s_atv_local[i].push_back(atv_copy[iatv][2]);
    if (s_ind[i] == 1) {
      s_atv_local[i].push_back(t_team_local[iteam][0]);
      s_atv_local[i].push_back((interv_fim - atv_copy[iatv][5]));
      s_atv_local[i].push_back(interv_fim);
    }

    // zerando controles
    busca_m = 0;
    busca_t = 0;
    valid_m = 0;
    valid_t = 0;
    valid_tt = 0;
    imaq = 0;
    iatv = 0;
    iteam = 0;
    interv_init = 0;
    interv_fim = 0;
  }

  // atualiza valor de fo em relacao as equipes
  for (int j = 0; j < nteam_l; j++) {
    if (t_team_local[j][3] > 0) {
      fo = fo + 1;
      qteam_local = qteam_local + 1;
    }
  }

  if (cont > 0) {
#pragma omp atomic
    contador++;
  }

  return fo;
}
