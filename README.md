# Newton Fractals (C++20, OpenMP)

A compact but rigorous **Newton fractal** renderer intended as a showcase for
HPC-aware scientific software engineering (Cambridge MPhil level).

- **Numerics**: damping, convergence criteria, tolerance study.
- **Validation**: deterministic unit tests vs known roots + golden image checksum.
- **Performance**: OpenMP scaling harness, scheduling options, cache notes, and SIMD.
- **Reproducibility**: Bash (Debian), scripts to reproduce figures/benchmarks, release-friendly layout.
- **Optional viewer**: minimal ImGui+GLFW interactive viewer to explore basins & iteration heatmaps.

<img width="801" height="1036" alt="7A0CDA9B-75A1-417A-A3D8-9CDCF5265409" src="https://github.com/user-attachments/assets/2f5c20b9-29cc-4dbf-ac19-90fab4daadf8" />

## Quick Start

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./newton_fractals --poly z3-1 --size 1920x1080 --max-iters 200 --tol 1e-12 \
                  --damping 1.0 --bounds -2 2 -1.5 1.5 --threads 8 --out run/z3
