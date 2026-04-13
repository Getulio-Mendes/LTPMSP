#!/usr/bin/env python3
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


def collect_results_for_algo(algo_name, results_dir, data):
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


def summarize(values):
    runs = len(values)
    if runs == 0:
        return 0, 0.0, 0.0, 0.0
    best = min(values)
    average = statistics.mean(values)
    deviation = statistics.stdev(values) if runs > 1 else 0.0
    return runs, best, average, deviation


def export_to_csv(data, output_file):
    if not data:
        print("No data collected.")
        return

    # Find all algorithms
    algorithms = set()
    for instance in data:
        algorithms.update(data[instance].keys())
    
    # Sort algorithms: Original, Current, then alphabetically
    ordered_algos = []
    if 'Original' in algorithms: ordered_algos.append('Original')
    if 'Current' in algorithms: ordered_algos.append('Current')
    ordered_algos.extend(sorted([a for a in algorithms if a not in ('Original', 'Current')]))

    headers = ['Instance', 'Best_Algo']
    for algo in ordered_algos:
        headers.extend([
            f"{algo} Runs",
            f"{algo} Best_FO",
            f"{algo} Avg FO",
            f"{algo} Avg Time (s)"
        ])

    try:
        with open(output_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(headers)

            for instance in sorted(data):
                # Find best algo(s) for this instance
                best_overall_fo = float('inf')
                best_algos = []
                
                for algo in ordered_algos:
                    if algo in data[instance]:
                        runs_fo, best_fo, avg_fo, std_fo = summarize(data[instance][algo]['fo'])
                        if runs_fo > 0:
                            if best_fo < best_overall_fo:
                                best_overall_fo = best_fo
                                best_algos = [algo]
                            elif best_fo == best_overall_fo:
                                best_algos.append(algo)
                
                best_algo_str = ", ".join(best_algos)
                
                row = [instance, best_algo_str]
                for algo in ordered_algos:
                    if algo in data[instance]:
                        metrics = data[instance][algo]
                        runs_fo, best_fo, avg_fo, std_fo = summarize(metrics['fo'])
                        runs_t, best_t, avg_time, std_time = summarize(metrics['time'])
                        # Combine Avg and Std using +-
                        row.extend([
                            runs_fo, 
                            f"{best_fo:.2f}", 
                            f"{avg_fo:.2f} +- {std_fo:.2f}", 
                            f"{avg_time:.2f} +- {std_time:.2f}"
                        ])
                    else:
                        row.extend([0, "", "", ""])
                writer.writerow(row)
        print(f"Results successfully summarized and exported to {output_file}")
    except IOError as e:
        print(f"Failed to write CSV file: {e}")


def main():
    # We resolve from the current file's directory path since it's saved in the project root
    workspace_root = Path(__file__).resolve().parent
    
    original_dir = workspace_root / 'Experimentos' / 'Original' / 'results'
    meu_dir = workspace_root / 'Experimentos' / 'Meu' / 'results'

    data = defaultdict(dict)

    # Baseline: Original
    collect_results_for_algo('Original', original_dir, data)

    # Current version (only .txt files at the root of Meu/results)
    collect_results_for_algo('Current', meu_dir, data)

    # Subdirectories in Meu/results are distinct algorithms
    if meu_dir.exists() and meu_dir.is_dir():
        for item in meu_dir.iterdir():
            if item.is_dir():
                collect_results_for_algo(item.name, item, data)

    output_file = workspace_root / 'summary.csv'
    export_to_csv(data, output_file)


if __name__ == '__main__':
    main()