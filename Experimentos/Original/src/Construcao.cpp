#include <cstdlib>
#include <iostream>
#include <climits>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include "Construcao.h"

void constroi_solucao(int n, vector<int> &s, vector< vector<float> > &atv)
{
    s.clear(); // limpa solucao
    //insere todas atividades sequencialmente
    for (int j=0; j < n; j++)s.push_back(atv[j][0]);

}

void constroi_solucao_aleatoria(int n, vector<int> &s)
{

    s.clear();

    for (int j=1; j < n+1; j++)s.push_back(j);

    //Para c++ 11
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine rand_seed(seed);
    shuffle ( s.begin(), s.end(), rand_seed );

}

void maquinas_unicas(vector<int> &uni_maq, vector< vector<float> > &atv, int natv ){

    uni_maq.clear();
    for(int i=0; i<natv; i++){
        uni_maq.push_back(atv[i][2]);
    }

    sort(uni_maq.begin(), uni_maq.end());
    vector<int>::iterator it;
    it = unique(uni_maq.begin(), uni_maq.end());
    uni_maq.resize(distance(uni_maq.begin(),it));

    /*int tam = uni_maq.size();

    for(int i=0; i<tam; i++){
       cout<<uni_maq[i]<<endl;
    }*/

}

void busca_team(int &valid_t, int &nteam, int &iteam, vector< vector<float> > &t_team, int &iatv, vector< vector<float> > &atv){

    bool stop = false;
    int i = iteam;
    while(i<nteam && stop!=true){
        if(atv[iatv][1]==t_team[i][1] && t_team[i][2]>=atv[i][5]){
            stop=true;
            valid_t=1;
            iteam = i;
            //cout<<"OPA"<<endl;
        }
        i++;
    }

}


void busca_intervalo_maquina(int &valid_m, int &imaq, vector< vector<float> > &t_maq, int &iatv, vector< vector<float> > &atv, int &aux){

    bool stop = false;
    int tam = t_maq[imaq].size();
    //interv_maq.clear();
    while(stop!=true && aux<tam){

    /*verificar as condicoes para agendamento
    o tempo disponivel na maquina deve ser >= tempo pode duracao da atividade
    o tempo inicial do intervalo deve ser <= tempo final - duracao da atividade
    o tempo final do intervalo deve ser >= tempo inicial + duracao da atividade
    o final da atividade <= tempos tempo limite da atividade
    */

    if (t_maq[imaq][1]>=atv[iatv][5] &&
        t_maq[imaq][aux]<0 &&
        t_maq[imaq][aux-1]<=(atv[iatv][4]-atv[iatv][5]) &&
        t_maq[imaq][aux+1]>=(atv[iatv][3]+atv[iatv][5])){
            /*enviar o codigo da maquina
            posicao do intervalo = aux
            minimo do intervalo
            maximo do intervalo

            interv_maq.push_back(t_maq[imaq][0]);
            interv_maq.push_back(aux);
            interv_maq.push_back(t_maq[imaq][aux-1]);
            interv_maq.push_back(t_maq[imaq][aux+1]);
            //cout<<atv[iatv][0]<<" "<<interv_maq[0]<<" "<<interv_maq[1]<<" "<<interv_maq[2]<<" "<<interv_maq[3]<<endl;
            */

            valid_m = 1;
            stop=true;
        }
        aux++;
    }
    //aux--;

}



void busca_intervalo_team(int &valid_tt, int &iteam, vector< vector<float> > &t_team, int &iatv, vector< vector<float> > &atv, int &aux){

    bool stop=false;
    int tam = t_team[iteam].size();
    //interv_team.clear();
    while(stop!=true && aux<tam){

    /*verificar as condicoes para agendamento
    o tempo disponivel na equipe deve ser >= tempo pode duracao da atividade
    o tempo inicial do intervalo deve ser <= tempo final - duracao da atividade
    o tempo final do intervalo deve ser >= tempo inicial + duracao da atividade
    o final da atividade <= tempos tempo limite da atividade
    */

    if (t_team[iteam][2]>=atv[iatv][5] &&
        t_team[iteam][1]==atv[iatv][1] &&
        t_team[iteam][aux]<0 &&
        t_team[iteam][aux-1]<=(atv[iatv][4]-atv[iatv][5]) &&
        t_team[iteam][aux+1]>=(atv[iatv][3]+atv[iatv][5])){
            /*enviar o codigo da e
            posicao do intervalo = aux
            minimo do intervalo
            maximo do intervalo

            interv_team.push_back(t_team[iteam][0]);
            interv_team.push_back(aux);
            interv_team.push_back(t_team[iteam][aux-1]);
            interv_team.push_back(t_team[iteam][aux+1]);
            */

            valid_tt = 1;
            stop=true;
        }
        aux++;

    }


}


