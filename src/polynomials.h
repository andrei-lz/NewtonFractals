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

inline std::unique_ptr<Poly> make_poly(const std::string &s) {
  if (s == "z3-1")
    return std::make_unique<PolyZ3Minus1>();
  if (s == "z5-1")
    return std::make_unique<PolyZ5Minus1>();
  if (s == "z3-2z+2")
    return std::make_unique<PolyZ3Minus2ZPlus2>();
  throw std::runtime_error("unknown polynomial id: " + s);
}
