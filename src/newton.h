#pragma once
#include "polynomials.h"
#include <complex>
#include <limits>
#include <tuple>
#include <vector>

struct NewtonParams {
  int max_iters = 100;
  double tol = 1e-10;
  double damping = 1.0; // alpha in (0,1]
};

inline std::tuple<int, int>
newton_iterate(std::complex<double> z0, const Poly &poly,
               const std::vector<std::complex<double>> &roots,
               const NewtonParams &p) {
  using cd = std::complex<double>;
  cd z = z0;
  int k = 0;
  const double tiny = 1e-30;
  for (; k < p.max_iters; ++k) {
    cd f = poly.eval(z);
    cd fp = poly.deriv(z);
    double denom = std::abs(fp);
    if (denom < tiny)
      break; // near critical point
    cd step = p.damping * f / fp;
    cd z1 = z - step;
    if (std::abs(z1 - z) < p.tol || std::abs(f) < p.tol) {
      z = z1;
      ++k;
      break;
    }
    z = z1;
  }
  // Choose nearest root if close; else -1
  int rid = -1;
  double best = std::numeric_limits<double>::infinity();
  for (int i = 0; i < (int)roots.size(); ++i) {
    double d = std::abs(z - roots[(size_t)i]);
    if (d < best) {
      best = d;
      rid = i;
    }
  }
  if (best > 1e-5)
    rid = -1;
  return {rid, k};
}
