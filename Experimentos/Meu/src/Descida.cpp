#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <chrono>
#include <utility> //funcao swap (para versao anterior a C11 usar <algorithm>)
#include <omp.h>
#include "Construcao.h"
#include "Descida.h"
#include "Util.h"

using namespace std;

int melhor_vizinho(int n, vector<int> &s, vector<int> &s_ind, int fo, int *melhor_i, int *melhor_j){
    int aux;
    int fo_melhor_viz = fo;
    int fo_viz;

    for(int i=0 ; i < n - 1 ; i++){
        for(int j=i+1 ; j < n ; j++) {

          // Faz o movimento
          aux = s[j];
          s[j] = s[i];
          s[i] = aux;

          //cout<<s[i]<<" "<<s[j]<<endl;

          // Calcular a nova distancia
          //fo_viz = calcula_fo(n, s, s_ind);
          fo_viz = jpa_inv(n, s, s_ind, 1);

          // Armazenar o melhor movimento (melhor troca)
          if(fo_viz < fo_melhor_viz){
            *melhor_i = i;
            *melhor_j = j;
            fo_melhor_viz = fo_viz;
          }

          // Desfaz o movimento
          aux = s[j];
          s[j] = s[i];
          s[i] = aux;

          //cout<<s[i]<<" "<<s[j]<<endl;
        }
    }

    return fo_melhor_viz;

}

int descida(int n, vector<int> &s, vector<int> &s_ind){
    int aux, melhor_i, melhor_j;
    int fo_viz, fo;
    bool melhorou;

    //fo = fo_viz = calcula_fo(n, s, s_ind);
    fo = fo_viz = jpa_inv(n, s, s_ind, 1);

    do{
     melhorou = false;
     fo_viz = melhor_vizinho(n, s, s_ind, fo, &melhor_i, &melhor_j);
     if (fo_viz < fo){
          //printf("Rota antes:  Fo antes = %f \n", fo);
          //imprime_rota(s,n);
          //cout<< "FO era: "<<fo<<" - Agora: "<<fo_viz<<endl;

          aux = s[melhor_j];
          s[melhor_j] = s[melhor_i];
          s[melhor_i] = aux;

          fo = fo_viz;
          melhorou = true;
          //fim_CPU = clock();
          //imprime_fo(Descida, (fim_CPU - inicio_CPU)/CLOCKS_PER_SEC,fo,0);

          //printf("Vou trocar %d com %d \n",melhor_i,melhor_j);
          //printf("Rota depois do movimento: Fo melhor vizinho = %f \n", fo_viz);
          //imprime_rota(s,n);
          //getchar();
        }
    } while (melhorou == true);

    //imprime_vetor(s,n);

    return fo;
}

int vizinho_primeiro_melhora(int n, vector<int> &s, vector<int> &s_ind, int fo, int *melhor_i, int *melhor_j){
    int aux;
    int fo_melhor_viz = fo;
    int fo_viz;
    bool melhorou = false;
    vector<int> vet;

    for (int i=0; i < n; i++) vet.push_back(i);
    //Para c++ 11
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine r(seed);
    shuffle ( vet.begin(), vet.end(), r );

    for(int i=0 ; i < n - 1 ; i++){
        for(int j=i+1 ; j < n ; j++) {

          // Faz o movimento
          aux = s[vet[j]];
          s[vet[j]] = s[vet[i]];
          s[vet[i]] = aux;

          //cout<<s[i]<<" "<<s[j]<<endl;

          // Calcular a nova distancia
          fo_viz = jpa_inv(n, s, s_ind, 1);

          // Armazenar o melhor movimento (melhor troca)
          if(fo_viz < fo_melhor_viz){
            *melhor_i = i;
            *melhor_j = j;
            fo_melhor_viz = fo_viz;
            melhorou=true;
          }

          // Desfaz o movimento
          aux = s[vet[j]];
          s[vet[j]] = s[vet[i]];
          s[vet[i]] = aux;

          //cout<<s[i]<<" "<<s[j]<<endl;
          if(melhorou)break;
        }
        if(melhorou)break;
    }

    return fo_melhor_viz;

}


int descida_primeiro_melhora(int n, vector<int> &s, vector<int> &s_ind){
    int aux, melhor_i, melhor_j;
    int fo_viz, fo;
    bool melhorou;

    fo = fo_viz = jpa_inv(n, s, s_ind, 1);

    do{
     melhorou = false;
     fo_viz = vizinho_primeiro_melhora(n, s, s_ind, fo, &melhor_i, &melhor_j);
     if (fo_viz < fo){
          //printf("Rota antes:  Fo antes = %f \n", fo);
          //imprime_rota(s,n);

          //cout<< "FO era: "<<fo<<" - Agora: "<<fo_viz<<endl;

          aux = s[melhor_j];
          s[melhor_j] = s[melhor_i];
          s[melhor_i] = aux;

          fo = fo_viz;
          melhorou = true;
          //fim_CPU = clock();
          //imprime_fo(Descida, (fim_CPU - inicio_CPU)/CLOCKS_PER_SEC,fo,0);

          //printf("Vou trocar %d com %d \n",melhor_i,melhor_j);
          //printf("Rota depois do movimento: Fo melhor vizinho = %f \n", fo_viz);
          //imprime_rota(s,n);
          //getchar();
        }
    } while (melhorou == true);

    //imprime_vetor(s,n);

    return fo;
}

