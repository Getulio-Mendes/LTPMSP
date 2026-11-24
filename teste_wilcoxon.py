#!/usr/bin/env python3
"""Executa o teste não paramétrico dos postos sinalizados de Wilcoxon (pareado, bilateral)
sobre os resultados médios por instância (ILS Original vs. Abordagem Proposta - ID 41).

Entradas (pasta tabelas_resultados/):
  qualidade.csv
  tempo.csv

Saída (pasta tabelas_resultados/):
  wilcoxon.csv -> tab:wilcoxon

Metodologia:
  - Para FO Médio, calcula-se em cada instância i o gap percentual relativo
    d_i = (Orig_i - Atual_i) / Orig_i * 100%, a fim de assegurar comensurabilidade
    entre instâncias de diferentes ordens de grandeza (Demšar, 2006). Valores d_i > 0
    favorecem a abordagem proposta (menor custo médio).
  - Para Tempo Médio, calcula-se a redução percentual relativa
    d_i = (Orig_i - Atual_i) / Orig_i * 100%, censurando como empate nulo (d_i = 0)
    as instâncias em que ambos os métodos atingiram o tempo-limite de N segundos
    (Orig_i >= N e Atual_i >= N).
  - Diferenças nulas (|d_i| <= 1e-9) são contabilizadas como empates e excluídas do
    ranqueamento (procedimento padrão de Wilcoxon, 1945), restando n diferenças não nulas.
  - Ordenam-se os valores absolutos |d_i| em ordem crescente e atribuem-se postos de 1 a n,
    usando o posto médio em caso de empate entre valores absolutos.
  - Somam-se os postos positivos (W+, d_i > 0) e negativos (W-, d_i < 0), definindo
    W_min = min(W+, W-).
  - Calcula-se a estatística normal assintótica Z = (W+ - media_W) / desvio_W (sem
    correção de continuidade, com correção de variância para empates nos postos), em que
    Z > 0 indica predominância dos postos favoráveis à abordagem proposta, além do
    valor-p exato bilateral via programação dinâmica sobre as 2^n combinações de sinais.

Uso: python3 teste_wilcoxon.py
"""
import csv
import math
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
SAIDA = RAIZ / 'tabelas_resultados'

CALIBRACAO = {
    'LTPMSP_01_04800_00197_01128',
    'LTPMSP_03_04800_00130_00720',
    'LTPMSP_06_19200_00132_00908',
}


def ler_csv_instancias(caminho):
    with open(caminho) as f:
        return [linha for linha in csv.DictReader(f) if linha['Instancia'] != 'Media Geral']


def formatar_posto(valor):
    arred = round(valor, 2)
    return int(arred) if arred.is_integer() else arred


def gap_relativo(orig, atual):
    return (orig - atual) / orig * 100.0


def reducao_tempo_censurada(orig, atual, limite_n):
    if orig >= limite_n and atual >= limite_n:
        return 0.0
    return (orig - atual) / orig * 100.0


