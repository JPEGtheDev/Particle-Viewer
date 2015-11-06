cmake_minimum_required (VERSION 2.8.11)
project (Particle-Viewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -march=native")

find_package(GLM REQUIRED)
find_package(SDL2 REQUIRED)


add_executable(Viewer
  src/glad/glad.c
  src/main.cpp
)
target_link_libraries(Viewer ${CMAKE_DL_LIBS})

target_include_directories(Viewer PUBLIC
  ${GLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} 
)


target_link_libraries(Viewer
  ${SDL2_LIBRARIES}  ${OPENGL_gl_LIBRARY}
)
