cmake_minimum_required (VERSION 2.8.12)
project (Particle-Viewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")


if(DEFINED ENV{TRAVIS_OS_NAME})
    SET(CMAKE_CXX_FLAGS="-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage -std=c++11 -march=native")
    SET(CMAKE_C_FLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
    # some more commands
endif()

message("CXX Flags:" $ENV{CMAKE_CXX_FLAGS})


file(GLOB DIALOG src/tinyFileDialogs/*.c)
file(GLOB srcCPP src/*.cpp)
file(GLOB srcHPP src/*.hpp)
add_executable(Viewer
	${DIALOG}
	${srcCPP}
	${srcHPP}
	external/glad/src/glad.c
)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(external/glfw)
target_link_libraries(Viewer ${CMAKE_DL_LIBS} glfw ${GLFW_LIBRARIES}  )

target_include_directories(Viewer PUBLIC
	external
	external/glm
	external/glad/include
	external/rapidjson/include
	external/glfw/include
)
file(COPY src/shaders/ DESTINATION Viewer-Assets/shaders)
install (TARGETS Viewer DESTINATION bin)
install (DIRECTORY src/shaders/ DESTINATION bin/Viewer-Assets/shaders)