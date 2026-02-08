cmake_minimum_required (VERSION 3.18)

# Accept version as CMake variable, or use git tags as fallback
if(NOT DEFINED PROJECT_VERSION)
    execute_process(
        COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Remove 'v' prefix if present and use as project version
    if(GIT_VERSION)
        string(REGEX REPLACE "^v" "" PROJECT_VERSION "${GIT_VERSION}")
    else()
        # Fallback version if no tags exist
        set(PROJECT_VERSION "0.0.0")
    endif()
endif()

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

# Install desktop and metainfo files for Flatpak
if(EXISTS "${CMAKE_SOURCE_DIR}/flatpak/org.particleviewer.ParticleViewer.desktop")
    install(FILES flatpak/org.particleviewer.ParticleViewer.desktop 
            DESTINATION share/applications)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/flatpak/org.particleviewer.ParticleViewer.metainfo.xml")
    install(FILES flatpak/org.particleviewer.ParticleViewer.metainfo.xml 
            DESTINATION share/metainfo)
endif()

# Install application icon for Flatpak
if(EXISTS "${CMAKE_SOURCE_DIR}/flatpak/org.particleviewer.ParticleViewer.svg")
    install(FILES flatpak/org.particleviewer.ParticleViewer.svg 
            DESTINATION share/icons/hicolor/scalable/apps)
endif()

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