int descida_randomica(int n, vector<int> &s, vector<int> &s_ind, int IterMax) //intermax = 0,01*n((n-1)/2)
{
  int i, j, iter, aux;
  int fo, fo_viz;


  //fo = calcula_fo(n, s, s_ind);
  fo = jpa_inv(n, s, s_ind, 1);

  iter = 0;
  while (iter < IterMax){
    iter ++;
    j = rand() % (n);
    do{
      i = rand() % (n);
    }while (i == j);


    aux = s[i];
    s[i] = s[j];
    s[j] = aux;


    //fo_viz = calcula_fo(n, s, s_ind);
    fo_viz = jpa_inv(n, s, s_ind, 1);

    if (fo_viz < fo){
      iter = 0;
      fo = fo_viz;
    }
    else{
      aux = s[i];
      s[i] = s[j];
      s[j] = aux;
    }
  }
  return fo;
}

int descida_randomica_viz(int n, vector<int> &s, vector<int> &s_ind, int IterMax, int viz, vector< vector<float> > &atv, float &t_init)
{
  int i, j, iter, aux, tam_viz, low_atv_ref, upper_atv_ref, i_atv, j_atv;
  int fo, fo_viz;
  float t_atual, duracao;
  
  duracao = 0;

  //fo = calcula_fo(n, s, s_ind);
  fo = jpa_inv(n, s, s_ind, 1);

  tam_viz=n*viz/100;
  iter = 0;
  while (iter < IterMax && duracao < n){
    iter ++;
    
    t_atual = clock();
    duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);


    //pertubacoes com vizinhanca limitada
    i_atv=0;
    j=0;
    // selecao aletoria da posicao da atividade referencia para trocar
    i = rand() % n;
    //encontrar indice da atividade referencia na matriz de atividades ordenadas
    while(atv[i_atv][0]!=s[i]){
      i_atv++;
    }
    //criar intervalo de vizinhanca da atv_ref
    if (i_atv<=tam_viz) low_atv_ref = 0;
    else low_atv_ref = i_atv-tam_viz;

    if (i_atv>=(n-1)-tam_viz) upper_atv_ref = n-1;
    else upper_atv_ref = i_atv+tam_viz;

    //gera aleatorio dentro da vizinhanca
    do{
      j_atv = low_atv_ref + rand() % ((upper_atv_ref +1) - low_atv_ref);
    }while (i_atv == j_atv);

    //cout<<i_atv<<" ATV INDEX "<<j_atv<<endl;

    while(s[j]!=atv[j_atv][0]){
      j++;
    }

    aux = s[i];
    s[i] = s[j];
    s[j] = aux;


    //fo_viz = calcula_fo(n, s, s_ind);
    fo_viz = jpa_inv(n, s, s_ind, 1);

    if (fo_viz < fo){
      iter = 0;
      fo = fo_viz;
    }
    else{
      aux = s[i];
      s[i] = s[j];
      s[j] = aux;
    }
  }
  return fo;
}

int descida_randomica_t(int n, vector<int> &s, vector<int> &s_ind, int IterMax, float &t_init)
{
  int i, j, iter, aux;
  int fo, fo_viz;
  float t_atual, duracao;
  
  duracao = 0;

  //fo = calcula_fo(n, s, s_ind);
  fo = jpa_inv(n, s, s_ind, 1);

  iter = 0;
  while (iter < IterMax && duracao < n){
    iter ++;
    
    t_atual = clock();
    duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
    
    j = rand() % (n);
    do{
      i = rand() % (n);
    }while (i == j);

    aux = s[i];
    s[i] = s[j];
    s[j] = aux;


    //fo_viz = calcula_fo(n, s, s_ind);
    fo_viz = jpa_inv(n, s, s_ind, 1);

    if (fo_viz < fo){
      iter = 0;
      fo = fo_viz;
    }
    else{
      aux = s[i];
      s[i] = s[j];
      s[j] = aux;
    }
  }
  return fo;
}

int descida_2opt(int n, vector<int> &s, vector<int> &s_ind) {
    int i, j, iter;
    int fo, fo_viz;
    float t_atual, duracao;
    
    duracao = 0;
    fo = jpa_inv(n, s, s_ind, 1);

    for (int i = 1; i < n - 2; i++) {
      for (int j = i + 1; j < n - 1; j++) {
        int start = min(i, j);
        int end = max(i, j);

        reverse(s.begin() + start, s.begin() + end);
        
        fo_viz = jpa_inv(n, s, s_ind, 1);
        
        if (fo_viz < fo) {
            fo = fo_viz;
            iter = 0;
        } else {
            reverse(s.begin() + start, s.begin() + end);
        }
      }
    }
    return fo;
}
