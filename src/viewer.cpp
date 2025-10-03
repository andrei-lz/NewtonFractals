#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <string>
#include <vector>

#include "image.h"
#include "newton.h"
#include "polynomials.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
// ImGui will be provided by CMake FetchContent when BUILD_VIEWER=ON
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct State {
  int W = 1024, H = 768;
  double xmin = -2, xmax = 2, ymin = -1.5, ymax = 1.5;
  int max_iters = 300;
  double tol = 1e-12, damping = 1.0;
  std::string poly_id = "z3-1";
  bool dirty = true;
};

static GLuint make_texture_from(ImageRGBA &img) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, img.pixels.data());
  return tex;
}

static void compute_images(State &S, ImageRGBA &basin, ImageRGBA &iters) {
  std::unique_ptr<Poly> poly(make_poly(S.poly_id));
  auto roots = poly->roots();
  NewtonParams np;
  np.max_iters = S.max_iters;
  np.tol = S.tol;
  np.damping = S.damping;

  basin = ImageRGBA(S.W, S.H);
  iters = ImageRGBA(S.W, S.H);
  const double dx = (S.xmax - S.xmin) / double(S.W);
  const double dy = (S.ymax - S.ymin) / double(S.H);

  auto colors =
      make_basin_palette((int)roots.size(), BasinPalette::BlueGold, &roots);
  RGBA no_conv{0, 0, 0, 255};
  int max_k = 1;
#pragma omp parallel for schedule(static) reduction(max : max_k)
  for (int y = 0; y < S.H; y++) {
    for (int x = 0; x < S.W; x++) {
      std::complex<double> z0(S.xmin + (x + 0.5) * dx, S.ymin + (y + 0.5) * dy);
      auto [rid, k] = newton_iterate(z0, *poly, roots, np);
      if (k > max_k)
        max_k = k;
      basin.at(x, y) = (rid >= 0) ? colors[rid] : no_conv;
      unsigned char g = (unsigned char)(k < 255 ? k : 255);
      iters.at(x, y) = RGBA{g, g, g, 255};
    }
  }
  for (int y = 0; y < S.H; y++) {
    for (int x = 0; x < S.W; x++) {
      double t = iters.at(x, y).r / double(max_k);
      iters.at(x, y) = turbo_colormap(t);
    }
  }
}

int main() {
  if (!glfwInit()) {
    std::puts("GLFW init failed");
    return 1;
  }
  GLFWwindow *win =
      glfwCreateWindow(1200, 900, "Newton Viewer", nullptr, nullptr);
  if (!win) {
    std::puts("GLFW window failed");
    return 1;
  }
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(win, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  State S;
  ImageRGBA basin, iters;
  GLuint tex_basin = 0, tex_iters = 0;

  while (!glfwWindowShouldClose(win)) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Controls");
    ImGui::InputInt("Width", &S.W);
    ImGui::InputInt("Height", &S.H);
    ImGui::InputInt("Max iters", &S.max_iters);
    ImGui::InputDouble("tol", &S.tol);
    ImGui::InputDouble("damping", &S.damping);
    static int poly_idx = 0;
    const char *polys[] = {"z3-1", "z5-1", "z3-2z+2",
                           "tight-clusters-archipelagos",
                           "mixed-radii-pentagon-stack"};
    if (ImGui::Combo("poly", &poly_idx, polys, IM_ARRAYSIZE(polys))) {
      S.poly_id = polys[poly_idx];
      S.dirty = true;
    }
    ImGui::InputDouble("xmin", &S.xmin);
    ImGui::InputDouble("xmax", &S.xmax);
    ImGui::InputDouble("ymin", &S.ymin);
    ImGui::InputDouble("ymax", &S.ymax);
    if (ImGui::Button("Render"))
      S.dirty = true;
    ImGui::SameLine();
    if (ImGui::Button("Save PNGs")) {
      basin.save_png("viewer_basins.png");
      iters.save_png("viewer_iters.png");
    }
    ImGui::End();

    if (S.dirty) {
      compute_images(S, basin, iters);
      if (tex_basin)
        glDeleteTextures(1, &tex_basin);
      if (tex_iters)
        glDeleteTextures(1, &tex_iters);
      tex_basin = make_texture_from(basin);
      tex_iters = make_texture_from(iters);
      S.dirty = false;
    }

    int ww, hh;
    glfwGetFramebufferSize(win, &ww, &hh);
    glViewport(0, 0, ww, hh);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Begin("Images");
    ImGui::Text("Basins");
    ImGui::Image((void *)(intptr_t)tex_basin, ImVec2(512, 512));
    ImGui::SameLine();
    ImGui::Text("Iterations");
    ImGui::Image((void *)(intptr_t)tex_iters, ImVec2(512, 512));
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(win);
  }

  if (tex_basin)
    glDeleteTextures(1, &tex_basin);
  if (tex_iters)
    glDeleteTextures(1, &tex_iters);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(win);
  glfwTerminate();
  return 0;
}
