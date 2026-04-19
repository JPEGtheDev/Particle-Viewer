---
title: "Integration Test Patterns"
description: "Test patterns, fixture structure, and how to add new integration tests in Particle-Viewer."
domain: testing
subdomain: integration-testing
tags: [testing, integration-testing, gtest, mocks, c++]
related:
  - "integration-tests.md"
  - "integration-tests-mocking.md"
  - "../TESTING_STANDARDS.md"
---

# Integration Test Patterns

## Test Suite Overview

| Suite | Purpose |
|-------|---------|
| `DataLoadingPipelineTest` | Data flow from binary files through SettingsIO to Particle |
| `ShaderPipelineTest` | Shader loading, compilation, and linking workflow |
| `FramePlaybackTest` | Frame loading, seeking, and playback boundary handling |

## Fixture Structure

```cpp
class MyPipelineTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
        createTestData();
    }

    void TearDown() override { cleanupTestData(); }
};
```

Always call `MockOpenGL::reset()` and `MockOpenGL::initGLAD()` in `SetUp()`. See [mock patterns](integration-tests-mocking.md) for details.

## Patterns by Pipeline

### Data Loading Pipeline

```cpp
TEST_F(DataLoadingPipelineTest, LoadFrame_PopulatesParticleData)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert
    EXPECT_EQ(part.n, expectedParticleCount);
    EXPECT_FLOAT_EQ(part.translations[0].x, expectedX);
}
```

### Frame Boundary

```cpp
TEST_F(FramePlaybackTest, LoadBeyondMax_ClampsAndStopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act
    settings.readPosVelFile(settings.frames + 10, &part, false);

    // Assert
    EXPECT_FALSE(settings.isPlaying);
}
```

### Shader Pipeline

```cpp
TEST_F(ShaderPipelineTest, FullPipeline_CompletesSuccessfully)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(vertexPath, fragmentPath);
    shader.Use();

    // Assert
    EXPECT_EQ(MockOpenGL::createProgramCalls, 1);
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 2);
    EXPECT_GT(shader.Program, 0u);
}
```

## Adding New Tests

1. Place files in `tests/integration/`. Use naming: `{Feature}PipelineTests.cpp` or `{Feature}IntegrationTests.cpp`
2. Inherit from `::testing::Test` and follow the fixture pattern above
3. Write tests using the AAA pattern — never combine Act and Assert
4. If you added new files, re-run CMake:

```bash
rm -rf build && cmake -B build -S . -DBUILD_TESTS=ON && cmake --build build
```

## Running Integration Tests

```bash
# Run all integration test suites
./build/tests/ParticleViewerTests --gtest_filter="*Pipeline*:*Playback*"

# Run a single suite
./build/tests/ParticleViewerTests --gtest_filter="DataLoadingPipelineTest.*"
```

## Related

- [Integration Tests Overview](integration-tests.md) — Running all tests and directory structure
- [Mock Patterns](integration-tests-mocking.md) — MockOpenGL setup and troubleshooting
- [Testing Standards](../TESTING_STANDARDS.md) — AAA pattern and project-wide test guidelines
