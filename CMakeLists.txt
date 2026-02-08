cmake_minimum_required (VERSION 3.18)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)

project (Particle-Viewer VERSION ${PROJECT_VERSION})
option(alloutput off)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

if(${alloutput})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
    # some more commands
endif()
message("CXX Flags:" ${CMAKE_CXX_FLAGS})

file(GLOB DIALOG src/tinyFileDialogs/*.c)
file(GLOB srcCPP src/*.cpp)
file(GLOB srcHPP src/*.hpp)
add_executable(Viewer
	${DIALOG}
	${srcCPP}
	${srcHPP}
	src/glad/src/glad.c
	src/glad/include/glad/glad.h
	src/glad/include/KHR/khrplatform.h
)   

target_link_libraries(Viewer ${CMAKE_DL_LIBS} glfw OpenGL::GL)

target_include_directories(Viewer PUBLIC
	src/glad/include
)

if(WIN32)
  file(COPY src/shaders/ DESTINATION Debug/Viewer-Assets/shaders)
  file(COPY src/shaders/ DESTINATION Release/Viewer-Assets/shaders)
endif()

file(COPY src/shaders/ DESTINATION Viewer-Assets/shaders)
install (TARGETS Viewer DESTINATION bin)
install (DIRECTORY src/shaders/ DESTINATION bin/Viewer-Assets/shaders)

# ============================================
# Testing Configuration
# ============================================
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Download and configure GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Add test subdirectory
    add_subdirectory(tests)
endif()
