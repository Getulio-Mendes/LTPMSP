#include <fstream> // biblioteca para manipulacao de arquivos (streams) (entrada e saida por arquivos)
#include <iostream> // biblioteca para entrada e saida de dados pela tela e teclado
#include <sstream>
#include <limits>
#include <climits>
#include <vector>
#include <string>
#include <algorithm>
#include <time.h>
#include <math.h>
#include <chrono>
#include <random>
#include "Construcao.h"
#include "Util.h"
#include "Descida.h"
#include "ILS.h"
#include "ArgParser.h"


using namespace std; // usa a biblioteca std por padrao

//funcoes

bool sortcol_cres(const vector<float>& v1, const vector<float>& v2 );
bool sortcol_decres(const vector<float>& v1, const vector<float>& v2 );
void OrdenaColuna(int col, int ordena, vector <vector<float>> &matriz );
//int jpa_inv(int n, vector<int> &s, vector<int> &s_ind);


int atrib_atv=7, atrib_team=4; //quantidade de atributos-colunas das atividades e equipes
int nmaq=0, natv=0, nteam=0, qteam=0, qatv=0; //varivaeis auxiliares dos dados de entrada
int fo=0, fo_stoch=0;  // variavel para valor da funcao objetivo
int col_ref=0, n_col=4, fatorial_col=0, melhor_comb=0, melhor_permuta=0; //variaveis para permuta e combina��o de ordena��o de colunas
int contador = 0; //contador de avaliações da função objetivo
int num_exec=30; //numero de execuções do metodo
float max_tempo=0;

vector< vector<float> > atv; //atividades a serem executadas
vector< vector<float> > team; //equipes disponiveis para execucao
vector< vector<float> > t_team; // matriz de tempo das equipes
vector< vector<float> > t_maq; //matriz de tempo das maquinas
vector< vector<float> > s_atv; //matriz de com a solucao encontrada
vector< vector<float> > general; //matriz de solucao geral com todas as execuções
vector<int> uni_maq; //vetor com maquinas unicas
vector< vector<int> > col; //matriz de permutas de colunas
vector<int> permuta_col; //vetor valores iniciais da permuta
vector< vector<int> > o_col; //matriz de orientacoes das colunas
vector<int> comb_col; //vetor valores iniciais da combinacao
vector<int> s; //vetor de solucao
vector<int> s_ind; //vetor de solucao indireta




