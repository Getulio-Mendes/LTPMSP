#!/usr/bin/env python3
import subprocess
import os

FLAMEGRAPH = "/home/getuio/Facul/TCC/FlameGraph"
RUNS = 3

INSTANCES = [
    ("data/LTPMSP_01_00600_00165_00329.txt", "results/LTPMSP_01.svg"),
    ("data/LTPMSP_02_00600_00256_00388.txt", "results/LTPMSP_02.svg"),
    ("data/LTPMSP_03_00600_00120_00288.txt", "results/LTPMSP_03.svg"),
    ("data/LTPMSP_04_00600_00077_00215.txt", "results/LTPMSP_04.svg"),
    ("data/LTPMSP_05_00600_00126_00279.txt", "results/LTPMSP_05.svg"),
    ("data/LTPMSP_06_04800_00129_00713.txt", "results/LTPMSP_06.svg"),
]

subprocess.run(["make", "profile"], check=True)

for instance, svg in INSTANCES:
    # Check if the result file already exists
    if os.path.exists(svg):
        print(f"\nSkipping {instance}, {svg} already exists.")
        continue

    print(f"\n{instance} -> {svg}")
    perf_data = svg + ".data"
    subprocess.run(["perf", "record", "-g", "-F", "99", "-o", perf_data,
                    "--", "./manut", instance, str(RUNS)], check=True)
    with open(svg, "w") as f:
        subprocess.run(
            f"perf script -i {perf_data}"
            f" | {FLAMEGRAPH}/stackcollapse-perf.pl"
            f" | {FLAMEGRAPH}/flamegraph.pl",
            shell=True, stdout=f, check=True,
        )
    print(f"Done: {svg}")

print("\nAll done.")
