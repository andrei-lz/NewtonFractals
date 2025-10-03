# Newton Fractals (C++20, OpenMP, Research-Grade)

A compact but rigorous **Newton fractal** renderer intended as a showcase for
HPC-aware scientific software engineering (Cambridge MPhil level).

- **Numerics**: damping, convergence criteria, tolerance study.
- **Validation**: deterministic unit tests vs known roots + golden image checksum.
- **Performance**: OpenMP scaling harness, scheduling options, cache notes, and a SIMD toggle.
- **Reproducibility**: CI (Ubuntu), scripts to reproduce figures/benchmarks, release-friendly layout.
- **Optional viewer**: minimal ImGui+GLFW interactive viewer to explore basins & iteration heatmaps.

![Fractal Generation Viewer]({7A0CDA9B-75A1-417A-A3D8-9CDCF5265409}.png)

## Quick Start

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./newton_fractals --poly z3-1 --size 1920x1080 --max-iters 200 --tol 1e-12 \
                  --damping 1.0 --bounds -2 2 -1.5 1.5 --threads 8 --out run/z3
