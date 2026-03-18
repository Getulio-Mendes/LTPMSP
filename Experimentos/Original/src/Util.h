#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <iomanip> 
using namespace std;

//exporta vetor de solucao geral
void export_general (vector< vector<float> > &general, string solution);

//exporta solucao para txt
void write(vector< vector<float> > &atv, vector< vector<float> > &team, vector< vector<float> > &s_atv, vector<int> &s, vector<int> &s_ind, int &fo, int &qteam, int &n_col, int &melhor_permuta, int &melhor_comb, int &fo_init, vector<int> &s_init, vector< vector<int> > &o_col, vector< vector<int> > &col,  string solution);

//append a single summary line to an existing solution file in the format:
// Nome;fo;tempo;interação  (appends one new line each call, preserves existing content)
void append_result_line(const string &solution, int fo, float tempo, int interacao, int contador);


//imprime vetores de entrada
void imprime_matriz(vector< vector<float> > &data);

void imprime_matriz_int(vector< vector<int> > &data);

int calcula_fo(int n, vector<int> &s, vector<int> &s_ind);

int jpa_inv(int n, vector<int> &s, vector<int> &s_ind, int cont);

int fatorial(int n);

float distribuicao_normal(float desvio);

#endif

