#!/usr/bin/env bash
set -euo pipefail
: "${IMG:=1536x1536}"
: "${ITERS:=400}"
: "${TOL:=1e-12}"
: "${POLY:=z3-1}"
: "${BOUNDS:="-2 2 -2 2"}"

echo "cores,size,iters,seconds"
for t in 6 4 2 1; do
  OMP_NUM_THREADS=$t ./newton_fractals --poly "${POLY}" --size "${IMG}" --max-iters "${ITERS}" --tol "${TOL}" \
      --bounds ${BOUNDS} --threads "${t}" --out /tmp/bench >/tmp/runlog 2>&1 || true

  # Extract the "Computed in Xs" time; strip the trailing 's'
  awk -v c="$t" -v img="$IMG" -v iters="$ITERS" '
    /Computed in/ {
      sec=$3; sub(/s$/,"",sec);
      print c "," img "," iters "," sec
    }
  ' /tmp/runlog
done
