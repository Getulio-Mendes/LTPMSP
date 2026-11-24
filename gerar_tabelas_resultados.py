#!/usr/bin/env python3
"""Gera, em CSV, os dados das tabelas do capítulo de Resultados.

Entradas:
  Experimentos/Original/results/*.txt  ILS original (até 10 primeiras execuções pareadas)
  Experimentos/Meu/results/*.txt       Abordagem proposta, ID 41 (nível superior da pasta)
  ../PFC1/Table-10.csv                 Tabela 10 de Coelho, Souza e Cota (2026)

Saídas (pasta tabelas_resultados/):
  qualidade.csv                    -> tab:resultados_qualidade (37 instâncias)
  tempo.csv                        -> tab:resultados_tempo (37 instâncias)
  literatura.csv                   -> tab:literatura (37 instâncias, inclui RPD do ILS local)
  sintese_porte.csv                -> tab:sintese_porte (37 instâncias)
  sintese_porte_sem_calibracao.csv -> tab:sintese_sem_calibracao (34 instâncias sem treino do irace)

Uso: python3 gerar_tabelas_resultados.py
"""
import csv
import statistics
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
DIR_EXP = (RAIZ / 'Experimentos') if (RAIZ / 'Experimentos').exists() else (RAIZ.parent / 'TCC' / 'Experimentos')
DIR_ORIG = DIR_EXP / 'Original' / 'results'
DIR_ATUAL = DIR_EXP / 'Meu' / 'results'
TABELA_10 = (RAIZ / 'Table-10.csv') if (RAIZ / 'Table-10.csv').exists() else (RAIZ.parent / 'PFC1' / 'Table-10.csv')
SAIDA = RAIZ / 'tabelas_resultados'

EXECUCOES = 10
EXCLUIDAS = {'LTPMSP_02_04800_00078_01286t'}
CALIBRACAO = {
    'LTPMSP_01_04800_00197_01128',
    'LTPMSP_03_04800_00130_00720',
    'LTPMSP_06_19200_00132_00908',
}
PORTES = {'Médio': (150, 600), 'Grande': (1200, 4800), 'Extra Grande': (9600, 33484)}


def ler_resultados(pasta):
    """{instância: (lista de FO, lista de tempos)} lida dos arquivos .txt de resultados.
    Formato do arquivo: linha com o caminho da instância, seguida de linhas 'fo;tempo;...'."""
    dados = {}
    for arquivo in sorted(pasta.glob('*.txt')):
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
    return dados


def ler_literatura():
    """{(N, M, Q): linha da Tabela 10}. Colunas: BKV=4, ILS Best/Avg=8/9, GRASP Best/Avg=20/21."""
    lit = {}
    with open(TABELA_10) as f:
        for linha in csv.reader(f):
            if linha and linha[0].isdigit():
                n, m, q = (int(x.strip('*')) for x in linha[1:4])
                lit[(n, m, q)] = {
                    'BKV': float(linha[4]),
                    'ILS_melhor': float(linha[8]),
                    'ILS_medio': float(linha[9]),
                    'GRASP_melhor': float(linha[20]),
                    'GRASP_medio': float(linha[21]),
                }
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


def montar_sintese(instancias, qualidade, tempo, literatura, filtro=None):
    """Agrega contagens e médias por porte (e Total), aplicando filtro opcional sobre o nome da instância."""
    sintese = []
    grupos = {nome: (lo, hi) for nome, (lo, hi) in PORTES.items()}
    grupos['Total'] = (0, 10**9)
    conta = lambda cond: sum(1 for x in cond if x)

    for nome, (lo, hi) in grupos.items():
        idx = [
            k for k, inst in enumerate(instancias)
            if lo <= numero_ordens(inst) <= hi and (filtro is None or filtro(inst))
        ]
        Q = [qualidade[k] for k in idx]
        T = [tempo[k] for k in idx]
        Lt = [literatura[k] for k in idx]
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
            'Speedup_medio': media_coluna(T, 'Speedup'),
            'Melhor<=BKV_Lit': conta(l['Atual_melhor'] <= l['BKV_Lit'] for l in Lt),
            'Novos_BKV':       conta(l['Atual_melhor'] < l['BKV_Lit'] for l in Lt),
            'RPD_GRASP_%': media_coluna(Lt, 'GRASP_RPD_%'),
            'RPD_ILS_%':   media_coluna(Lt, 'ILS_RPD_%'),
            'RPD_Orig_%':  media_coluna(Lt, 'Orig_RPD_%'),
            'RPD_Atual_%': media_coluna(Lt, 'Atual_RPD_%'),
        })
    return sintese