int main(int argc, char **argv) {

    // if(argc < 4) {
    //     clog << "Usage" << endl << endl;
    //     clog <<"       ./manut input_file best_solution_file NumberVariation%  NumberExecutionILS" << endl << endl;
    //     exit(0);
    // }

    AppArgs args = parseArgs(argc, argv);
    int ItermaxRandom = args.itermax, VizRandom = 30, NivelILS = args.nivel, ILSmax = args.ilsmax;
    float VarSimHeuristic = 0;

    num_exec = args.runs;
    string input_file = args.input_file;
    string algorithm_name = args.algorithm;
    string descida_tipo = args.descida;

    DISABLE_LOGGING = true;



    //sscanf(argv[3], "%f", &VarSimHeuristic);
    //sscanf(argv[4], "%d", &num_exec);
    /*sscanf(argv[7], "%d", &NivelILS);
    sscanf(argv[9], "%d", &ILSmax);*/

    clock_t inicio_CPU,      // clock no inicio da aplicacao do metodo
            fim_CPU;          // clock no final da aplicacao do metodo
    
    float t_init = clock();
    

    /* initialize random seed: */
    srand(args.seed);

    //auxiliares para escolha da solu��o inicial
    vector<int> s_init;
    int fo_init = INT_MAX;

    //iniciar a leitura dos dados
    ifstream arq_entrada;
    //cout << "LEITURA DOS DADOS" << endl;
    arq_entrada.open(input_file);

    arq_entrada >> nmaq;
    arq_entrada >> natv;
    arq_entrada >> nteam;

    //cria matriz de atividades
    atv.resize(natv, vector<float>(atrib_atv));
    //cria matriz de equipes
    team.resize(nteam, vector<float>(atrib_team));

    for (int i=0; i<natv; i++){
        for (int j=0; j<atrib_atv; j++){
            arq_entrada >> atv[i][j];
        }
    }

    for (int i=0; i<nteam; i++){
        for (int j=0; j<atrib_team; j++){
            arq_entrada >> team[i][j];
        }
    }
    arq_entrada.close();
    //finalizar a leitura dos dados
    /*

    //iniciando vetores de solucao
    s.resize(natv,0);

    //Leitura da melhor solucao deterministica
    ifstream arq_solucao;
    arq_solucao.open(best_solution_file);

    for (int i=0; i<natv; i++){
            arq_solucao>> s[i];
    }

    arq_solucao.close();
    //finalizar a leitura dos dados da melhor solucao deterministica

    //for (auto x:s)cout << x << " ";
        
    cout << endl;

    fo = jpa_inv(natv,s,s_ind,1);
    
    cout<<endl<<"Instancia: "<<natv<<"_"<<nteam<<"_"<<nmaq<<endl;

    cout<<endl<<"FO: "<<fo<<endl;
    
    cout<<"Quant Atv Alocadas: "<<qatv<<" de "<<natv<<endl;
    cout<<"Quant Atv Nao Alocadas: "<<natv-qatv<<" de "<<natv<<endl;
    cout<<"Quant Equipes Utilizadas: "<<qteam<<" de "<<nteam<<endl;
    
    fo_stoch = simulation_new(natv, s, s_ind, t_init, VarSimHeuristic, num_exec, atv);
    
    cout<<endl<<"Incerteza %: "<<VarSimHeuristic<<endl;    
    cout<<endl<<"N_deep: "<<num_exec<<endl;  
    cout<<endl<<"FO estocastco: "<<fo_stoch<<endl;
    
    
    if(VarSimHeuristic>0){
        string solution_file = "LTPMSP_"+to_string(natv)+"_"+to_string(nteam)+"_"+to_string(nmaq)+"_stoch_sol_"+to_string((int)VarSimHeuristic)+"_"+to_string(num_exec)+".txt";
        
        ofstream file;
		file.open(solution_file);
		
		file<<endl<<"Instancia: "<<natv<<"_"<<nteam<<"_"<<nmaq<<endl;
		
		file<<best_solution_file<<endl;

		file<<endl<<"FO: "<<fo<<endl;
    
		file<<"Quant Atv Alocadas: "<<qatv<<" de "<<natv<<endl;
		file<<"Quant Atv Nao Alocadas: "<<natv-qatv<<" de "<<natv<<endl;
		file<<"Quant Equipes Utilizadas: "<<qteam<<" de "<<nteam<<endl;
		
		file<<endl<<"Incerteza %: "<<VarSimHeuristic<<endl;    
		file<<endl<<"N_deep: "<<num_exec<<endl;  
		file<<endl<<"FO estocastco: "<<fo_stoch<<endl;
		
		file.close();
        
        cout<<endl<<"ARQUIVO GERADO->"<<endl;  
      };
      
      */



    //Gerar construtivo

    inicio_CPU = clock();
    //ordenacao de matriz de equipes
    //ordenar decrescente pela maior disponibilidade
    col_ref=3;
    sort(team.begin(), team.end(), sortcol_decres);
    //ordenar crescente pelo tipo da equipe
    col_ref=1;
    sort(team.begin(), team.end(), sortcol_cres);

    //limpar matriz de permutas e combinacoes de colunas;
    col.clear();
    permuta_col.clear();
    o_col.clear();
    comb_col.clear();
    fatorial_col=0;

    //redimesionar quant linhas da matriz de permutas e combinacoes
    col.resize(fatorial(n_col));
    o_col.resize(pow(2,n_col));

    //gera vetor com valores iniciais da permuta e combinacao
    for(int i=0; i<n_col; i++){
        permuta_col.push_back(i+3); //as colunas a serem orientada sao as 4 ultimas de 7 -> [3 4 5 6]
        comb_col.push_back(0);
    }

    //permuta_col = {2, 3, 4, 5, 6};

    //gerar todas as permutas e combinacoes
    permuta(permuta_col, 0, permuta_col.size()-1, fatorial_col, col);
    combinacao(comb_col, o_col);

    for(int i=0; i<col.size(); i++){
        for(int j=0; j<o_col.size(); j++){
            for(int k=0; k<n_col; k++){
                int v_col=col[i][k];
                int ov_col = o_col[j][k];
                OrdenaColuna(v_col,ov_col,atv);
            }
            constroi_solucao(natv, s, atv);
            fo = jpa_inv(natv, s, s_ind, 0);
            //fo = calcula_fo(natv, s, s_ind);
            if(fo<fo_init){
                //cout<< "FO era: "<<fo_init<<" - Agora: "<<fo<<endl;
                s_init=s;
                fo_init = fo;
                melhor_comb = j;
                melhor_permuta = i;
            }

        }
    }

    s = s_init;
    fo = fo_init;

    fim_CPU = clock();
    float construcao_time = (float) (fim_CPU - inicio_CPU)/CLOCKS_PER_SEC;

    string solution_file;
    if (!DISABLE_LOGGING) {
        if(VarSimHeuristic>0){
            solution_file = "./results/LTPMSP_"+to_string(natv)+"_"+to_string(nteam)+"_"+to_string(nmaq)+"_SIM_ILS_"+".txt";
        }
        else{
            solution_file = "./results/LTPMSP_"+to_string(natv)+"_"+to_string(nteam)+"_"+to_string(nmaq)+"_ILS_DET_"+".txt";
        }
        ofstream file;
        file.open(solution_file, ios::app);
        file << input_file << endl;
        file <<"Construtivo Tempo CPU (s): " << construcao_time << endl;
        file.close();
    }

    /*
    cout<<endl<<"SOLUCAO INICIAL"<<endl;
    cout<<"Quant Atv Alocadas: "<<qatv<<" de "<<natv<<endl;
    cout<<"Quant Atv Nao Alocadas: "<<natv-qatv<<" de "<<natv<<endl;
    cout<<"Quant Equipes Utilizadas: "<<qteam<<" de "<<nteam<<endl;
    cout<<endl<<"FO: "<<fo<<endl;
    cout<<"Melhor Permuta"<<endl;
    for(int i=0; i<n_col; i++) cout<<col[melhor_permuta][i]<<" ";
    cout<<endl;
    cout<<"Melhor Ordenacao. 0=crescente - 1=decrescente"<<endl;
    for(int i=0; i<n_col; i++) cout<<o_col[melhor_comb][i]<<" ";
    cout<<endl;
    cout<<"Quantidade avaliacoes: "<<contador<<endl;
    printf("Tempo execucao = %10.2f segundos\n",(float)(fim_CPU - inicio_CPU)/CLOCKS_PER_SEC);
    cout<<endl;*/


    //ILS
    if(!DISABLE_LOGGING){
        //imprime dados
        //cout <<"ItermaxRandom: "<<ItermaxRandom << endl; 
        cout <<"NivelILS: "<<NivelILS << endl;
        cout <<"ILSmax: "<<ILSmax << endl;
        cout <<"VarSimHeuristic: "<<VarSimHeuristic << endl;
        cout<<endl<<"METODO ILS"<<endl;
     }

    general.resize(num_exec);

    int num_interacoes = 0;

    for (int i=0; i<num_exec; i++){
      s = s_init;
      fo = fo_init;
        inicio_CPU = clock();
        if(VarSimHeuristic>0){
            fo = SimILS(natv, s, s_ind,ItermaxRandom, NivelILS, ILSmax, VarSimHeuristic, atv, num_interacoes);
        }else{
            if(algorithm_name == "SmartILS"){
                    fo = SmartILS(natv, s, s_ind,ItermaxRandom, NivelILS, ILSmax, atv, num_interacoes, descida_tipo);
            }else{
                    fo = LateAcceptanceILS(natv, s, s_ind,ItermaxRandom, NivelILS, ILSmax, atv, num_interacoes, descida_tipo);
            }
        };
      fim_CPU = clock();
      col_ref=1;
      sort(s_atv.begin(), s_atv.end(), sortcol_decres);   

      //insere dados da execução no vetor de resultados gerais
      general[i].push_back((float)fo);
      general[i].push_back((float)(fim_CPU - inicio_CPU)/CLOCKS_PER_SEC);
      general[i].push_back((float)contador);
      

      if(DISABLE_LOGGING) {
        cout<<endl<<"FO: "<<fo<<endl;
        cout<<"Quant Atv Alocadas: "<<qatv<<" de "<<natv<<endl;
        cout<<"Quant Atv Nao Alocadas: "<<natv-qatv<<" de "<<natv<<endl;
        cout<<"Quant Equipes Utilizadas: "<<qteam<<" de "<<nteam<<endl;
        cout<<"Quantidade avaliacoes: "<<contador<<endl;
 
      }
      cout << fo << endl;
      
      
    //   if(VarSimHeuristic>0){
    //     write(atv, team, s_atv, s, s_ind, fo, qteam,n_col,melhor_permuta,melhor_comb,fo_init,s_init,o_col,col, t+"SIM"+solution_file);
    //   }else{
    //     write(atv, team, s_atv, s, s_ind, fo, qteam,n_col,melhor_permuta,melhor_comb,fo_init,s_init,o_col,col, t+solution_file);
    //   };


        // append a summary line: Nome;fo;tempo;interacao
        // Nome: use the input file name (argv[1] stored in input_file)
        float tempo_exec = (float)(fim_CPU - inicio_CPU)/CLOCKS_PER_SEC;
        append_result_line(solution_file, fo, tempo_exec,num_interacoes, contador);

        // cout<<endl<<"ARQUIVO GERADO->"<<i+1<<endl;

        // reset evaluation counter for next run
        contador = 0; 

    }  

 

    // if(VarSimHeuristic>0){
    //   export_general (general, "GERAL_SIM"+solution_file);
    // }else{
    //   export_general (general, "GERAL_"+solution_file);
    // };
    /*cout<<"Quant Atv Alocadas: "<<qatv<<" de "<<natv<<endl;
    cout<<"Quant Atv Nao Alocadas: "<<natv-qatv<<" de "<<natv<<endl;
    cout<<"Quant Equipes Utilizadas: "<<qteam<<" de "<<nteam<<endl;
    cout<<endl<<"FO: "<<fo<<endl;
    printf("Tempo execucao = %10.2f segundos\n",(float)(fim_CPU - inicio_CPU)/CLOCKS_PER_SEC);*/

    //cout<<fo<<endl; 

}

