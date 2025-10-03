#include "image.h"
#include <algorithm>
#include <array>
#include <numbers>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static inline uint8_t clamp8(int v) {
  return (uint8_t)std::min(255, std::max(0, v));
}

RGBA make_rgba(int r, int g, int b, int a) {
  return RGBA{clamp8(r), clamp8(g), clamp8(b), clamp8(a)};
}

bool ImageRGBA::save_png(const std::string &path) const {
  if (width <= 0 || height <= 0 || pixels.size() != (size_t)width * height)
    return false;
  return stbi_write_png(path.c_str(), width, height, 4, pixels.data(),
                        width * 4) != 0;
}

// Turbo colormap approximation (Google's Turbo)
RGBA turbo_colormap(double x) {
  x = std::clamp(x, 0.0, 1.0);
  // Simple polynomial approx (coarse but OK for visualization)
  double r = std::clamp(1.0 + 0.0 * x - 3.0 * (x - 0.5) * (x - 0.5), 0.0, 1.0);
  double g = std::clamp(1.2 * x * (1.0 - x) * 4.0, 0.0, 1.0);
  double b = std::clamp(1.0 - x + 0.3 * std::sin(6.28318530718 * x), 0.0, 1.0);
  return RGBA{(uint8_t)(255 * r), (uint8_t)(255 * g), (uint8_t)(255 * b), 255};
}

RGBA label_color(int label) {
  static const std::array<RGBA, 12> tab = {
      RGBA{230, 25, 75, 255},  RGBA{60, 180, 75, 255},
      RGBA{255, 225, 25, 255}, RGBA{0, 130, 200, 255},
      RGBA{245, 130, 48, 255}, RGBA{145, 30, 180, 255},
      RGBA{70, 240, 240, 255}, RGBA{240, 50, 230, 255},
      RGBA{210, 245, 60, 255}, RGBA{250, 190, 190, 255},
      RGBA{0, 128, 128, 255},  RGBA{230, 190, 255, 255}};
  return tab[(size_t)label % tab.size()];
}

static RGBA hsv(double h, double s, double v) {
  h = std::fmod(std::max(0.0, h), 360.0) / 60.0;
  int i = (int)std::floor(h);
  double f = h - i;
  double p = v * (1.0 - s);
  double q = v * (1.0 - s * f);
  double t = v * (1.0 - s * (1.0 - f));
  double r, g, b;
  switch (i) {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  default:
    r = v;
    g = p;
    b = q;
    break;
  }
  return RGBA{(uint8_t)(255 * r), (uint8_t)(255 * g), (uint8_t)(255 * b), 255};
}

std::vector<RGBA>
make_basin_palette(int N, BasinPalette pal,
                   const std::vector<std::complex<double>> *roots) {
  std::vector<RGBA> out;
  out.reserve((size_t)N);
  if (pal == BasinPalette::AngleHue && roots) {
    for (int i = 0; i < N; i++) {
      double ang = std::arg((*roots)[(size_t)i]); // [-pi,pi]
      double deg = (ang * 180.0 / std::numbers::pi_v<double> + 360.0);
      out.push_back(hsv(std::fmod(deg, 360.0), 0.85, 0.95));
    }
    return out;
  }
  if (pal == BasinPalette::BlueGold) {
    for (int i = 0; i < N; i++) {
      double t = (double)i / std::max(1, N - 1);
      int r = (int)std::round(30 + 200 * t);
      int g = (int)std::round(80 + 140 * t);
      int b = (int)std::round(200 - 120 * t);
      out.push_back(make_rgba(r, g, b, 255));
    }
    return out;
  }
  if (pal == BasinPalette::ColorblindSafe) {
    static const std::array<RGBA, 8> tol = {
        make_rgba(68, 119, 170),  make_rgba(102, 204, 238),
        make_rgba(34, 136, 51),   make_rgba(204, 187, 68),
        make_rgba(238, 102, 119), make_rgba(170, 51, 119),
        make_rgba(187, 187, 187), make_rgba(51, 34, 136)};
    for (int i = 0; i < N; i++)
      out.push_back(tol[(size_t)i % tol.size()]);
    return out;
  }
  // Pastel
  for (int i = 0; i < N; i++) {
    double t = (double)i / std::max(1, N - 1);
    out.push_back(make_rgba((int)(200 - 40 * t), (int)(180 + 30 * t),
                            (int)(220 - 60 * t), 255));
  }
  return out;
}