void atualiza_tempo(int &i, int &j, vector< vector<float> > &t_tempo, int &iatv, vector< vector<float> > &atv, float interv_init){
	//atualizar o vetor de tempo da maquina e da equipe

    float t_final = interv_init + atv[iatv][5];
    j = j-1;

    //cout<<"inicio: "<<interv_init<<" inicioVet: "<<t_tempo[i][j-1]<<" final: "<<t_final<<" fimVet: "<<t_tempo[i][j+1]<<endl;


    if (interv_init>t_tempo[i][j-1] && t_final<t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j, t_final);
        t_tempo[i].insert(t_tempo[i].begin() + j, interv_init);
        t_tempo[i].insert(t_tempo[i].begin() + j, -1);
    }

    if (interv_init==t_tempo[i][j-1] && t_final<t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j, t_final);
    }

    if (interv_init>t_tempo[i][j-1] && t_final==t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j+1, interv_init);
    }

    if (interv_init==t_tempo[i][j-1] && t_final==t_tempo[i][j+1]){
        t_tempo[i].erase(t_tempo[i].begin() + j);
    }

}

void atualiza_tempo_inv(int &i, int &j, vector< vector<float> > &t_tempo, int &iatv, vector< vector<float> > &atv, float interv){
	//atualizar o vetor de tempo da maquina e da equipe

    float interv_init = interv - atv[iatv][5];
    float t_final = interv;
    j = j-1;

    //cout<<"inicio: "<<interv_init<<" inicioVet: "<<t_tempo[i][j-1]<<" final: "<<t_final<<" fimVet: "<<t_tempo[i][j+1]<<endl;


    if (interv_init>t_tempo[i][j-1] && t_final<t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j, t_final);
        t_tempo[i].insert(t_tempo[i].begin() + j, interv_init);
        t_tempo[i].insert(t_tempo[i].begin() + j, -1);
    }

    if (interv_init==t_tempo[i][j-1] && t_final<t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j, t_final);
    }

    if (interv_init>t_tempo[i][j-1] && t_final==t_tempo[i][j+1]){
        t_tempo[i].insert(t_tempo[i].begin() + j+1, interv_init);
    }

    if (interv_init==t_tempo[i][j-1] && t_final==t_tempo[i][j+1]){
        t_tempo[i].erase(t_tempo[i].begin() + j);
    }

}


/* imprime a solucao */
void imprime_vetor(vector<int> s, int n)
{
    for (int j=0; j < n; j++) cout << "s[" << j << "]=" << s[j] << " ";
    cout<<endl;
}

void permuta(vector<int> &vetor, int inf, int sup, int &j, vector<vector<int>> &data){
	if (inf == sup){
		for(int i=0; i<=sup; i++){
			data[j].push_back(vetor[i]);

		}
		j++;
	}
	else{
		for(int i=inf; i<=sup; i++){
			swap(vetor[inf],vetor[i]);
			permuta(vetor, inf+1, sup, j, data);
			swap(vetor[inf],vetor[i]);
		}
	}


}

void combinacao(vector<int> &vetor, vector<vector<int>> &data){
	int num = vetor.size();
	int decimal = pow(2,num)-1;

	for(int i=0; i<=decimal; i++){
		int j = i;
		vetor.clear();
		while (j>0){
			vetor.push_back(j%2);
			j = j/2;
		}

		vetor.resize(num-1);

		for(int k=vetor.size(); k>=0; k--){
				data[i].push_back(vetor[k]);

		}
		//cout<<endl;

	}

}

