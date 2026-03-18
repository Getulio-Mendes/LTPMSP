#ifndef _CONSTRUCAO_H_
#define _CONSTRUCAO_H_
#include <vector>
using namespace std;

void constroi_solucao(int n, vector<int> &s, vector< vector<float> > &atv);

void constroi_solucao_aleatoria(int n, vector<int> &s);

void maquinas_unicas(vector<int> &uni_maq, vector< vector<float> > &atv, int natv);

void busca_team(int &valid_t, int &nteam, int &iteam, vector< vector<float> > &t_team, int &iatv, vector< vector<float> > &atv);

void busca_intervalo_maquina(int &valid_m, int &imaq, vector< vector<float> > &t_maq, int &iatv, vector< vector<float> > &atv, int &aux);

void busca_intervalo_team(int &valid_tt, int &iteam, vector< vector<float> > &t_team, int &iatv, vector< vector<float> > &atv, int &aux);

void atualiza_tempo(int &i, int &j, vector< vector<float> > &t_tempo, int &iatv, vector< vector<float> > &atv, float interv_init);

void atualiza_tempo_inv(int &i, int &j, vector< vector<float> > &t_tempo, int &iatv, vector< vector<float> > &atv, float interv);

void imprime_vetor(vector<int> s, int n);

void permuta(vector<int> &vetor, int inf, int sup, int &j, vector<vector<int>> &data);

void combinacao(vector<int> &vetor, vector<vector<int>> &data);


#endif
