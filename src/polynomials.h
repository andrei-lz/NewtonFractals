#pragma once
#include <complex>
#include <memory>
#include <string>
#include <vector>

static const double pi = 3.14159265358979323846;

struct Poly {
  virtual ~Poly() = default;
  virtual std::complex<double> eval(std::complex<double> z) const = 0;
  virtual std::complex<double> deriv(std::complex<double> z) const = 0;
  virtual std::vector<std::complex<double>> roots() const = 0;
  virtual const char *id() const = 0;
};

struct PolyZ3Minus1 : Poly {
  std::complex<double> eval(std::complex<double> z) const override {
    return z * z * z - 1.0;
  }
  std::complex<double> deriv(std::complex<double> z) const override {
    return 3.0 * z * z;
  }
  std::vector<std::complex<double>> roots() const override {
    using cd = std::complex<double>;
    return {cd(1, 0), std::polar(1.0, 2.0 * pi / 3.0),
            std::polar(1.0, -2.0 * pi / 3.0)};
  }
  const char *id() const override { return "z3-1"; }
};

struct PolyZ5Minus1 : Poly {
  std::complex<double> eval(std::complex<double> z) const override {
    return std::pow(z, 5) - 1.0;
  }
  std::complex<double> deriv(std::complex<double> z) const override {
    return 5.0 * std::pow(z, 4);
  }
  std::vector<std::complex<double>> roots() const override {
    using cd = std::complex<double>;
    std::vector<cd> r;
    r.reserve(5);
    for (int k = 0; k < 5; k++)
      r.push_back(std::polar(1.0, 2.0 * pi * k / 5.0));
    return r;
  }
  const char *id() const override { return "z5-1"; }
};

struct PolyZ3Minus2ZPlus2 : Poly {
  std::complex<double> eval(std::complex<double> z) const override {
    return z * z * z - 2.0 * z + 2.0;
  }
  std::complex<double> deriv(std::complex<double> z) const override {
    return 3.0 * z * z - 2.0;
  }
  std::vector<std::complex<double>> roots() const override {
    // Numerical roots (precomputed)
    using cd = std::complex<double>;
    return {cd(-1.7692923542386314, 0.0),
            cd(0.8846461771193157, 0.5897428050222055),
            cd(0.8846461771193157, -0.5897428050222055)};
  }
  const char *id() const override { return "z3-2z+2"; }
};

struct PolyTightClusters : Poly {
  using cd = std::complex<double>;

  // radius of the small 4-point clusters around ±1
  static constexpr double r = 0.12;

  // Precomputed roots: 4 around +1 and 4 around -1, at 90° offsets
  static const std::vector<cd> &rootsList() {
    static const std::vector<cd> R = {
        cd(1.0 + r, 0.0),  // 1 + 0.12
        cd(1.0, r),        // 1 + 0.12i
        cd(1.0 - r, 0.0),  // 1 - 0.12
        cd(1.0, -r),       // 1 - 0.12i
        cd(-1.0 + r, 0.0), // -1 + 0.12
        cd(-1.0, r),       // -1 + 0.12i
        cd(-1.0 - r, 0.0), // -1 - 0.12
        cd(-1.0, -r)       // -1 - 0.12i
    };
    return R;
  }

  std::complex<double> eval(std::complex<double> z) const override {
    cd p(1.0, 0.0);
    for (const cd &rj : rootsList()) {
      p *= (z - rj);
    }
    return p;
  }

  std::complex<double> deriv(std::complex<double> z) const override {
    // p'(z) = p(z) * sum_j 1/(z - r_j)
    cd p = eval(z);
    cd s(0.0, 0.0);
    for (const cd &rj : rootsList()) {
      s += cd(1.0, 0.0) / (z - rj);
    }
    return p * s;
  }

  std::vector<std::complex<double>> roots() const override {
    return rootsList();
  }

  const char *id() const override { return "tight-clusters-archipelagos"; }
};

struct PolyMixedRadiiPentagonStack : Poly {
  using cd = std::complex<double>;

  static const std::vector<cd> &rootsList() {
    static std::vector<cd> R = [] {
      std::vector<cd> v;
      v.reserve(15);
      constexpr double tau = 6.28318530717958647692; // 2π
      constexpr double radii[3] = {1.0, 2.0, 0.5};
      for (double r : radii) {
        for (int k = 0; k < 5; ++k) {
          double theta = tau * k / 5.0;
          v.emplace_back(std::polar(r, theta)); // r * e^{i theta}
        }
      }
      return v;
    }();
    return R;
  }

  std::complex<double> eval(std::complex<double> z) const override {
    cd p(1.0, 0.0);
    for (const cd &rj : rootsList()) {
      p *= (z - rj);
    }
    return p;
  }

  std::complex<double> deriv(std::complex<double> z) const override {
    // p'(z) = p(z) * sum_j 1/(z - r_j)
    cd p = eval(z);
    cd s(0.0, 0.0);
    for (const cd &rj : rootsList()) {
      s += cd(1.0, 0.0) / (z - rj);
    }
    return p * s;
  }

  std::vector<std::complex<double>> roots() const override {
    return rootsList();
  }

  const char *id() const override { return "mixed-radii-pentagon-stack"; }
};

inline std::unique_ptr<Poly> make_poly(const std::string &s) {
  if (s == "z3-1")
    return std::make_unique<PolyZ3Minus1>();
  if (s == "z5-1")
    return std::make_unique<PolyZ5Minus1>();
  if (s == "z3-2z+2")
    return std::make_unique<PolyZ3Minus2ZPlus2>();
  if (s == "tight-clusters-archipelagos")
    return std::make_unique<PolyTightClusters>();
  if (s == "mixed-radii-pentagon-stack")
    return std::make_unique<PolyMixedRadiiPentagonStack>();
  throw std::runtime_error("unknown polynomial id: " + s);
}
