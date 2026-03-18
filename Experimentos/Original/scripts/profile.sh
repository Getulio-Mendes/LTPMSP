#!/usr/bin/env bash
set -e

FLAMEGRAPH=/home/getuio/Facul/TCC/FlameGraph

if [[ -z "${1-}" ]]; then
    echo "Usage: $0 <instance_file> [runs] [output.svg]"
    echo ""
    echo "  instance_file   Path to the .txt instance file (e.g. data/LTPMSP_...txt)"
    echo "  runs            Number of runs to pass to the binary (default: 3)"
    echo "  output.svg      Output flamegraph file (default: flamegraph.svg)"
    exit 1
fi

INSTANCE=$1
RUNS=$2
OUTPUT=${3:-flamegraph.svg}

make profile
perf record -g -F 99 -o perf.data -- ./manut "$INSTANCE" "$RUNS"
perf script | "$FLAMEGRAPH/stackcollapse-perf.pl" | "$FLAMEGRAPH/flamegraph.pl" > "$OUTPUT"

echo "Done: $OUTPUT"
