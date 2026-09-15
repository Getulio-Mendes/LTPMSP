#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <algorithm>
#include <random>
#include <utility> //funcao swap (para versao anterior a C11 usar <algorithm>)
#include "Construcao.h"
#include "Descida.h"
#include "VND.h"
#include "Util.h"
#include "ILS.h"

using namespace std;


int ILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
          int & num_iteracoes)
{
    vector<int> s_star, s_base;
    int nivel_init, troca_init, iter, MelhorIter, intermax, duracao;
    int fo, fo_2l;
    float t_init, t_atual;

    t_init = clock();

    intermax = n*Itermax/100;

    fo = descida_2opt_segmento_randomica_t(n, s, s_ind,n,t_init);
    s_star = s;
    s_base = s;
    iter = MelhorIter = duracao = 0;
    troca_init = 1;
    while(iter - MelhorIter < ILSmax  && duracao < n){
	//while(duracao < n){
      iter++;
      t_atual = clock();
      duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
      s = s_base;
      nivel_init = 0;
      while(nivel_init<nivel){
        fo_2l = fo;
        int ntrocasMax = troca_init + 1;
        int ntrocas = 0, i, j;
        while (ntrocas < ntrocasMax){
          ntrocas++;
          
          // seleciona as posicoes das atividades para trocar*/
          i = rand() % n;
          do{
            j = rand() % n;
          }while (i == j);
          // Faz o movimento*/
          swap(s[i], s[j]);
				}
		  // fazer descida randomica
      fo_2l = descida_2opt_segmento_randomica_t(n, s, s_ind,n,t_init);
		  if(fo_2l < fo){
			//cout << "FO era: " << fo << " - Agora: " << fo_2l << endl;
			fo = fo_2l;
			s_star = s;
			s_base = s;
			nivel_init = 0;
			troca_init = 1;
			MelhorIter = iter;
			}
		  nivel_init++;
	    }
      troca_init++;
      //cout << "A pertubacao aumentou para: " << troca_init << endl;
    }

    num_iteracoes = iter;

    s = s_star;

	  //fo = calcula_fo(n,s,s_ind);
    fo = jpa_inv(n,s,s_ind,1);

    return fo;
}

int SmartILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
          int & num_iteracoes,
          const string &descida_tipo) // descida_tipo controla a ordem do VND: swap_2opt, 2opt_swap, aleatorio
{
    vector<int> s_star, s_base;
    int falhas_no_nivel, nivel_perturbacao, iter, MelhorIter, intermax, duracao;
    int fo, fo_2l;
    float t_init, t_atual;

    t_init = clock();

    intermax = n*Itermax/100;

    fo = VND(n, s, s_ind, intermax, t_init, descida_tipo);
    s_star = s;
    s_base = s;
    iter = MelhorIter = duracao = 0;
    
    nivel_perturbacao = 1;
    falhas_no_nivel = 0;

    while(iter - MelhorIter < ILSmax && duracao < 3600){
      iter++;
      t_atual = clock();
      duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
      s = s_base;
      
      int ntrocasMax = nivel_perturbacao + 1;
      int ntrocas = 0, i, j;
      while (ntrocas < ntrocasMax){
        ntrocas++;
        // seleciona as posicoes das atividades para trocar*/
        i = rand() % n;
        do{
          j = rand() % n;
        }while (i == j);
        // Faz o movimento*/
        swap(s[i], s[j]);
      }
      
      // fazer busca local VND
      fo_2l = VND(n, s, s_ind, intermax, t_init, descida_tipo);
      
      if(fo_2l < fo){
          //cout << "FO era: " << fo << " - Agora: " << fo_2l << endl;
          fo = fo_2l;
          s_star = s;
          s_base = s;
          nivel_perturbacao = 1;
          falhas_no_nivel = 0;
          MelhorIter = iter;
      } else if (falhas_no_nivel >= nivel) {
          falhas_no_nivel = 1;
          nivel_perturbacao++;
      } else {
          falhas_no_nivel++;
      }
    }

    num_iteracoes = iter;

    s = s_star;

    //fo = calcula_fo(n,s,s_ind);
    fo = jpa_inv(n,s,s_ind,1);

    return fo;
}

int LateAcceptanceILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          vector< vector<float> > &atv,
          int & num_iteracoes,
          const string &descida_tipo) // descida_tipo controla a ordem do VND: swap_2opt, 2opt_swap, aleatorio
{
    vector<int> s_star, s_base, s_best_in_block;
    int nivel_perturbacao, iter, MelhorIter, intermax, duracao;
    int fo, fo_2l, fo_best_in_block;
    float t_init, t_atual;

    t_init = clock();

    intermax = n*Itermax/100;

    fo = VND(n, s, s_ind, intermax, t_init, descida_tipo);
    s_star = s;
    s_base = s;
    iter = MelhorIter = duracao = 0;
    
    nivel_perturbacao = 1;
    
    // Calcula tamanho do bloco para aceitação tardia
    int block_size = max(1, ILSmax / 10);
    int block_iter = 0;
    fo_best_in_block = INT_MAX;

    while(iter - MelhorIter < ILSmax && duracao < 3600){
      iter++;
      t_atual = clock();
      duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
      s = s_base;
      
      int ntrocasMax = nivel_perturbacao + 1;
      int ntrocas = 0, i, j;
      while (ntrocas < ntrocasMax){
        ntrocas++;
        // seleciona as posicoes das atividades para trocar*/
        i = rand() % n;
        do{
          j = rand() % n;
        }while (i == j);
        // Faz o movimento*/
        swap(s[i], s[j]);
      }
      
      // fazer busca local VND
      fo_2l = VND(n, s, s_ind, intermax, t_init, descida_tipo);
      
      if(fo_2l < fo){
          // MELHORA: volta ao comportamento padrão
          fo = fo_2l;
          s_star = s;
          s_base = s;
          nivel_perturbacao = 1;
          MelhorIter = iter;
          block_iter = 0;
          fo_best_in_block = INT_MAX;
      } else {
          // PIORA OU IGUAL: aceitação tardia
          
          // Manter a melhor solução entre as soluções não melhoradas do bloco
          if(block_iter == 0 || fo_2l < fo_best_in_block){
              s_best_in_block = s;
              fo_best_in_block = fo_2l;
          }
          
          // Continuar explorando a partir da solução piorada (late acceptance)
          s_base = s;
          
          // Ao final do bloco: preparar para próxima perturbação
          if(block_iter >= block_size){
              // Usar a melhor solução não melhorada como base
              s_base = s_best_in_block;  
              nivel_perturbacao++;
              block_iter = 0;
              fo_best_in_block = INT_MAX;
          }
          
          block_iter++;
      }
    }

    num_iteracoes = iter;

    s = s_star;

    //fo = calcula_fo(n,s,s_ind);
    fo = jpa_inv(n,s,s_ind,1);

    return fo;
}

int SimILS(int n, vector<int> &s, vector<int> &s_ind,
          int Itermax, //maximo de iteracoes na descida randomica baseado no percentual do n de atvs
          int nivel, // nro de vezes sem melhora em um dado nivel
          int ILSmax,
          float &VarSimHeuristic,
          vector< vector<float> > &atv,
          int & num_iteracoes)
{
    vector<int> s_star, s_base;
    vector< vector<int> > pool; //pool com as melhores solucoes
    int nivel_init, troca_init, iter, MelhorIter, intermax, duracao; //variaveis auxiliares
    int fo, fo_2l, fo_stoch, fo_2l_stoch, pool_index=0, pool_size=10, pool_evaluate=0;
    float t_init, t_atual;

    t_init = clock();

    intermax = n*Itermax/100;  
	fo = descida_randomica_t(n, s, s_ind,intermax, t_init);
    s_star = s;
    s_base = s;
    pool.clear();
    pool.resize(pool_size);
    
    //pega valor estocastico e deterministico de s_star
    fo = jpa_inv(n, s_star, s_ind,1);
    fo_stoch = simulation(n, s_star, s_ind, t_init, VarSimHeuristic, 100, atv);
    iter = MelhorIter = duracao = 0;
    troca_init = 1;
    while(iter - MelhorIter < ILSmax  && duracao < n){
      iter++;
      t_atual = clock();
      duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
      s = s_base;
      nivel_init = 0;
      while(nivel_init<nivel){
        fo_2l = fo;
        int ntrocasMax = troca_init + 1;
        int ntrocas = 0, i, j;
        while (ntrocas < ntrocasMax){
          ntrocas++;          
          // seleciona as posicoes das atividades para trocar*/
          i = rand() % n;
          do{
            j = rand() % n;
          }while (i == j);
          // Faz o movimento*/
          swap(s[i], s[j]);
				}
		  //fazer descida randomica
          fo_2l = descida_randomica_t(n, s, s_ind,intermax,t_init);
		  if(fo_2l < fo){
			    //cout << "FO Best: " << fo << " - FO 2l: " << fo_2l << endl;
			    //pega valor estocastico de s
			    //cout<<"simulation"<<endl;
			    fo_2l_stoch = simulation(n, s, s_ind, t_init, VarSimHeuristic, 100, atv);
			    if(fo_2l_stoch<fo_stoch){				
				      //cout << "FO era: " << fo << " - Agora: " << fo_2l << endl;
				      //cout << "FO_stoch era: " << fo_stoch << " - Agora: " << fo_2l_stoch << endl;
				      fo = fo_2l;
				      fo_stoch = fo_2l_stoch;
				      s_star = s;
					  s_base = s;
				      nivel_init = 0;
				      troca_init = 1;
				      MelhorIter = iter;
              for(auto x:s_star) pool[pool_index%pool_size].push_back(x);
              pool[pool_index%pool_size].push_back(fo);
              pool_index++;
			      }
		    }
		  nivel_init++;
	    }
      troca_init++;
      //cout << "A pertubacao aumentou para: " << troca_init << endl;
    }

    num_iteracoes = iter;

    /*for (auto x:pool){
        //para cada elemento y em x
        for (auto y:x) cout << y << " ";
        cout << endl;
    }*/

    //avaliacao das solucoes do pool com deep simulation
    while(pool_evaluate < pool_size){

      //t_atual = clock();
      //duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
      
      if (pool[pool_evaluate].size() == n+1){
        fo_2l = pool[pool_evaluate].back();
        pool[pool_evaluate].pop_back();
        s = pool[pool_evaluate];
                          
        if(pool_evaluate != pool_index-1 ) fo_2l_stoch = simulation(n, s, s_ind, t_init, VarSimHeuristic, 1000, atv);

        if(fo_2l_stoch<fo_stoch){	
          fo = fo_2l;
          fo_stoch = fo_2l_stoch;
          s_star = s;
        }
      }
      pool_evaluate++;
      //cout<<"Deep-> "<<pool_evaluate<<endl; 
      
    }

    s = s_star;
    //cout<<"OPA_OK"<<endl;
    return fo;
}

