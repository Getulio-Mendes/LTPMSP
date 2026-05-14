#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath>
#include <climits>
#include <ctime>
#include <chrono>
#include <random>
#include <iomanip> 
#include "Util.h"


#define MAX(x,y) ((x)<(y) ? (y) : (x))

using namespace std;

bool DISABLE_LOGGING = true;

//exporta vetor de solucao geral

void export_general (vector< vector<float> > &general, string solution){
	ofstream file;
	file.open(solution);

  	for(auto x: general){
            for(auto y:x) file<<y<<";";
            file << endl;
	}
  
  file.close();

}


// append a single summary line to the solution file
// format: ;fo;tempo;interacao;num_jpa_inv
void append_result_line(const string &solution, int fo, float tempo, int interacao, int contador) {
		if (DISABLE_LOGGING) return;
		
		// open in append mode so existing lines are preserved
		ofstream file;
		file.open(solution, ios::app);
		if (!file.is_open()) return; // silently return if cannot open

		// write the semicolon separated line and a newline
		file  << fo << ";" << fixed << setprecision(2) << tempo << ";" << interacao << ";" << contador << endl;

		file.close();
}



//exporta solucao para txt
void write(vector< vector<float> > &atv, vector< vector<float> > &team, vector< vector<float> > &s_atv, vector<int> &s, vector<int> &s_ind, int &fo, int &qteam, int &n_col, int &melhor_permuta, int &melhor_comb, int &fo_init, vector<int> &s_init, vector< vector<int> > &o_col, vector< vector<int> > &col,  string solution){

	ofstream file;
	file.open(solution);

	file <<"Funcao Objetivo" <<endl;
	file<<fo<<endl;
	file<<"Quant. Equipes"<<endl;
	file<<qteam<<endl;
	
		file <<"Construtivo Funcao Objetivo" <<endl;
	file<<fo_init<<endl;
	file <<"Contrutivo Regra" <<endl;
	
     for(int i=n_col; i>0; i--) file<<col[melhor_permuta][i-1]<<" ";
    file<<endl;

    for(int i=n_col; i>0; i--) file<<o_col[melhor_comb][i-1]<<" ";
    file<<endl;
	
	
	for(auto x: s_atv){
            for(auto y:x) file<<y<<";";
            file << endl;
	}
	file << endl;
	for(auto x: team){
            for(auto y:x) file<<y<<";";
            file << endl;
	}
	file << endl;
	for(auto x: atv){
            for(auto y:x) file<<y<<";";
            file << endl;
	}
    file << endl;
	for(auto x: s){
            file<<x<<";";
	}
    file << endl;
	for(auto x: s_ind){
            file<<x<<";";
	}
	file << endl;

	file <<"Contrutivo Solucao" <<endl;
	for(auto x: s_init){
            file<<x<<";";
	}

	file.close();
}





//imprime vetores de entrada
void imprime_matriz(vector< vector<float> > &data){
    //para cada elemento x em data
    for (auto x:data){
        //para cada elemento y em x
        for (auto y:x) cout << y << " ";
        cout << endl;
    }
}

//imprime vetores de entrada
void imprime_matriz_int(vector< vector<int> > &data){
    //para cada elemento x em data
    for (auto x:data){
        //para cada elemento y em x
        for (auto y:x) cout << y << " ";
        cout << endl;
    }
}


int fatorial(int n) {
	int fator = 1;
	for(int i=1; i<=n; i++){
		fator = fator*i;
	}
	return fator;
}

float distribuicao_normal(float desvio) {
	float valor, variacao;
	
	variacao = desvio/3;//99,73% de certeza na distriuição normal temos 3*Sigma+/-desvio
	
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator (seed);

	std::normal_distribution<double> distribution (0.0,variacao);
	
	valor = distribution(generator);
	
	return valor;
	
}

