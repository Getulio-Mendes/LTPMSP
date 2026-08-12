#!/usr/bin/env python3
import argparse
import csv
from collections import defaultdict
from pathlib import Path
import statistics


def normalize_instance_name(header_line):
    return Path(header_line).name


def parse_result_line(line):
    parts = line.split(';')
    if len(parts) < 2:
        return None
    try:
        return float(parts[0]), float(parts[1])
    except ValueError:
        return None


def collect_results(results_dir, data, algo_name):
    if not results_dir.exists() or not results_dir.is_dir():
        return
    for filepath in sorted(results_dir.glob('*.txt')):
        current_instance = None
        try:
            with filepath.open('r', encoding='utf-8', errors='ignore') as handle:
                for raw_line in handle:
                    line = raw_line.strip()
                    if not line:
                        continue
                    if line.endswith('.txt'):
                        current_instance = normalize_instance_name(line)
                        continue
                    if current_instance is None:
                        continue
                    parsed = parse_result_line(line)
                    if parsed is None:
                        continue
                    fo, time_value = parsed
                    if algo_name not in data[current_instance]:
                        data[current_instance][algo_name] = {'fo': [], 'time': []}
                    data[current_instance][algo_name]['fo'].append(fo)
                    data[current_instance][algo_name]['time'].append(time_value)
        except OSError:
            continue


def mean(values):
    return statistics.mean(values) if values else 0.0


def std(values):
    return statistics.stdev(values) if len(values) > 1 else 0.0


def stats_row(values):
    """Retorna (média, melhor, pior, desvio) para uma lista de valores."""
    return mean(values), min(values), max(values), std(values)


def main():
    parser = argparse.ArgumentParser(description='Compara resultados Original vs Atual')
    parser.add_argument('--runs', type=int, default=10, help='Número de execuções a considerar (default: 10)')
    args = parser.parse_args()
    n_runs = args.runs

    workspace_root = Path(__file__).resolve().parent
    original_dir = workspace_root / 'Experimentos' / 'Original' / 'results'
    meu_dir = workspace_root / 'Experimentos' / 'Meu' / 'results'

    data = defaultdict(dict)
    collect_results(original_dir, data, 'Original')
    collect_results(meu_dir, data, 'Atual')

    # Instâncias presentes em ambos
    has_both = lambda inst: all(
        algo in data[inst] and data[inst][algo]['fo']
        for algo in ('Original', 'Atual')
    )
    instances = sorted(filter(has_both, data))

    # Cabeçalho do relatório por instância
    headers = [
        'Instancia',
        'Original fo_medio', 'Original melhor_fo', 'Original pior_fo', 'Original desvio_fo',
        'Original tempo_medio', 'Original desvio_tempo',
        'Atual fo_medio', 'Atual melhor_fo', 'Atual pior_fo', 'Atual desvio_fo',
        'Atual tempo_medio', 'Atual desvio_tempo',
        'gap'
    ]

    rows = []
    # Acumuladores para tabela de médias
    acc_orig_fo, acc_orig_t = [], []
    acc_atual_fo, acc_atual_t = [], []
    gaps = []

    for inst in instances:
        orig_fo = data[inst]['Original']['fo'][:n_runs]
        orig_t  = data[inst]['Original']['time'][:n_runs]
        atual_fo = data[inst]['Atual']['fo'][:n_runs]
        atual_t  = data[inst]['Atual']['time'][:n_runs]

        o_fo_mean, o_fo_best, o_fo_worst, o_fo_std = stats_row(orig_fo)
        o_t_mean, _, _, o_t_std = stats_row(orig_t)
        a_fo_mean, a_fo_best, a_fo_worst, a_fo_std = stats_row(atual_fo)
        a_t_mean, _, _, a_t_std = stats_row(atual_t)

        gap = (o_fo_mean - a_fo_mean) / o_fo_mean if o_fo_mean != 0 else 0.0

        acc_orig_fo.append(o_fo_mean)
        acc_orig_t.append(o_t_mean)
        acc_atual_fo.append(a_fo_mean)
        acc_atual_t.append(a_t_mean)
        gaps.append(gap)

        fmt = lambda v: f"{v:.2f}"
        rows.append([
            inst,
            fmt(o_fo_mean), fmt(o_fo_best), fmt(o_fo_worst), fmt(o_fo_std),
            fmt(o_t_mean), fmt(o_t_std),
            fmt(a_fo_mean), fmt(a_fo_best), fmt(a_fo_worst), fmt(a_fo_std),
            fmt(a_t_mean), fmt(a_t_std),
            f"{gap * 100:.2f}%"
        ])

    output_file = workspace_root / f'relatorio_{n_runs}.csv'
    with open(output_file, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(headers)
        w.writerows(rows)

        w.writerow([])
        w.writerow(['Tabela Médias'])
        w.writerow(['Algoritmo', 'fo_medio', 'tempo_medio', 'gap_medio'])
        w.writerow(['Original', f"{mean(acc_orig_fo):.2f}", f"{mean(acc_orig_t):.2f}", ''])
        w.writerow(['Atual', f"{mean(acc_atual_fo):.2f}", f"{mean(acc_atual_t):.2f}", f"{mean(gaps) * 100:.2f}%"
        ])

    print(f"Relatório gerado: {output_file}")


if __name__ == '__main__':
    main()
