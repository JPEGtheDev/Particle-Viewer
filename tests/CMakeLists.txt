# Test executable configuration
add_executable(ParticleViewerTests
    core/CameraTests.cpp
    core/ShaderTests.cpp
    core/ParticleTests.cpp
    core/SettingsIOTests.cpp
    mocks/MockOpenGL.cpp
)

# Link Google Test and required libraries
target_link_libraries(ParticleViewerTests
    gtest
    gtest_main
    ${CMAKE_DL_LIBS}
)

# Include directories for tests
target_include_directories(ParticleViewerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/glad/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
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
