#ifndef _DESCIDA_H_
#define _DESCIDA_H_

#include <vector>
using namespace std;

int melhor_vizinho(int n, vector<int> &s, vector<int> &s_ind, int fo, int *melhor_i, int *melhor_j);

int descida(int n, vector<int> &s, vector<int> &s_ind);

int vizinho_primeiro_melhora(int n, vector<int> &s, vector<int> &s_ind, int fo, int *melhor_i, int *melhor_j);

int descida_primeiro_melhora(int n, vector<int> &s, vector<int> &s_ind);

int descida_randomica(int n, vector<int> &s, vector<int> &s_ind, int IterMax);

int descida_randomica_viz(int n, vector<int> &s, vector<int> &s_ind, int IterMax, int viz, vector< vector<float> > &atv, float &t_init);

int descida_randomica_t(int n, vector<int> &s, vector<int> &s_ind, int IterMax, float &t_init);

int descida_2opt(int n, vector<int> &s, vector<int> &s_ind);

int descida_2opt_randomica_t(int n, vector<int> &s, vector<int> &s_ind, int IterMax, float &t_init);

#endif // _DESCIDA_H_
