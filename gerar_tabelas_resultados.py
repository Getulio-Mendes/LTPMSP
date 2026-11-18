#!/usr/bin/env python3
"""Gera, em CSV, os dados das tabelas do capítulo de Resultados.

Entradas:
  Experimentos/Original/results/*.txt  ILS original (usa as 10 primeiras execuções)
  Experimentos/Meu/results/*.txt       Abordagem proposta, ID 41 (só o nível superior da pasta)
  ../PFC1/Table-10.csv                 Tabela 10 de Coelho, Souza e Cota (literatura)

Saídas (pasta tabelas_resultados/):
  qualidade.csv    -> tab:resultados_qualidade
  tempo.csv        -> tab:resultados_tempo
  literatura.csv   -> tab:literatura
  sintese_porte.csv -> tab:sintese_porte

Uso: python3 gerar_tabelas_resultados.py
"""
import csv
import statistics
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
DIR_ORIG = RAIZ / 'Experimentos' / 'Original' / 'results'
DIR_ATUAL = RAIZ / 'Experimentos' / 'Meu' / 'results'
TABELA_10 = RAIZ.parent / 'PFC1' / 'Table-10.csv'
SAIDA = RAIZ / 'tabelas_resultados'

EXECUCOES = 10
EXCLUIDAS = {'LTPMSP_02_04800_00078_01286t', 'LTPMSP_06_33484_00145_01032'}
PORTES = {'Médio': (150, 600), 'Grande': (1200, 4800), 'Extra Grande': (9600, 19200)}


def ler_resultados(pasta):
    """{instância: (lista de FO, lista de tempos)} com as primeiras EXECUCOES linhas.
    Formato do arquivo: linha com o caminho da instância, seguida de linhas 'fo;tempo;...'."""
    dados = {}
    for arquivo in pasta.glob('*.txt'):
        instancia = None
        for linha in arquivo.read_text().splitlines():
            linha = linha.strip()
            if linha.endswith('.txt'):
                instancia = Path(linha).stem
                dados.setdefault(instancia, ([], []))
            elif instancia and ';' in linha:
                fo, tempo = linha.split(';')[:2]
                dados[instancia][0].append(float(fo))
                dados[instancia][1].append(float(tempo))
    return {i: (fo[:EXECUCOES], t[:EXECUCOES]) for i, (fo, t) in dados.items()}


def ler_literatura():
    """{(N, M, Q): linha da Tabela 10}. Colunas: BKV=4, ILS Best/Avg=8/9, GRASP Best/Avg=20/21."""
    lit = {}
    with open(TABELA_10) as f:
        for linha in csv.reader(f):
            if linha and linha[0].isdigit():
                n, m, q = (int(x.strip('*')) for x in linha[1:4])
                lit[(n, m, q)] = {'BKV': float(linha[4]),
                                  'ILS_melhor': float(linha[8]), 'ILS_medio': float(linha[9]),
                                  'GRASP_melhor': float(linha[20]), 'GRASP_medio': float(linha[21])}
    return lit


def numero_ordens(instancia):
    # nome: LTPMSP_GG_NNNNN_MMMMM_QQQQQ
    return int(instancia.split('_')[2])


def salvar(nome, linhas):
    # números reais gravados com 4 casas decimais
    linhas = [{c: round(v, 4) if isinstance(v, float) else v for c, v in l.items()} for l in linhas]
    with open(SAIDA / nome, 'w', newline='') as f:
        escritor = csv.DictWriter(f, fieldnames=linhas[0].keys())
        escritor.writeheader()
        escritor.writerows(linhas)
    print(f'gerado: {SAIDA / nome}')


def media_coluna(linhas, coluna):
    return statistics.mean(l[coluna] for l in linhas)


