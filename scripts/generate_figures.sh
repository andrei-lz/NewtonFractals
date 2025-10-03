#!/usr/bin/env bash
set -euo pipefail
mkdir -p run

./newton_fractals --poly z3-1 --size 1920x1080 --max-iters 200 --tol 1e-12 --damping 1.0 --bounds -2 2 -1.5 1.5 --threads 8 --out run/z3
./newton_fractals --poly z5-1 --size 1920x1080 --max-iters 300 --tol 1e-12 --damping 1.0 --bounds -2 2 -2 2 --threads 8 --out run/z5
./newton_fractals --poly z3-2z+2 --size 1920x1080 --max-iters 400 --tol 1e-12 --damping 1.0 --bounds -2.5 2.5 -2.0 2.0 --threads 8 --out run/z3m2zp2

echo "Figures written to ./run"