def wilcoxon_pareado(diferencas_brutas, tol=1e-9):
    """Recebe lista de diferenças pareadas d_i e retorna dicionário com estatísticas do teste."""
    diferencas = [d for d in diferencas_brutas if abs(d) > tol]
    empates = len(diferencas_brutas) - len(diferencas)
    n = len(diferencas)

    if n == 0:
        return {
            'Total_inst': len(diferencas_brutas),
            'Empates': empates,
            'N_efetivo': 0,
            'Favor_Atual': 0,
            'Favor_Orig': 0,
            'W_pos': 0,
            'W_neg': 0,
            'W_min': 0,
            'Z': 0.0,
            'p_assintotico': '1',
            'p_exato': '1',
            'Significativo_5%': 'Nao',
        }

    # Ordena índices pelo valor absoluto |d_i| e atribui postos médios em caso de empate
    abs_ordenados = sorted(enumerate(abs(d) for d in diferencas), key=lambda item: item[1])
    postos = [0.0] * n
    soma_correcao_empates = 0.0
    i = 0
    while i < n:
        j = i
        while j < n and abs(abs_ordenados[j][1] - abs_ordenados[i][1]) <= tol:
            j += 1
        posto_medio = (i + 1 + j) / 2.0
        for k in range(i, j):
            postos[abs_ordenados[k][0]] = posto_medio
        t = j - i
        soma_correcao_empates += t**3 - t
        i = j

    w_pos = sum(r for d, r in zip(diferencas, postos) if d > 0)
    w_neg = sum(r for d, r in zip(diferencas, postos) if d < 0)
    w = min(w_pos, w_neg)

    # Aproximação normal assintótica (sem correção de continuidade, com correção de empates)
    media_w = n * (n + 1) / 4.0
    variancia_w = (n * (n + 1) * (2 * n + 1)) / 24.0 - soma_correcao_empates / 48.0
    desvio_w = math.sqrt(variancia_w)
    z = (w_pos - media_w) / desvio_w
    p_assintotico = math.erfc(abs(z) / math.sqrt(2.0))

    # Valor-p exato bilateral por programação dinâmica sobre 2 * posto (sempre inteiro)
    postos_duplos = [int(round(2 * r)) for r in postos]
    dp = {0: 1}
    for r2 in postos_duplos:
        novo_dp = dict(dp)
        for soma, contagem in dp.items():
            novo_dp[soma + r2] = novo_dp.get(soma + r2, 0) + contagem
        dp = novo_dp
    alvo = int(round(2 * w))
    soma_total = sum(postos_duplos)
    cauda_bilateral = sum(
        contagem for soma, contagem in dp.items()
        if soma <= alvo or soma >= soma_total - alvo
    )
    p_exato = min(1.0, cauda_bilateral / (2**n))

    return {
        'Total_inst': len(diferencas_brutas),
        'Empates': empates,
        'N_efetivo': n,
        'Favor_Atual': sum(1 for d in diferencas if d > 0),
        'Favor_Orig': sum(1 for d in diferencas if d < 0),
        'W_pos': formatar_posto(w_pos),
        'W_neg': formatar_posto(w_neg),
        'W_min': formatar_posto(w),
        'Z': round(z, 4),
        'p_assintotico': f'{p_assintotico:.4g}',
        'p_exato': f'{p_exato:.4g}',
        'Significativo_5%': 'Sim' if p_exato < 0.05 else 'Nao',
    }


def main():
    qualidade = ler_csv_instancias(SAIDA / 'qualidade.csv')
    tempo = ler_csv_instancias(SAIDA / 'tempo.csv')

    recortes = [
        (
            'FO Medio - Conjunto completo (37 inst.)',
            [
                gap_relativo(float(q['Orig_FO_medio']), float(q['Atual_FO_medio']))
                for q in qualidade
            ],
        ),
        (
            'FO Medio - Sem calibracao (34 inst.)',
            [
                gap_relativo(float(q['Orig_FO_medio']), float(q['Atual_FO_medio']))
                for q in qualidade if q['Instancia'] not in CALIBRACAO
            ],
        ),
        (
            'FO Medio - Maiores escalas N >= 4800 (9 inst.)',
            [
                gap_relativo(float(q['Orig_FO_medio']), float(q['Atual_FO_medio']))
                for q in qualidade if int(q['N']) >= 4800
            ],
        ),
        (
            'FO Medio - Maiores escalas s/ calibracao N >= 4800 (6 inst.)',
            [
                gap_relativo(float(q['Orig_FO_medio']), float(q['Atual_FO_medio']))
                for q in qualidade
                if int(q['N']) >= 4800 and q['Instancia'] not in CALIBRACAO
            ],
        ),
        (
            'Tempo Medio - Conjunto completo (37 inst.)',
            [
                reducao_tempo_censurada(
                    float(t['Orig_tempo_medio']),
                    float(t['Atual_tempo_medio']),
                    float(t['N']),
                )
                for t in tempo
            ],
        ),
    ]

    resultados = []
    for descricao, diferencas in recortes:
        est = wilcoxon_pareado(diferencas)
        linha = {'Comparacao': descricao, **est}
        resultados.append(linha)
        print(
            f"{descricao}: n={est['N_efetivo']} (empates={est['Empates']}), "
            f"W+={est['W_pos']}, W-={est['W_neg']}, Z={est['Z']:+.2f}, "
            f"p_assint={est['p_assintotico']}, p_exato={est['p_exato']} "
            f"({est['Significativo_5%']})"
        )

    caminho_saida = SAIDA / 'wilcoxon.csv'
    with open(caminho_saida, 'w', newline='') as f:
        escritor = csv.DictWriter(f, fieldnames=resultados[0].keys())
        escritor.writeheader()
        escritor.writerows(resultados)
    print(f'gerado: {caminho_saida}')


if __name__ == '__main__':
    main()