def main():
    orig = ler_resultados(DIR_ORIG)
    atual = ler_resultados(DIR_ATUAL)
    lit = ler_literatura()
    instancias = sorted(i for i in orig if i in atual and i not in EXCLUIDAS)
    SAIDA.mkdir(exist_ok=True)

    qualidade, tempo, literatura = [], [], []
    for inst in instancias:
        fo_o, t_o = orig[inst]
        fo_a, t_a = atual[inst]

        # ---------- Tabela de qualidade (mesmo hardware) ----------
        q = {
            'Instancia': inst,
            'N': numero_ordens(inst),
            'Orig_FO_medio': statistics.mean(fo_o),
            'Orig_melhor':   min(fo_o),
            'Orig_pior':     max(fo_o),
            'Orig_desvio':   statistics.stdev(fo_o),   # desvio padrão amostral
            'Atual_FO_medio': statistics.mean(fo_a),
            'Atual_melhor':   min(fo_a),
            'Atual_pior':     max(fo_a),
            'Atual_desvio':   statistics.stdev(fo_a),
        }
        # Gap (%) = (FO_medio_Orig - FO_medio_Atual) / FO_medio_Orig * 100
        q['Gap_%'] = (q['Orig_FO_medio'] - q['Atual_FO_medio']) / q['Orig_FO_medio'] * 100
        qualidade.append(q)

        # ---------- Tabela de tempo ----------
        t = {
            'Instancia': inst,
            'N': numero_ordens(inst),
            'Orig_tempo_medio':  statistics.mean(t_o),
            'Orig_tempo_desvio': statistics.stdev(t_o),
            'Atual_tempo_medio':  statistics.mean(t_a),
            'Atual_tempo_desvio': statistics.stdev(t_a),
        }
        # Speedup = tempo_medio_Orig / tempo_medio_Atual
        # (com os tempos arredondados a 2 casas, como exibidos na tabela de tempo)
        t['Speedup'] = round(t['Orig_tempo_medio'], 2) / round(t['Atual_tempo_medio'], 2)
        tempo.append(t)

        # ---------- Tabela da literatura ----------
        L = lit[tuple(int(x) for x in inst.split('_')[2:5])]
        # FO_BKV = min(BKV da literatura, melhor solução da abordagem proposta)
        fo_bkv = min(L['BKV'], q['Atual_melhor'])
        # RPD (%) = (FO_medio - FO_BKV) / FO_BKV * 100
        rpd = lambda fo_medio: (fo_medio - fo_bkv) / fo_bkv * 100
        literatura.append({
            'Instancia': inst,
            'N': numero_ordens(inst),
            'BKV_Lit': L['BKV'],
            'FO_BKV': fo_bkv,
            'GRASP_melhor': L['GRASP_melhor'], 'GRASP_FO_medio': L['GRASP_medio'], 'GRASP_RPD_%': rpd(L['GRASP_medio']),
            'ILS_melhor':   L['ILS_melhor'],   'ILS_FO_medio':   L['ILS_medio'],   'ILS_RPD_%':   rpd(L['ILS_medio']),
            'Atual_melhor': q['Atual_melhor'], 'Atual_FO_medio': q['Atual_FO_medio'], 'Atual_RPD_%': rpd(q['Atual_FO_medio']),
        })

    # Linha "Média Geral" = média aritmética de cada coluna sobre as instâncias
    for linhas in (qualidade, tempo, literatura):
        colunas = [c for c in linhas[0] if c not in ('Instancia', 'N')]
        linhas.append({'Instancia': 'Media Geral', 'N': '',
                       **{c: media_coluna(linhas, c) for c in colunas}})

    # ---------- Síntese por porte ----------
    sintese = []
    grupos = {nome: (lo, hi) for nome, (lo, hi) in PORTES.items()}
    grupos['Total'] = (0, 10**9)
    for nome, (lo, hi) in grupos.items():
        idx = [k for k, inst in enumerate(instancias) if lo <= numero_ordens(inst) <= hi]
        Q = [qualidade[k] for k in idx]
        T = [tempo[k] for k in idx]
        Lt = [literatura[k] for k in idx]
        conta = lambda cond: sum(1 for x in cond if x)
        sintese.append({
            'Porte': nome,
            'Instancias': len(idx),
            # contagens: "<=" inclui empates; "<" são reduções estritas
            'Melhor<=Orig': conta(q['Atual_melhor'] <= q['Orig_melhor'] for q in Q),
            'Melhor<Orig':  conta(q['Atual_melhor'] < q['Orig_melhor'] for q in Q),
            'Medio<=Orig':  conta(q['Atual_FO_medio'] <= q['Orig_FO_medio'] for q in Q),
            'Medio<Orig':   conta(q['Atual_FO_medio'] < q['Orig_FO_medio'] for q in Q),
            'Pior<=Orig':   conta(q['Atual_pior'] <= q['Orig_pior'] for q in Q),
            'Pior<Orig':    conta(q['Atual_pior'] < q['Orig_pior'] for q in Q),
            'Gap_medio_%':  media_coluna(Q, 'Gap_%'),
            'Speedup_medio': media_coluna(T, 'Speedup'),   # média das razões por instância
            'Melhor<=BKV_Lit': conta(l['Atual_melhor'] <= l['BKV_Lit'] for l in Lt),
            'Novos_BKV':       conta(l['Atual_melhor'] < l['BKV_Lit'] for l in Lt),
            'RPD_GRASP_%': media_coluna(Lt, 'GRASP_RPD_%'),
            'RPD_ILS_%':   media_coluna(Lt, 'ILS_RPD_%'),
            'RPD_Atual_%': media_coluna(Lt, 'Atual_RPD_%'),
        })

    salvar('qualidade.csv', qualidade)
    salvar('tempo.csv', tempo)
    salvar('literatura.csv', literatura)
    salvar('sintese_porte.csv', sintese)


if __name__ == '__main__':
    main()
