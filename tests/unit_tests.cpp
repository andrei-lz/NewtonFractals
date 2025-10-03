#include "../src/image.h"
#include "../src/newton.h"
#include "../src/polynomials.h"
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <string>
#include <vector>

static bool approx_eq(std::complex<double> a, std::complex<double> b,
                      double eps = 1e-8) {
  return std::abs(a - b) < eps;
}

int test_roots() {
  int fails = 0;
  {
    PolyZ3Minus1 p;
    auto r = p.roots();
    for (auto z0 : r) {
      NewtonParams np;
      np.max_iters = 50;
      np.tol = 1e-14;
      np.damping = 1.0;
      auto [rid, k] =
          newton_iterate(z0 + std::complex<double>(1e-3, -1e-3), p, r, np);
      if (rid < 0) {
        std::fprintf(stderr, "z^3-1 did not converge\n");
        ++fails;
      }
    }
  }
  {
    PolyZ5Minus1 p;
    auto r = p.roots();
    NewtonParams np;
    np.max_iters = 80;
    np.tol = 1e-13;
    auto [rid, k] = newton_iterate(std::complex<double>(0.5, 0.6), p, r, np);
    if (rid < 0) {
      std::fprintf(stderr, "z^5-1 did not converge from (0.5,0.6)\n");
      ++fails;
    }
  }
  {
    PolyZ3Minus2ZPlus2 p;
    auto r = p.roots();
    NewtonParams np;
    np.max_iters = 100;
    np.tol = 1e-12;
    auto [rid, k] = newton_iterate(std::complex<double>(-2.0, 0.3), p, r, np);
    if (rid < 0) {
      std::fprintf(stderr, "z^3-2z+2 did not converge\n");
      ++fails;
    }
  }
  return fails;
}

// Golden image checksum for determinism
int test_golden() {
  const int W = 256, H = 256;
  PolyZ3Minus1 p;
  auto roots = p.roots();
  NewtonParams np;
  np.max_iters = 100;
  np.tol = 1e-12;
  np.damping = 1.0;
  ImageRGBA bas(W, H), iters(W, H);
  const double xmin = -2, xmax = 2, ymin = -2, ymax = 2;
  const double dx = (xmax - xmin) / double(W), dy = (ymax - ymin) / double(H);

  auto colors =
      make_basin_palette((int)roots.size(), BasinPalette::AngleHue, &roots);
  RGBA no_conv{0, 0, 0, 255};
  int maxk = 1;
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      std::complex<double> z0(xmin + (x + 0.5) * dx, ymin + (y + 0.5) * dy);
      auto [rid, k] = newton_iterate(z0, p, roots, np);
      if (k > maxk)
        maxk = k;
      bas.at(x, y) = (rid >= 0) ? colors[rid] : no_conv;
      unsigned char g = (unsigned char)(k < 255 ? k : 255);
      iters.at(x, y) = RGBA{g, g, g, 255};
    }
  }
  // checksum
  unsigned long long sum = 0;
  for (const auto &px : bas.pixels) {
    sum = sum * 1315423911ull + px.r * 3 + px.g * 5 + px.b * 7 + px.a;
  }
  const unsigned long long GOLD =
      16134397293084637927ull; // update if algorithm changes intentionally
  if (sum != GOLD) {
    std::fprintf(stderr, "Golden checksum mismatch: got %llu\n",
                 (unsigned long long)sum);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc > 1 && std::string(argv[1]) == "--roots") {
    return test_roots();
  } else if (argc > 1 && std::string(argv[1]) == "--golden") {
    return test_golden();
  } else {
    std::puts("Usage: unit_tests --roots | --golden");
    return 0;
  }
}
