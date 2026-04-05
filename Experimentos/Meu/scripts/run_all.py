#!/usr/bin/env python3
import argparse
import glob
import os
import subprocess
import sys


def find_instances(data_dir):
    files = []
    for name in sorted(os.listdir(data_dir)):
        path = os.path.join(data_dir, name)
        if not os.path.isfile(path):
            continue
        if not name.lower().endswith('.txt'):
            continue
        if '_BEST_ILS' in name:
            # ignore the BEST files themselves
            continue
        files.append(name)
    return files


def count_collected(data_dir, results_dir, instance_name):
    total = 0
    if not os.path.isdir(results_dir):
        return 0

    # variants that may appear in result files
    rel = os.path.join(data_dir, instance_name)
    variants = {rel, f"./{rel}", instance_name}

    for entry in sorted(os.listdir(results_dir)):
        path = os.path.join(results_dir, entry)
        if not os.path.isfile(path):
            continue
        if not entry.lower().endswith('.txt'):
            continue
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                lines = [ln.rstrip('\n') for ln in fh]
        except Exception:
            continue

        i = 0
        while i < len(lines):
            line = lines[i]
            if any(v in line for v in variants):
                # scan subsequent lines for data entries
                j = i + 1
                while j < len(lines):
                    l = lines[j].strip()
                    # if this looks like the header of the next instance, break
                    if '.txt' in l and '/' in l:
                        break
                    # count lines with >= 3 semicolons as data entries
                    if l.count(';') >= 3:
                        total += 1
                    j += 1
                i = j
            else:
                i += 1

    return total


def run_instance(binary, instance_path, runs=10, dry_run=False):
    cmd = [binary, instance_path]
    cmd.append(str(runs))
    print(f"Running: {' '.join(cmd)}")
    if dry_run:
        return 0
    try:
        res = subprocess.run(cmd)
        return res.returncode
    except FileNotFoundError:
        print(f"Error: binary not found: {binary}")
        return 2
    except Exception as e:
        print(f"Error running command: {e}")
        return 3


def main():
    parser = argparse.ArgumentParser(description='Run manut on all instances in a data directory')
    parser.add_argument('--data-dir', default='./data', help='Directory with instance files')
    parser.add_argument('--results-dir', default='./results', help='Directory where manut writes result files')
    parser.add_argument('--binary', default='./manut', help='Path to the manut executable')
    parser.add_argument('--dry-run', action='store_true', help='Only print what would be run')
    parser.add_argument('--exit-on-error', action='store_true', help='Stop on first non-zero exit code')
    parser.add_argument('--target', type=int, default=3, help='Target number of collections per instance')
    args = parser.parse_args()

    data_dir = args.data_dir
    if not os.path.isdir(data_dir):
        print(f"Data directory not found: {data_dir}")
        sys.exit(1)

    instances = find_instances(data_dir)
    if not instances:
        print(f"No instance files to process in {data_dir}")
        sys.exit(0)

    print(f"Found {len(instances)} candidate instance(s) in {data_dir}")

    failures = 0
    for name in instances:
        total = count_collected(data_dir, args.results_dir, name)
        if total >= args.target:
            print(f"Skipping {name}: already collected {total}/{args.target}")
            continue

        need = args.target - total
        print(f"Instance {name}: collected {total}/{args.target} -> need {need} run(s)")

        if need <= 0:
            continue

        instance_path = os.path.join(data_dir, name)
        # call manut once with the needed count as an additional argument
        rc = run_instance(args.binary, instance_path, runs=need, dry_run=args.dry_run)
        if rc != 0:
            print(f"Instance {name} run (needed={need}) returned exit code {rc}")
            failures += 1
            if args.exit_on_error:
                break

    if failures:
        print(f"Completed with {failures} failures")
        sys.exit(4)
    print("All done.")


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nInterrupted by user')
        sys.exit(130)
