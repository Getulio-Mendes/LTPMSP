#!/usr/bin/env python3
"""Gera, em CSV, as estatísticas de quantidade de avaliações da função objetivo (contador).

Entradas:
  Experimentos/Original/results/*.txt  ILS original (até 10 primeiras execuções pareadas)
  Experimentos/Meu/results/*.txt       Abordagem proposta, ID 41 (nível superior da pasta)

Saídas (pasta tabelas_resultados/):
  avaliacoes.csv          -> Todas as 37 instâncias pareadas + Média Geral
  avaliacoes_n4800.csv    -> 9 instâncias com N >= 4.800 (corresponde a tab:avaliacoes_fo)
  avaliacoes_sintese.csv  -> Síntese por faixa de porte (média, geomean, min, max, somas)

Uso: python3 gerar_tabela_avaliacoes.py
"""
import csv
import math
import statistics
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
DIR_EXP = (RAIZ / 'Experimentos') if (RAIZ / 'Experimentos').exists() else (RAIZ.parent / 'TCC' / 'Experimentos')
DIR_ORIG = DIR_EXP / 'Original' / 'results'
DIR_ATUAL = DIR_EXP / 'Meu' / 'results'
SAIDA = RAIZ / 'tabelas_resultados'

EXECUCOES = 10
EXCLUIDAS = {'LTPMSP_02_04800_00078_01286t'}
THREADS_ATUAL = 10
FAIXAS = {
    'Medio (N<=600)': (150, 600),
    'Intermediarias (1200-2400)': (1200, 2400),
    'Maiores escalas (N>=4800)': (4800, 33484),
    'Grande (1200-4800)': (1200, 4800),
    'Extra Grande (9600-33484)': (9600, 33484),
    'Total (37)': (0, 10**9),
}


def ler_resultados(pasta):
    """{instância: [(fo, tempo, iteracoes, avaliacoes), ...]} lida dos arquivos .txt.
    Formato do arquivo: linha com caminho .txt da instância, seguida de 'fo;tempo;iter;avaliacoes'."""
    dados = {}
    for arquivo in sorted(pasta.glob('*.txt')):
        instancia = None
        for linha in arquivo.read_text().splitlines():
            linha = linha.strip()
            if linha.endswith('.txt'):
                instancia = Path(linha).stem
                dados.setdefault(instancia, [])
            elif instancia and ';' in linha:
                partes = linha.split(';')
                if len(partes) >= 4:
                    fo, tempo, iteracoes, avaliacoes = partes[:4]
                    dados[instancia].append((
                        float(fo),
                        float(tempo),
                        int(iteracoes),
                        int(avaliacoes),
                    ))
    return dados


def numero_ordens(instancia):
    # nome: LTPMSP_GG_NNNNN_MMMMM_QQQQQ
    return int(instancia.split('_')[2])


def media_geometrica(valores):
    return math.exp(statistics.mean(math.log(v) for v in valores))


def salvar(nome, linhas):
    # números reais gravados com 4 casas decimais
    linhas = [{c: round(v, 4) if isinstance(v, float) else v for c, v in l.items()} for l in linhas]
    with open(SAIDA / nome, 'w', newline='') as f:
        escritor = csv.DictWriter(f, fieldnames=linhas[0].keys())
        escritor.writeheader()
        escritor.writerows(linhas)
    print(f'gerado: {SAIDA / nome}')


def montar_sintese(linhas):
    """Agrega razões e somas de avaliações por faixa de N."""
    sintese = []
    for nome, (lo, hi) in FAIXAS.items():
        sub = [l for l in linhas if lo <= l['N'] <= hi]
        razoes_av = [l['Razao_aval'] for l in sub]
        razoes_tx = [l['Razao_taxa'] for l in sub]
        soma_orig = sum(l['Orig_aval_media'] for l in sub)
        soma_atual = sum(l['Atual_aval_media'] for l in sub)
        sintese.append({
            'Faixa': nome,
            'Instancias': len(sub),
            'Razao_aval_media': statistics.mean(razoes_av),
            'Razao_aval_geomean': media_geometrica(razoes_av),
            'Razao_aval_min': min(razoes_av),
            'Razao_aval_max': max(razoes_av),
            'Razao_taxa_media': statistics.mean(razoes_tx),
            'Razao_taxa_geomean': media_geometrica(razoes_tx),
            'Razao_taxa_min': min(razoes_tx),
            'Razao_taxa_max': max(razoes_tx),
            'Eficiencia_10t_media_%': statistics.mean(l['Eficiencia_10t_%'] for l in sub),
            'Soma_aval_Orig': soma_orig,
            'Soma_aval_Atual': soma_atual,
            'Razao_soma_aval': soma_atual / soma_orig,
        })
    return sintese


