#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

#include "image.h"
#include "newton.h"
#include "polynomials.h"
#include "timing.h"

#if defined(HAVE_OPENMP) || defined(_OPENMP)
#include <omp.h>
#endif

struct Args {
  std::string poly = "z3-1";
  int W = 1024, H = 768;
  int max_iters = 300;
  double tol = 1e-12, damping = 1.0;
  double xmin = -2, xmax = 2, ymin = -1.5, ymax = 1.5;
  int threads = 0;
  std::string out_prefix = "run/out";
};

static void usage() {
  std::puts("newton_fractals\n"
            "  --poly ID           (z3-1 | z5-1 | z3-2z+2)\n"
            "  --size WxH          (default 1024x768)\n"
            "  --max-iters N       (default 300)\n"
            "  --tol EPS           (default 1e-12)\n"
            "  --damping A         (default 1.0)\n"
            "  --bounds xmin xmax ymin ymax\n"
            "  --threads T         (0=auto)\n"
            "  --out PREFIX        (default run/out)\n");
}

static bool parse_size(const std::string &s, int &W, int &H) {
  auto x = s.find('x');
  if (x == std::string::npos)
    return false;
  W = std::stoi(s.substr(0, x));
  H = std::stoi(s.substr(x + 1));
  return W > 0 && H > 0;
}

int main(int argc, char **argv) {
  Args a;
  for (int i = 1; i < argc; i++) {
    std::string k = argv[i];
    auto need = [&](int more) -> const char * {
      if (i + more >= argc) {
        usage();
        std::exit(1);
      }
      return argv[++i];
    };
    if (k == "--poly")
      a.poly = need(1);
    else if (k == "--size") {
      std::string s = need(1);
      if (!parse_size(s, a.W, a.H)) {
        usage();
        return 1;
      }
    } else if (k == "--max-iters")
      a.max_iters = std::atoi(need(1));
    else if (k == "--tol")
      a.tol = std::atof(need(1));
    else if (k == "--damping")
      a.damping = std::atof(need(1));
    else if (k == "--bounds") {
      a.xmin = std::atof(need(1));
      a.xmax = std::atof(need(1));
      a.ymin = std::atof(need(1));
      a.ymax = std::atof(need(1));
    } else if (k == "--threads")
      a.threads = std::atoi(need(1));
    else if (k == "--out")
      a.out_prefix = need(1);
    else {
      usage();
      return 1;
    }
  }

  if (a.threads > 0) {
#ifdef HAVE_OPENMP
    omp_set_num_threads(a.threads);
#endif
  }

  auto poly = make_poly(a.poly);
  auto roots = poly->roots();

  ImageRGBA bas(a.W, a.H), iters(a.W, a.H);
  const double dx = (a.xmax - a.xmin) / double(a.W),
               dy = (a.ymax - a.ymin) / double(a.H);

  auto colors =
      make_basin_palette((int)roots.size(), BasinPalette::Pastel, &roots);
  RGBA no_conv{0, 0, 0, 255};

  NewtonParams np;
  np.max_iters = a.max_iters;
  np.tol = a.tol;
  np.damping = a.damping;

  Timer t;
  int maxk = 1;
#pragma omp parallel for schedule(static) reduction(max : maxk)
  for (int y = 0; y < a.H; y++) {
    for (int x = 0; x < a.W; x++) {
      std::complex<double> z0(a.xmin + (x + 0.5) * dx, a.ymin + (y + 0.5) * dy);
      auto [rid, k] = newton_iterate(z0, *poly, roots, np);
      if (k > maxk)
        maxk = k;
      bas.at(x, y) = (rid >= 0) ? colors[rid] : no_conv;
      unsigned char g = (unsigned char)(k < 255 ? k : 255);
      iters.at(x, y) = RGBA{g, g, g, 255};
    }
  }
  double secs = t.seconds();
  std::printf("Computed in %.6f seconds for %dx%d, max_iters=%d\n", secs, a.W,
              a.H, a.max_iters);

  // map iterations to turbo
  for (int y = 0; y < a.H; y++) {
    for (int x = 0; x < a.W; x++) {
      double v = iters.at(x, y).r / double(maxk);
      iters.at(x, y) = turbo_colormap(v);
    }
  }

  std::string out_b = a.out_prefix + "_basins.png";
  std::string out_i = a.out_prefix + "_iters.png";
  bas.save_png(out_b);
  iters.save_png(out_i);
  std::printf("Wrote %s and %s\n", out_b.c_str(), out_i.c_str());
  return 0;
}
