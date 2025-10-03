#!/usr/bin/env bash
set -euo pipefail

: "${IMG:=1536x1536}"
: "${ITERS:=400}"
: "${TOL:=1e-12}"
: "${POLY:=z3-1}"
: "${BOUNDS:="-2 2 -2 2"}"

echo "cores,size,iters,seconds"
for t in 1 2 4 6; do
  # Measure wall-clock seconds with /usr/bin/time
  SEC=$(/usr/bin/time -f "%e" \
    OMP_NUM_THREADS="$t" ./newton_fractals \
      --poly "$POLY" --size "$IMG" --max-iters "$ITERS" --tol "$TOL" \
      --bounds $BOUNDS --threads "$t" --out /tmp/bench \
      >/dev/null 2> /tmp/.time  || true
    cat /tmp/.time)

  echo "$t,$IMG,$ITERS,$SEC"
done