//funcao jpa invertido com contador de avaliacoes, se cont>0 entao conta avaliacao
int jpa_inv(int n, vector<int> &s, vector<int> &s_ind, int cont){
    int iatv=0, imaq=0, iteam=0;
    int fo=0;
    int valid_t=0, valid_m=0, valid_tt=0;
    int tam_m=0, tam_t=0;
    int busca_m=0, busca_t=0;
    float interv_fim=0,interv_init=0;

    //limpa matriz de tempo das equipes e maquinas, e matriz de solucao
    qteam = 0;
    qatv = 0;
    t_team.clear();
    t_maq.clear();
    s_atv.clear();
    s_atv.resize(natv);
    s_ind.clear();
    s_ind.resize(natv,0);

    //cria vetor de maquinas unicas
    maquinas_unicas(uni_maq, atv, natv);

    //inicializa novas matrizes
    t_team.resize(nteam);
    for (int i=0; i<nteam; i++){
        t_team[i].push_back(team[i][0]); // codigo da equipe
        t_team[i].push_back(team[i][1]); // especialidade da equipe
        t_team[i].push_back(team[i][3]); // quant de unidade de tempo disponivel
        t_team[i].push_back(0);          // quant tempo ja alocada para equipe
        t_team[i].push_back(0);          // tempo inicial dos trabalhos
        t_team[i].push_back(-1);         // indicador de intervalo disponivel
        t_team[i].push_back(team[i][3]); // tempo final dos trabalhos
        if (max_tempo < team[i][3])
            max_tempo = team[i][3];
    }
    t_maq.resize(nmaq);
    for (int i=0; i<nmaq; i++){
        t_maq[i].push_back(uni_maq[i]); //c�digo da maquina
        t_maq[i].push_back(max_tempo);  // quant de unidade de tempo disponivel
        t_maq[i].push_back(0);          // quant tempo ja alocada para m�quina
        t_maq[i].push_back(0);          // tempo inicial dos trabalhos
        t_maq[i].push_back(-1);         // indicador de intervalo disponivel
        t_maq[i].push_back(max_tempo);  // tempo final dos trabalhos
    }
    for (int i=0; i<n; i++){

        //encontrar dados das atividades
        while(atv[iatv][0]!=s[i]){
            iatv++;
        }
        //cout <<s[i]<<" "<<atv[iatv][0]<<endl;

        //procurar maquina correspondente
        while(atv[iatv][2]!=t_maq[imaq][0]){
            imaq++;
        }

        //procurar intervalo de tempo na m�quina selecionada
        tam_m = t_maq[imaq].size();
        while(valid_m!=1 && busca_m<tam_m){
            busca_m = 4;
            busca_intervalo_maquina(valid_m, imaq, t_maq, iatv, atv, busca_m);
            if(valid_m==1){
                //cout<<"Maq OK Intervalo OK"<<endl;

                    //procurar equipe
                    iteam=0;
                    while(valid_t!=1 && iteam<nteam){
                        busca_team(valid_t, nteam, iteam, t_team, iatv, atv);
                        //cout<<i<<" "<<iteam<<" "<<nteam<<" "<<t_team[iteam][0]<<" "<<t_team[iteam][1]<<" "<<atv[iatv][1]<<endl;
                        if(valid_t==1){
                            //cout<<"Equipe OK "<<iteam<<endl;

                            //procurar intervalo da equipe
                            tam_t = t_team[iteam].size();
                            busca_t=5;
                            while(valid_tt!=1 && busca_t<tam_t){
                                busca_intervalo_team(valid_tt, iteam, t_team, iatv, atv, busca_t);
                                //cout<<i<<" "<<busca_t<<" "<<tam_t<<endl;
                                if(valid_tt==1){
                                    //cout<<"Intervalo Equipe OK "<<endl;
                                    //verificar intervalos encontrados
                                    interv_init = max({atv[iatv][3],t_maq[imaq][busca_m-2],t_team[iteam][busca_t-2]});
                                    interv_fim = min({atv[iatv][4],t_maq[imaq][busca_m],t_team[iteam][busca_t]});
                                    if((interv_fim-interv_init)>=atv[iatv][5]){
                                        //atualiza vetor de tempo da m�quina
                                        atualiza_tempo_inv(imaq,busca_m,t_maq,iatv, atv,interv_fim);
                                        //atualiza vetor de tempo da equipe
                                        atualiza_tempo_inv(iteam,busca_t,t_team,iatv,atv,interv_fim);
                                        //atualiza programacao nos vetores de maquina e equipe
                                        t_maq[imaq][1]= t_maq[imaq][1]- atv[iatv][5];
                                        t_maq[imaq][2]= t_maq[imaq][2]+ atv[iatv][5];
                                        t_team[iteam][2] = t_team[iteam][2]- atv[iatv][5];
                                        t_team[iteam][3] = t_team[iteam][3]+ atv[iatv][5];
                                        //atualizar vetor de solu��o indireta
                                        s_ind[i] = 1;
                                        qatv++;
                                        //cout<<i<<" Deu Certo "<<atv[i][0]<<endl;
                                    }else valid_tt=0;

                                }else{
                                    //cout<<"NOK Intervalo Equipe "<<endl;
                                    valid_t=0;
                                    iteam++;
                                }
                            }
                        }else{
                            //cout<<"Equip N Encotrada"<<endl;
                            valid_t=1;
                        }
                    }
            }else{
                //cout<<"Intervalo NOK"<<endl;
                //valid_m=0;
            }
        }
        //atualiza valor de fo em relacao as atividades
        fo = fo + (atv[iatv][6]*(1-s_ind[i]));

        //elaborando matriz de solu��o
        s_atv[i].push_back(s[i]);
        s_atv[i].push_back(s_ind[i]);
        s_atv[i].push_back(atv[iatv][2]);
        if(s_ind[i]==1){
            s_atv[i].push_back(t_team[iteam][0]);
            s_atv[i].push_back((interv_fim-atv[iatv][5]));
            s_atv[i].push_back(interv_fim);
        }
        //zerando controles
        busca_m=0;
        busca_t=0;
        valid_m=0;
        valid_t=0;
        valid_tt=0;
        imaq = 0;
        iatv = 0;
        iteam = 0;
        interv_init=0;
        interv_fim=0;
    }

    //atualiza valor de fo em relacao as equipes
    for(int j=0; j<nteam; j++){
        if (t_team[j][3]>0){
            fo = fo+1;
            qteam = qteam+1;
        }
    }

    if (cont>0) contador++;

    return fo;
    }



//ordenar matriz baseado em determinada coluna crescente
bool sortcol_cres(const vector<float>& v1,
               const vector<float>& v2 ) {
    return v1[col_ref] < v2[col_ref];
    }

//ordenar matriz baseado em determinada coluna decrescente
bool sortcol_decres(const vector<float>& v1,
               const vector<float>& v2 ) {
    return v1[col_ref] > v2[col_ref];
    }

//ordena matriz flex, 0 para crescente e 1 para decrescente
void OrdenaColuna(int col, int ordena, vector< vector<float> > &matriz){
  col_ref = col;
	if(ordena == 0 ) stable_sort(matriz.begin(), matriz.end(), sortcol_cres);
	else stable_sort(matriz.begin(), matriz.end(), sortcol_decres);

}
