#!/usr/bin/env bash
set -euo pipefail
: "${IMG:=1536x1536}"
: "${ITERS:=400}"
: "${TOL:=1e-12}"
: "${POLY:=z3-1}"
: "${BOUNDS:="-2 2 -2 2"}"

echo "cores,size,iters,seconds"
for t in 1 2 4 8 16; do
  OMP_NUM_THREADS=$t ./newton_fractals --poly ${POLY} --size ${IMG} --max-iters ${ITERS} --tol ${TOL} \
      --bounds ${BOUNDS} --threads ${t} --out /tmp/bench >/tmp/runlog 2>&1 || true
  grep -E "Computed in " /tmp/runlog | awk -v c=$t '{print c",""'${IMG}'","'${ITERS}'","$3}'
done