def main():
    orig = ler_resultados(DIR_ORIG)
    atual = ler_resultados(DIR_ATUAL)
    lit = ler_literatura()
    instancias = sorted(i for i in orig if i in atual and i not in EXCLUIDAS)
    SAIDA.mkdir(exist_ok=True)

    qualidade, tempo, literatura = [], [], []
    soma_exec_orig = 0.0
    soma_exec_atual = 0.0
    total_execucoes = 0

    for inst in instancias:
        fo_o_bruto, t_o_bruto = orig[inst]
        fo_a_bruto, t_a_bruto = atual[inst]
        # Pareamento estrito: usa min(disponíveis Original, disponíveis Atual, EXECUCOES)
        # (10 execuções para N <= 19.200; 8 execuções pareadas para N = 33.484)
        n_exec = min(len(fo_o_bruto), len(fo_a_bruto), EXECUCOES)
        fo_o, t_o = fo_o_bruto[:n_exec], t_o_bruto[:n_exec]
        fo_a, t_a = fo_a_bruto[:n_exec], t_a_bruto[:n_exec]

        soma_exec_orig += sum(t_o)
        soma_exec_atual += sum(t_a)
        total_execucoes += n_exec

        # ---------- Tabela de qualidade (mesmo hardware) ----------
        q = {
            'Instancia': inst,
            'N': numero_ordens(inst),
            'Execucoes': n_exec,
            'Orig_FO_medio': statistics.mean(fo_o),
            'Orig_melhor':   min(fo_o),
            'Orig_pior':     max(fo_o),
            'Orig_desvio':   statistics.stdev(fo_o),
            'Atual_FO_medio': statistics.mean(fo_a),
            'Atual_melhor':   min(fo_a),
            'Atual_pior':     max(fo_a),
            'Atual_desvio':   statistics.stdev(fo_a),
        }
        q['Gap_%'] = (q['Orig_FO_medio'] - q['Atual_FO_medio']) / q['Orig_FO_medio'] * 100
        qualidade.append(q)

        # ---------- Tabela de tempo ----------
        t = {
            'Instancia': inst,
            'N': numero_ordens(inst),
            'Execucoes': n_exec,
            'Orig_tempo_medio':  statistics.mean(t_o),
            'Orig_tempo_desvio': statistics.stdev(t_o),
            'Atual_tempo_medio':  statistics.mean(t_a),
            'Atual_tempo_desvio': statistics.stdev(t_a),
        }
        t['Speedup'] = round(t['Orig_tempo_medio'], 2) / round(t['Atual_tempo_medio'], 2)
        tempo.append(t)

        # ---------- Tabela da literatura ----------
        L = lit[tuple(int(x) for x in inst.split('_')[2:5])]
        fo_bkv = min(L['BKV'], q['Atual_melhor'])
        rpd = lambda fo_medio: (fo_medio - fo_bkv) / fo_bkv * 100
        literatura.append({
            'Instancia': inst,
            'N': numero_ordens(inst),
            'BKV_Lit': L['BKV'],
            'FO_BKV': fo_bkv,
            'GRASP_melhor': L['GRASP_melhor'], 'GRASP_FO_medio': L['GRASP_medio'], 'GRASP_RPD_%': rpd(L['GRASP_medio']),
            'ILS_melhor':   L['ILS_melhor'],   'ILS_FO_medio':   L['ILS_medio'],   'ILS_RPD_%':   rpd(L['ILS_medio']),
            'Orig_melhor':  q['Orig_melhor'],  'Orig_FO_medio':  q['Orig_FO_medio'], 'Orig_RPD_%': rpd(q['Orig_FO_medio']),
            'Atual_melhor': q['Atual_melhor'], 'Atual_FO_medio': q['Atual_FO_medio'], 'Atual_RPD_%': rpd(q['Atual_FO_medio']),
        })

    # Sínteses agregadas por porte (37 instâncias e 34 instâncias sem calibração)
    sintese_37 = montar_sintese(instancias, qualidade, tempo, literatura)
    sintese_34 = montar_sintese(
        instancias, qualidade, tempo, literatura,
        filtro=lambda inst: inst not in CALIBRACAO,
    )

    # Somas de tempos médios por bateria antes de anexar a linha "Media Geral"
    bateria_orig = sum(t['Orig_tempo_medio'] for t in tempo)
    bateria_atual = sum(t['Atual_tempo_medio'] for t in tempo)

    # Linha "Média Geral" = média aritmética de cada coluna sobre as instâncias
    for linhas in (qualidade, tempo, literatura):
        colunas = [c for c in linhas[0] if c not in ('Instancia', 'N', 'Execucoes')]
        base = {'Instancia': 'Media Geral', 'N': ''}
        if 'Execucoes' in linhas[0]:
            base['Execucoes'] = ''
        linhas.append({**base, **{c: media_coluna(linhas, c) for c in colunas}})

    salvar('qualidade.csv', qualidade)
    salvar('tempo.csv', tempo)
    salvar('literatura.csv', literatura)
    salvar('sintese_porte.csv', sintese_37)
    salvar('sintese_porte_sem_calibracao.csv', sintese_34)

    print(
        f'Tempos (bateria de {len(instancias)} instâncias): '
        f'Orig = {bateria_orig:.2f} s | Atual = {bateria_atual:.2f} s | '
        f'Redução = {(bateria_orig - bateria_atual) / bateria_orig * 100:.2f}% | '
        f'Razão thread-s (10x/1x) = {(10 * bateria_atual) / bateria_orig:.2f}x'
    )
    print(
        f'Tempos (acumulado de {total_execucoes} execuções): '
        f'Orig = {soma_exec_orig:.2f} s | Atual = {soma_exec_atual:.2f} s | '
        f'Redução = {(soma_exec_orig - soma_exec_atual) / soma_exec_orig * 100:.2f}% | '
        f'Razão thread-s (10x/1x) = {(10 * soma_exec_atual) / soma_exec_orig:.2f}x'
    )


if __name__ == '__main__':
    main()