int simulation (
		int n, vector<int> &s, vector<int> &s_ind,
		float &t_init,
		float &VarSimHeuristic,
    int sim_iter, 
		vector< vector<float> > &atv){
		
	vector< vector<float> > base_atv; //base com valores originais das atividades
	float t_atual, duracao = 0;
	//int fast_sim_iterations = 100;
	int i = 0;
	int fo_sim = 0; //calculo de FO para uma simulacao
	int fo_sim_sum = 0; //soma de todas as FOs simualadas
	int fo_stoch = 0; //media de todas as FOs simualadas
	
	
	base_atv = atv;
	
	while (i<sim_iter && duracao < n){
		
		t_atual = clock();
		duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
	
		//insere aleatoriedade na duração das atividades
		for (int i=0; i<n; i++){ 
			float var = distribuicao_normal(VarSimHeuristic);
			atv[i][5] = atv[i][5] * ((var/100)+1);
		
		}
		
		fo_sim = jpa_inv(n,s,s_ind,0);
		
		fo_sim_sum = fo_sim_sum + fo_sim;
		
		fo_stoch = fo_sim_sum/(i+1);				
		
		atv = base_atv;	
		
		//cout<<i<< " "<< fo_stoch<<endl;
		
		i++;				
		
  }
    
  return fo_stoch;
	
}

int simulation_new (
		int n, vector<int> &s, vector<int> &s_ind,
		float &t_init,
		float &VarSimHeuristic,
    		int sim_iter, 
		vector< vector<float> > &atv){
		
	vector< vector<float> > base_atv; //base com valores originais das atividades
	float t_atual, duracao = 0;
	//int fast_sim_iterations = 100;
	int i = 0;
	int fo_sim = 0; //calculo de FO para uma simulacao
	int fo_sim_sum = 0; //soma de todas as FOs simualadas
	int fo_stoch = 0; //media de todas as FOs simualadas
	
	
	base_atv = atv;
	
	while (i<sim_iter){
		
		t_atual = clock();
	
		//insere aleatoriedade na duração das atividades
		for (int i=0; i<n; i++){ 
			float var = distribuicao_normal(VarSimHeuristic);
			atv[i][5] = atv[i][5] * ((var/100)+1);
		
		}
		
		fo_sim = jpa_inv(n,s,s_ind,0);
		
		fo_sim_sum = fo_sim_sum + fo_sim;
		
		fo_stoch = fo_sim_sum/(i+1);				
		
		atv = base_atv;	
		
		//cout<<i<< " "<< fo_stoch<<endl;
		
		duracao = ((t_atual-t_init)/CLOCKS_PER_SEC);
		i++;				
		
  }
  
  cout<<endl<<"Duracao em seg: "<<duracao<<endl;  
  return fo_stoch;
	
}
