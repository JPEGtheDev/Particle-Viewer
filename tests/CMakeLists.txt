# Test executable configuration
# Automatically discover all test files using GLOB for convenience
# Note: When adding new test files, re-run CMake to detect them:
#   rm -rf build && mkdir build && cd build && cmake ..
file(GLOB TEST_CORE_FILES core/*Tests.cpp)
file(GLOB TEST_MOCK_FILES mocks/*.cpp)
file(GLOB TEST_INTEGRATION_FILES integration/*Tests.cpp)
file(GLOB TEST_TESTING_FILES testing/*Tests.cpp)
file(GLOB TEST_VISUAL_REGRESSION_FILES visual-regression/*Tests.cpp)

# Testing utility sources (PixelComparator in src/testing/, ImageConverter in src/)
file(GLOB TESTING_UTILITY_SOURCES ${CMAKE_SOURCE_DIR}/src/testing/*.cpp)

add_executable(ParticleViewerTests
    ${TEST_CORE_FILES}
    ${TEST_MOCK_FILES}
    ${TEST_INTEGRATION_FILES}
    ${TEST_TESTING_FILES}
    ${TEST_VISUAL_REGRESSION_FILES}
    ${TESTING_UTILITY_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/Image.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/GLFWContext.cpp
    stb_image_write_impl.cpp
)

# Link Google Test and required libraries
target_link_libraries(ParticleViewerTests
    gtest
    gtest_main
    ${CMAKE_DL_LIBS}
    glfw
    OpenGL::GL
)

# Include directories for tests
target_include_directories(ParticleViewerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/glad/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
    ${CMAKE_SOURCE_DIR}/tests
)

# Add glad source for OpenGL function pointers
target_sources(ParticleViewerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/glad/src/glad.c
)

# Discover tests for CTest
include(GoogleTest)
gtest_discover_tests(ParticleViewerTests)

# Add coverage flags if requested
if(${alloutput})
    target_compile_options(ParticleViewerTests PRIVATE
        -g -O0 -fprofile-arcs -ftest-coverage
    )
    target_link_options(ParticleViewerTests PRIVATE
        -fprofile-arcs -ftest-coverage
    )
endif()
