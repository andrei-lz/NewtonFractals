#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <complex>

struct RGBA {
    uint8_t r,g,b,a;
};

enum class BasinPalette {
    AngleHue,       // color by angle of root in complex plane
    BlueGold,       // strong blue/gold pairing
    ColorblindSafe, // Paul Tol's bright palette
    Pastel          // subtle pastels
};

struct ImageRGBA {
    int width=0, height=0;
    std::vector<RGBA> pixels;
    ImageRGBA() = default;
    ImageRGBA(int w,int h):width(w),height(h),pixels((size_t)w*h) {}
    RGBA& at(int x,int y){ return pixels[(size_t)y*width + x]; }
    const RGBA& at(int x,int y) const { return pixels[(size_t)y*width + x]; }
    bool save_png(const std::string& path) const; // implemented in image.cpp
};

// Palettes implemented in image.cpp
std::vector<RGBA> make_basin_palette(int N, BasinPalette pal,
                                     const std::vector<std::complex<double>>* roots=nullptr);

RGBA make_rgba(int r,int g,int b,int a=255);
RGBA turbo_colormap(double x);
RGBA label_color(int label);
