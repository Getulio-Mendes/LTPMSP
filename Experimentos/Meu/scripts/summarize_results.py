#!/usr/bin/env python3
from collections import defaultdict
from pathlib import Path
import statistics


def get_results_dir():
    return Path(__file__).resolve().parent.parent / 'results'


def list_result_files(results_dir):
    return sorted(results_dir.glob('*.txt'))


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


def collect_results(results_dir):
    data = defaultdict(lambda: {'fo': [], 'time': []})

    for filepath in list_result_files(results_dir):
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
                    data[current_instance]['fo'].append(fo)
                    data[current_instance]['time'].append(time_value)
        except OSError:
            continue

    return data


def summarize(values):
    runs = len(values)
    average = statistics.mean(values)
    deviation = statistics.stdev(values) if runs > 1 else 0.0
    return runs, average, deviation


def format_markdown_table(data):
    headers = ["Instance", "Runs", "Avg FO", "Std Dev FO", "Avg Time (s)", "Std Dev Time"]
    rows = []

    for instance in sorted(data):
        fos, times = data[instance]['fo'], data[instance]['time']
        if not fos or not times:
            continue

        runs, avg_fo, std_fo = summarize(fos)
        _, avg_time, std_time = summarize(times)
        rows.append([instance, str(runs), f"{avg_fo:.2f}", f"{std_fo:.2f}", f"{avg_time:.4f}", f"{std_time:.4f}"])

    if not rows:
        return []

    widths = [max(len(str(item)) for item in col) for col in zip(headers, *rows)]
    
    def pad(row):
        cols = []
        for i, (val, w) in enumerate(zip(row, widths)):
            if i == 0:
                cols.append(val.ljust(w))
            else:
                cols.append(val.rjust(w))
        return " | ".join(cols)

    seps = ["-" * w for w in widths]

    lines = [
        f"| {pad(headers)} |",
        f"| {' | '.join(seps)} |"
    ]
    lines.extend(f"| {pad(row)} |" for row in rows)

    return lines


def main():
    results_dir = get_results_dir()
    data = collect_results(results_dir)

    for line in format_markdown_table(data):
        print(line)


if __name__ == '__main__':
    main()
