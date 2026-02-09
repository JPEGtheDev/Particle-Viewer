# Integration Tests Guide

This document provides guidance on running, adding, and troubleshooting integration tests for the Particle-Viewer project.

## Table of Contents

1. [Overview](#overview)
2. [Running Integration Tests](#running-integration-tests)
3. [Test Organization](#test-organization)
4. [Adding New Integration Tests](#adding-new-integration-tests)
5. [Test Patterns](#test-patterns)
6. [Coverage Considerations](#coverage-considerations)
7. [Troubleshooting](#troubleshooting)

---

## Overview

Integration tests verify that multiple components of the Particle-Viewer work correctly together. Unlike unit tests that test components in isolation, integration tests verify the data flow through the viewer pipeline:

**Viewer Pipeline Integration Points:**
- **Data Loading Pipeline**: SettingsIO reads file → Particle receives translations → VBO updated
- **Shader Pipeline**: Load shader source → compile vertex+fragment → link program
- **Frame Playback**: Load frame N → render → advance → load frame N+1

### Why Integration Tests?

- Catch bugs that only appear when components interact
- Verify the complete data flow from files to rendering
- Ensure frame boundaries are handled correctly
- Validate shader compilation and linking workflow

---

## Running Integration Tests

Integration tests are built and run alongside unit tests.

### Build and Run All Tests

```bash
# Configure and build
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build

# Run all tests (unit + integration)
./build/tests/ParticleViewerTests
```

### Run Integration Tests Only

```bash
# Run only integration test suites
./build/tests/ParticleViewerTests --gtest_filter="*Pipeline*:*Playback*"

# Run specific integration test suite
./build/tests/ParticleViewerTests --gtest_filter="DataLoadingPipelineTest.*"
./build/tests/ParticleViewerTests --gtest_filter="ShaderPipelineTest.*"
./build/tests/ParticleViewerTests --gtest_filter="FramePlaybackTest.*"
```

### Run Specific Test

```bash
./build/tests/ParticleViewerTests --gtest_filter="DataLoadingPipelineTest.LoadFrame0_ParticlePositionsMatchExpected"
```

### Verbose Output

```bash
./build/tests/ParticleViewerTests --gtest_filter="*Pipeline*" --gtest_verbose=1
```

---

## Test Organization

### Directory Structure

```
tests/
├── core/                      # Unit tests for individual classes
│   ├── CameraTests.cpp
│   ├── ParticleTests.cpp
│   ├── SettingsIOTests.cpp
│   └── ShaderTests.cpp
├── integration/               # Integration tests for pipelines
│   ├── DataLoadingPipelineTests.cpp
│   ├── ShaderPipelineTests.cpp
│   └── FramePlaybackTests.cpp
├── mocks/                     # Mock implementations
│   ├── MockOpenGL.hpp
│   └── MockOpenGL.cpp
└── CMakeLists.txt
```

### Test Suite Naming

| Suite Name | Purpose |
|------------|---------|
| `DataLoadingPipelineTest` | Tests data flow from binary files through SettingsIO to Particle |
| `ShaderPipelineTest` | Tests shader loading, compilation, and linking workflow |
| `FramePlaybackTest` | Tests frame loading, seeking, and playback boundaries |

---

## Adding New Integration Tests

### Step 1: Choose the Right Location

Place integration tests in `tests/integration/`. Name the file descriptively:
- `{Feature}PipelineTests.cpp` for end-to-end pipeline tests
- `{Feature}IntegrationTests.cpp` for cross-component tests

### Step 2: Create Test Fixture

Integration tests typically need setup and teardown for test data:

```cpp
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include "MockOpenGL.hpp"

class MyPipelineTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
        createTestData();
    }

    void TearDown() override
    {
        cleanupTestData();
    }

    void createTestData()
    {
        // Create test files in /tmp
    }

    void cleanupTestData()
    {
        // Remove test files
    }
};
```

### Step 3: Write Tests Following AAA Pattern

```cpp
TEST_F(MyPipelineTest, ComponentA_FlowsTo_ComponentB)
{
    // Arrange: Set up initial state
    ComponentA source;
    ComponentB target;

    // Act: Trigger the pipeline
    source.process();
    target.receive(source.output());

    // Assert: Verify the end result
    EXPECT_EQ(target.getData(), expectedValue);
}
```

### Step 4: Update CMakeLists.txt

If you added new test files, re-run CMake:

```bash
rm -rf build && cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
```

---

## Test Patterns

### Data Loading Pipeline Test Pattern

```cpp
TEST_F(DataLoadingPipelineTest, LoadFrame_PopulatesParticleData)
{
    // Arrange: Create SettingsIO with test binary file
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load a specific frame
    settings.readPosVelFile(0, &part, false);

    // Assert: Particle data matches expected
    EXPECT_EQ(part.n, expectedParticleCount);
    EXPECT_FLOAT_EQ(part.translations[0].x, expectedX);
}
```

### Frame Boundary Test Pattern

```cpp
TEST_F(FramePlaybackTest, LoadBeyondMax_ClampsAndStopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Request frame beyond available
    settings.readPosVelFile(settings.frames + 10, &part, false);

    // Assert: Should clamp and stop playing
    EXPECT_FALSE(settings.isPlaying);
}
```

### Shader Pipeline Test Pattern

```cpp
TEST_F(ShaderPipelineTest, FullPipeline_CompletesSuccessfully)
{
    // Arrange: Set up mock for successful compilation
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act: Full pipeline from file to shader use
    Shader shader(vertexPath, fragmentPath);
    shader.Use();

    // Assert: All pipeline stages completed
    EXPECT_EQ(MockOpenGL::createProgramCalls, 1);
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 2);
    EXPECT_EQ(MockOpenGL::linkProgramCalls, 1);
    EXPECT_GT(shader.Program, 0u);
}
```

---

## Coverage Considerations

### Integration Tests and Code Coverage

**Important:** Integration tests should NOT be counted towards code coverage metrics post-refactor.

**Rationale:**
- Integration tests exercise code paths across multiple components and may artificially inflate coverage metrics
- Code coverage should measure how well unit tests exercise individual components
- Counting integration tests can mask gaps in unit test coverage
- Integration tests serve a different purpose: verifying component interactions, not testing individual code paths

**Current Status:**
- Currently, all tests (unit + integration) contribute to coverage reports
- TODO: Implement separate coverage reporting that excludes integration tests

**Future Improvement:**
- Consider migrating to SonarQube for more sophisticated coverage analysis
- Separate coverage reports for unit tests vs integration tests
- Integration test execution time tracking

---

## Troubleshooting

### Common Issues

#### Test Files Not Found

**Problem**: Tests fail because test data files don't exist.

**Solution**: Check that `SetUp()` creates files in `/tmp/` and `TearDown()` cleans them up. Verify file paths are correct.

#### OpenGL Mock Not Initialized

**Problem**: Tests crash with segfault when calling OpenGL functions.

**Solution**: Ensure `MockOpenGL::initGLAD()` is called in `SetUp()`:

```cpp
void SetUp() override
{
    MockOpenGL::reset();
    MockOpenGL::initGLAD();  // Don't forget this!
}
```

#### Static Const Linker Errors

**Problem**: Undefined reference to static const members.

**Solution**: Use enum instead of static const for C++11 compatibility:

```cpp
// Instead of:
static const int NUM_PARTICLES = 50;

// Use:
enum { NUM_PARTICLES = 50 };
```

#### Test Data Corruption

**Problem**: Tests pass individually but fail when run together.

**Solution**: Ensure each test fixture creates fresh test data and cleans up properly. Don't share state between tests.

#### Coverage Not Showing Integration Tests

**Problem**: Integration test files don't appear in coverage report.

**Solution**: The tests themselves don't need coverage - they test YOUR source code in `src/`. Coverage should measure `src/*.hpp` and `src/*.cpp`.

### Debugging Tips

1. **Run single test**: Isolate the failing test with `--gtest_filter`
2. **Check test data**: Add debug output to `createTestData()` to verify file creation
3. **Mock state**: Add `MockOpenGL::reset()` at the start of each test
4. **Verbose output**: Use `--gtest_verbose=1` to see test execution details

### Getting Help

If you encounter issues not covered here:
1. Check existing tests for patterns
2. Review `docs/TESTING_STANDARDS.md` for testing guidelines
3. Open an issue on GitHub with test output and reproduction steps

---

## Related Documentation

- [Testing Standards](../TESTING_STANDARDS.md) - Unit test guidelines and best practices
- [Coding Standards](../CODING_STANDARDS.md) - Code style and naming conventions
- [README](../../README.md) - Project overview and build instructions
