cmake_minimum_required(VERSION 3.20)
project(NewtonFractals LANGUAGES CXX)

option(BUILD_VIEWER "Build optional OpenGL/ImGui viewer (requires GLFW/OpenGL)" OFF)
option(ENABLE_SIMD "Enable simple SIMD path" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings
if (MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)
endif()

# OpenMP (optional)
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  message(STATUS "OpenMP found: enabling parallel loops")
endif()

# stb_image_write (vendored minimal header)
add_library(stb_image_write INTERFACE)
target_include_directories(stb_image_write INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

# Sources
add_executable(newton_fractals
  src/main.cpp
  src/image.cpp
)
target_include_directories(newton_fractals PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(newton_fractals PRIVATE stb_image_write)
if (OpenMP_CXX_FOUND)
  target_link_libraries(newton_fractals PRIVATE OpenMP::OpenMP_CXX)
  target_compile_definitions(newton_fractals PRIVATE HAVE_OPENMP=1)
endif()
if (ENABLE_SIMD)
  target_compile_definitions(newton_fractals PRIVATE USE_SIMD=1)
endif()

# Optional viewer (GLFW + OpenGL + ImGui via FetchContent)
if (BUILD_VIEWER)
  include(FetchContent)
  FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
  FetchContent_MakeAvailable(glfw)

  FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.0)
  FetchContent_MakeAvailable(imgui)

  add_library(imgui_glfw_opengl3
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  )
  
  target_include_directories(imgui_glfw_opengl3 PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
  find_package(OpenGL REQUIRED)
  target_link_libraries(imgui_glfw_opengl3 PUBLIC glfw OpenGL::GL)

  add_executable(newton_viewer src/viewer.cpp src/image.cpp)
  target_include_directories(newton_viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(newton_viewer PRIVATE imgui_glfw_opengl3 stb_image_write)
  if (OpenMP_CXX_FOUND)
    target_link_libraries(newton_viewer PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(newton_viewer PRIVATE HAVE_OPENMP=1)
  endif()
endif()

# ---------- Tests ----------
enable_testing()
add_executable(unit_tests tests/unit_tests.cpp src/image.cpp)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(unit_tests PRIVATE stb_image_write)
if (OpenMP_CXX_FOUND)
  target_link_libraries(unit_tests PRIVATE OpenMP::OpenMP_CXX)
  target_compile_definitions(unit_tests PRIVATE HAVE_OPENMP=1)
endif()
add_test(NAME roots_converge COMMAND unit_tests --roots)
add_test(NAME golden_image COMMAND unit_tests --golden)

# --- MSVC per-target tweaks (add after targets are defined) ---
if (MSVC)
  foreach(tgt newton_fractals unit_tests)
    if (TARGET ${tgt})
      target_compile_definitions(${tgt} PRIVATE _CRT_SECURE_NO_WARNINGS)
      target_compile_options   (${tgt} PRIVATE /openmp:llvm)
    endif()
  endforeach()

  if (BUILD_VIEWER AND TARGET newton_viewer)
    target_compile_definitions(newton_viewer PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options   (newton_viewer PRIVATE /openmp:llvm)
  endif()
endif()
