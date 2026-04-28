#ifndef _ILS_H_
#define _ILS_H_

#include <vector>
using namespace std;


int ILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax,
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
		  int & num_iteracoes);
          
int SimILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          float &VarSimHeuristic, 
          vector< vector<float> > &atv,
		  int & num_iteracoes);          

int SmartILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
          int & num_iteracoes);

int LateAcceptanceILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
          int & num_iteracoes);
          
int simulation (
		int n, vector<int> &s, vector<int> &s_ind,
		float &t_init,
		float &VarSimHeuristic, 
    		int sim_iter, 
		vector< vector<float> > &atv);
		
int simulation_new (
		int n, vector<int> &s, vector<int> &s_ind,
		float &t_init,
		float &VarSimHeuristic, 
    		int sim_iter, 
		vector< vector<float> > &atv);

#endif // _ILS_H_

