cmake_minimum_required (VERSION 2.8.11)
project (Particle-Viewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")

find_package(GLM REQUIRED)
find_package(SDL2 REQUIRED)
find_package(assimp REQUIRED)
add_executable(Viewer
  src/glad/glad.c
  src/glad/glad.h
  src/main.cpp
  src/clutter.hpp
  src/shader.hpp
  src/shader.cpp
  src/stb/stb_image.h
  src/camera.hpp
  src/camera.cpp
  src/mesh.hpp
  src/mesh.cpp
  src/model.hpp
  src/model.cpp
)
target_link_libraries(Viewer ${CMAKE_DL_LIBS})

target_include_directories(Viewer PUBLIC
  ${GLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${ASSIMP_INCLUDE_DIR}
)


target_link_libraries(Viewer
  ${SDL2_LIBRARY} ${ASSIMP_LIBRARY}
)