def main():
    orig = ler_resultados(DIR_ORIG)
    atual = ler_resultados(DIR_ATUAL)
    instancias = sorted(i for i in orig if i in atual and i not in EXCLUIDAS)
    SAIDA.mkdir(exist_ok=True)

    linhas = []
    for inst in instancias:
        # Pareamento estrito: min(disponíveis Original, disponíveis Atual, EXECUCOES)
        n_exec = min(len(orig[inst]), len(atual[inst]), EXECUCOES)
        o_runs = orig[inst][:n_exec]
        a_runs = atual[inst][:n_exec]

        o_t = [r[1] for r in o_runs]
        o_it = [r[2] for r in o_runs]
        o_av = [r[3] for r in o_runs]

        a_t = [r[1] for r in a_runs]
        a_it = [r[2] for r in a_runs]
        a_av = [r[3] for r in a_runs]

        o_av_media = statistics.mean(o_av)
        o_t_medio = statistics.mean(o_t)
        o_taxa = o_av_media / o_t_medio
        o_taxa_exec = statistics.mean(av / t for av, t in zip(o_av, o_t) if t > 0)

        a_av_media = statistics.mean(a_av)
        a_t_medio = statistics.mean(a_t)
        a_taxa = a_av_media / a_t_medio
        a_taxa_exec = statistics.mean(av / t for av, t in zip(a_av, a_t) if t > 0)

        o_it_media = statistics.mean(o_it)
        a_it_media = statistics.mean(a_it)
        razao_av = a_av_media / o_av_media
        razao_tx = a_taxa / o_taxa

        linhas.append({
            'Instancia': inst,
            'N': numero_ordens(inst),
            'Execucoes': n_exec,
            'Orig_aval_media': o_av_media,
            'Orig_aval_desvio': statistics.stdev(o_av) if n_exec > 1 else 0.0,
            'Orig_tempo_medio': o_t_medio,
            'Orig_aval_por_s': o_taxa,
            'Orig_aval_por_s_media_exec': o_taxa_exec,
            'Orig_iter_media': o_it_media,
            'Atual_aval_media': a_av_media,
            'Atual_aval_desvio': statistics.stdev(a_av) if n_exec > 1 else 0.0,
            'Atual_tempo_medio': a_t_medio,
            'Atual_aval_por_s': a_taxa,
            'Atual_aval_por_s_media_exec': a_taxa_exec,
            'Atual_iter_media': a_it_media,
            'Razao_aval': razao_av,
            'Razao_taxa': razao_tx,
            'Eficiencia_10t_%': razao_tx / THREADS_ATUAL * 100.0,
            'Razao_iter': (a_it_media / o_it_media) if o_it_media > 0 else '',
            'Speedup': o_t_medio / a_t_medio,
        })

    sintese = montar_sintese(linhas)

    # Recorte das 9 instâncias com N >= 4.800 (corresponde à tab:avaliacoes_fo)
    colunas_n4800 = [
        'Instancia', 'N', 'Execucoes',
        'Orig_aval_media', 'Orig_aval_por_s',
        'Atual_aval_media', 'Atual_aval_por_s',
        'Razao_aval', 'Razao_taxa', 'Eficiencia_10t_%',
        'Orig_iter_media', 'Atual_iter_media',
    ]
    linhas_n4800 = [{c: l[c] for c in colunas_n4800} for l in linhas if l['N'] >= 4800]
    linhas_n4800.append({
        'Instancia': 'Media das razoes',
        'N': '',
        'Execucoes': '',
        'Orig_aval_media': '',
        'Orig_aval_por_s': '',
        'Atual_aval_media': '',
        'Atual_aval_por_s': '',
        'Razao_aval': statistics.mean(l['Razao_aval'] for l in linhas_n4800),
        'Razao_taxa': statistics.mean(l['Razao_taxa'] for l in linhas_n4800),
        'Eficiencia_10t_%': statistics.mean(l['Eficiencia_10t_%'] for l in linhas_n4800),
        'Orig_iter_media': '',
        'Atual_iter_media': '',
    })

    # Linha "Media Geral" em avaliacoes.csv
    colunas_media = [c for c in linhas[0] if c not in ('Instancia', 'N', 'Execucoes')]
    media_geral = {'Instancia': 'Media Geral', 'N': '', 'Execucoes': ''}
    for c in colunas_media:
        valores = [l[c] for l in linhas if isinstance(l[c], (int, float))]
        media_geral[c] = statistics.mean(valores) if valores else ''
    linhas_completas = [*linhas, media_geral]

    salvar('avaliacoes.csv', linhas_completas)
    salvar('avaliacoes_n4800.csv', linhas_n4800)
    salvar('avaliacoes_sintese.csv', sintese)

    for s in sintese:
        if s['Faixa'] in (
            'Medio (N<=600)',
            'Intermediarias (1200-2400)',
            'Maiores escalas (N>=4800)',
            'Total (37)',
        ):
            print(
                f"{s['Faixa']} ({s['Instancias']} inst.): "
                f"Razao_aval={s['Razao_aval_media']:.4f}x (geomean={s['Razao_aval_geomean']:.4f}x, "
                f"soma={s['Razao_soma_aval']:.4f}x) | "
                f"Razao_taxa={s['Razao_taxa_media']:.4f}x (ef={s['Eficiencia_10t_media_%']:.2f}%)"
            )


if __name__ == '__main__':
    main()

