cmake_minimum_required (VERSION 2.8.11)
project (Particle-Viewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")
file(GLOB DIALOG src/tinyFileDialogs/*.c)
file(GLOB srcCPP src/*.cpp)
file(GLOB srcHPP src/*.hpp)
find_package(SDL2 REQUIRED)
add_executable(Viewer
  ${DIALOG}
  ${srcCPP}
  ${srcHPP}
  external/glad/src/glad.c

)
target_link_libraries(Viewer ${CMAKE_DL_LIBS})

target_include_directories(Viewer PUBLIC
	external
	external/glm
	external/glad/include
	${SDL2_INCLUDE_DIR}
)

target_link_libraries(Viewer
  ${SDL2_LIBRARY}
)
file(COPY src/shaders/ DESTINATION Viewer-Assets/shaders)
install (TARGETS Viewer DESTINATION bin)
install (DIRECTORY src/shaders/ DESTINATION bin/Viewer-Assets/shaders)