# Particle-Viewer Testing Standards

This document defines the testing standards and best practices for the Particle-Viewer project. These standards ensure that tests are maintainable, reliable, and provide meaningful feedback when failures occur.

## Table of Contents

1. [Testing Philosophy](#testing-philosophy)
2. [Test Structure: Arrange-Act-Assert](#test-structure-arrange-act-assert)
3. [Single Assertion Principle](#single-assertion-principle)
4. [Test Naming Conventions](#test-naming-conventions)
5. [Test Organization](#test-organization)
6. [Mocking Guidelines](#mocking-guidelines)
7. [Coverage Expectations](#coverage-expectations)
8. [Running Tests](#running-tests)

---

## Testing Philosophy

**Core Principle: A test should only have one reason to fail**

This fundamental principle guides all our testing practices. When a test fails, it should be immediately clear what went wrong and why. This is achieved through:
- Clear test structure (Arrange-Act-Assert)
- Single assertion per test (when pragmatic)
- Descriptive test names
- Focused test scope

**Key Goals:**
- **Reliability**: Tests should pass consistently when code is correct
- **Maintainability**: Tests should be easy to understand and update
- **Speed**: Tests should run quickly to encourage frequent execution
- **Clarity**: Test failures should clearly indicate what broke

---

## Test Structure: Arrange-Act-Assert

All tests should follow the **Arrange-Act-Assert (AAA)** pattern. This pattern creates clear, readable tests with distinct phases:

### Arrange
Set up test preconditions and inputs:
- Create test objects
- Initialize test data
- Configure mocks
- Set up expected values

### Act
Execute the code under test:
- Call the method being tested
- Perform the operation
- Trigger the behavior

### Assert
Verify the outcome:
- Check return values
- Verify state changes
- Confirm side effects

### Example

```cpp
TEST(CameraTest, SetupCamReturnsViewMatrix)
{
    // Arrange
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.cameraPos = glm::vec3(1.0f, 2.0f, 3.0f);
    camera.cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Act
    glm::mat4 viewMatrix = camera.setupCam();

    // Assert
    glm::mat4 expected = glm::lookAt(
        glm::vec3(1.0f, 2.0f, 3.0f),
        glm::vec3(1.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    EXPECT_EQ(viewMatrix, expected);
}
```

**Visual Separation**: Use blank lines or comments to clearly separate the three phases in longer tests.

---

## Single Assertion Principle

**Pragmatic Approach**: Each test should verify one logical concept, which may require multiple assertions when they all relate to the same behavior.

### When to Use Single Assertion

**Good**: Testing one specific behavior
```cpp
TEST(CameraTest, MoveForwardIncreasesPositionAlongFrontVector)
{
    // Arrange
    Camera camera(800, 600);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.speed = 1.0f;
    
    // Act
    camera.moveForward();
    
    // Assert
    EXPECT_EQ(camera.cameraPos.z, -1.0f);
}
```

### When Multiple Assertions Are Acceptable

**Acceptable**: Multiple assertions testing the same logical concept
```cpp
TEST(CameraTest, UpdateCalculatesFrontVectorFromYawAndPitch)
{
    // Arrange
    Camera camera(800, 600);
    float deltaTime = 0.016f;
    
    // Act
    camera.update(deltaTime);
    
    // Assert - All three components are part of the same "front vector" concept
    EXPECT_FLOAT_EQ(camera.cameraFront.x, 0.0f);
    EXPECT_FLOAT_EQ(camera.cameraFront.y, 0.0f);
    EXPECT_FLOAT_EQ(camera.cameraFront.z, -1.0f);
}
```

### When to Split Tests

**Split**: Different behaviors or independent concerns
```cpp
// Bad: Testing two independent behaviors
TEST(CameraTest, MoveAndLook)
{
    Camera camera(800, 600);
    camera.moveForward();
    EXPECT_NE(camera.cameraPos, glm::vec3(0.0f));
    
    camera.lookUp(10.0f);
    EXPECT_GT(camera.pitch, 0.0f);  // Different behavior - should be separate test
}

// Good: Split into two tests
TEST(CameraTest, MoveForwardChangesPosition)
{
    // Test movement only
}

TEST(CameraTest, LookUpIncreasesPitch)
{
    // Test looking only
}
```

**Guideline**: If you can't describe the test in a single sentence without using "and", consider splitting it.

---

## Test Naming Conventions

Test names should clearly describe what is being tested and under what conditions.

### Pattern: `UnitName_StateUnderTest_ExpectedResult`

This pattern ensures tests clearly communicate:
- **UnitName**: The method, function, or behavior being tested
- **StateUnderTest**: The specific condition or scenario (optional if testing default behavior)
- **ExpectedResult**: What should happen

```cpp
// Testing specific methods
TEST(CameraTest, MoveForward_IncreasesZPosition)
TEST(CameraTest, ClampPitch_LimitsTo89Degrees)
TEST(CameraTest, Update_NormalizesDirectionVector)

// Testing with specific state/conditions
TEST(CameraTest, SetRenderDistance_WithZero_ClampsToMinimum)
TEST(CameraTest, SpherDistance_BelowOne_ClampsToOne)

// Testing error conditions
TEST(ShaderTest, Constructor_WithInvalidPath_PrintsError)
TEST(ShaderTest, CompileShader_WithSyntaxError_ReportsFailure)

// Testing default behavior (StateUnderTest can be omitted)
TEST(CameraTest, Constructor_InitializesDefaultPosition)
TEST(CameraTest, SetupCam_ReturnsValidViewMatrix)
```

### Test Suite Naming

- Use the class name followed by "Test": `CameraTest`, `ShaderTest`
- For functionality tests without a specific class: `GraphicsUtilsTest`, `MatrixMathTest`
- Keep suite names concise and descriptive

### Test Case Naming Guidelines

- Start with the method name or behavior being tested (UnitName)
- Include the condition or input state if relevant (StateUnderTest)
- End with the expected outcome (ExpectedResult)
- Use underscores to separate the three parts for readability
- Be specific and descriptive
- Use underscores to separate sections for readability
- Be specific and descriptive

**Examples:**
```cpp
// Good
TEST(CameraTest, UpdateSpeed_WithShiftKey_MultipliesByTwenty)
TEST(ShaderTest, Use_BindsProgram)
TEST(CameraTest, Pitch_Above89Degrees_IsClamped)

// Avoid - too vague
TEST(CameraTest, SpeedWorks)
TEST(ShaderTest, TestShader)
TEST(CameraTest, Update)
```

---

## Test Organization

### Directory Structure

```
tests/
├── core/
│   ├── CameraTests.cpp
│   ├── ShaderTests.cpp
│   └── ParticleTests.cpp
├── mocks/
│   ├── MockOpenGL.hpp
│   └── MockGLFW.hpp
├── utils/
│   └── TestHelpers.hpp
└── CMakeLists.txt
```

### File Organization

Each test file should:
1. Include necessary headers
2. Include the mock headers
3. Group related tests in the same suite
4. Order tests logically (basic → complex, common → edge cases)

```cpp
// CameraTests.cpp structure
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include "camera.hpp"
#include "mocks/MockOpenGL.hpp"

// Basic functionality tests
TEST(CameraTest, Constructor_InitializesDefaultValues) { }
TEST(CameraTest, SetupCam_ReturnsViewMatrix) { }

// Movement tests
TEST(CameraTest, MoveForward_IncreasesPosition) { }
TEST(CameraTest, MoveBackward_DecreasesPosition) { }

// Edge case tests
TEST(CameraTest, Pitch_Above89_IsClamped) { }
TEST(CameraTest, Pitch_BelowNegative89_IsClamped) { }
```

---

## Mocking Guidelines

Mocking is essential for testing graphics code without requiring a GPU or OpenGL context.

### When to Mock

- External dependencies (OpenGL, GLFW)
- File I/O operations
- System calls
- Expensive operations

### Mock Design Principles

1. **Track Calls**: Record which functions were called and with what parameters
2. **Predictable Returns**: Return consistent, testable values
3. **Verify Behavior**: Allow tests to verify the mock was used correctly
4. **Minimal Interface**: Only mock what you need to test

### Example: MockOpenGL

```cpp
class MockOpenGL
{
  public:
    // Call tracking
    static int createProgramCalls;
    static int compileShaderCalls;
    static int useProgramCalls;
    
    // Mock return values
    static GLuint nextProgramId;
    
    // Mock functions
    static GLuint mockCreateProgram()
    {
        createProgramCalls++;
        return nextProgramId++;
    }
    
    static void mockUseProgram(GLuint program)
    {
        useProgramCalls++;
        lastUsedProgram = program;
    }
    
    // Test helpers
    static void reset()
    {
        createProgramCalls = 0;
        compileShaderCalls = 0;
        useProgramCalls = 0;
        nextProgramId = 1;
    }
    
  private:
    static GLuint lastUsedProgram;
};

// Initialize static members in .cpp file
int MockOpenGL::createProgramCalls = 0;
// ... etc
```

### Using Mocks in Tests

```cpp
TEST(ShaderTest, Constructor_CallsCompileShader)
{
    // Arrange
    MockOpenGL::reset();
    
    // Act
    Shader shader("vertex.vs", "fragment.fs");
    
    // Assert
    EXPECT_GT(MockOpenGL::compileShaderCalls, 0);
}
```

### What to Mock

Mock external dependencies that are:
- Outside your control (OpenGL, system calls, file I/O)
- Slow to execute (database queries, network calls)
- Non-deterministic (random number generators, timestamps)
- Require special environment setup (GPU, hardware)

**Do NOT mock:**
- **Your unit under test**
  - When testing a class like `Shader`, test the real `Shader` class
  - Mock its dependencies (e.g., `OpenGL`), not the class itself
  - You CAN mock your own helper classes when they are dependencies of your unit under test
- Simple data structures or value objects
- Pure functions with no side effects

---

## Coverage Expectations

### Minimum Coverage: ≥80%

Aim for at least 80% code coverage for tested modules. Focus coverage on:
- Core logic and algorithms
- Public APIs
- Error handling paths
- Edge cases and boundary conditions

### What to Test

**Test YOUR code, not external libraries.**

The goal of unit testing is to verify that *your* code works correctly, not to test third-party libraries or frameworks. Trust that well-maintained external libraries have their own test suites.

**High Priority - Test These:**
- **Your public methods and APIs** - Core functionality you've written
- **Your complex algorithms and calculations** - Business logic and computations
- **Your error handling and validation** - How your code handles edge cases
- **Your state transitions** - How your objects change state
- **Your integration points** - How you use external APIs correctly

**Do NOT Test:**
- **External library functionality** - Don't test if `std::vector::push_back()` works
- **Framework behavior** - Don't test if Google Test assertions work
- **Mock infrastructure itself** - Testing mocks is testing test code, not production code
- **Simple getters/setters** - Unless they have validation logic
- **Trivial forwarding functions** - Functions that just call other functions

**Example - What to Test:**

```cpp
// ✓ GOOD: Testing YOUR code
TEST(ShaderTest, Constructor_WithValidFiles_InitializesProgram)
{
    // Testing that YOUR Shader class correctly uses OpenGL
    Shader shader("vertex.vs", "fragment.fs");
    EXPECT_NE(shader.Program, 0u);
}

// ✗ BAD: Testing external library or test infrastructure
TEST(ShaderTest, MockOpenGL_CanBeInitialized)
{
    // This tests the mock itself, not your Shader code
    MockOpenGL::reset();
    EXPECT_EQ(MockOpenGL::createProgramCalls, 0);
}

// ✗ BAD: Testing file I/O library
TEST(ShaderTest, TestFiles_CanBeCreatedAndRead)
{
    // This tests std::ifstream, not your code
    std::ifstream file("test.txt");
    EXPECT_TRUE(file.good());
}
```

### Coverage Guidelines

```cpp
// Example: Camera class coverage priorities

// High Priority - Test thoroughly
✓ update()              // Complex calculation
✓ setupCam()            // Core functionality
✓ clampPitch()          // Edge case handling
✓ moveForward/Back/etc  // State changes

// Medium Priority - Test key scenarios
✓ KeyReader()           // Main paths and edge cases
✓ RenderSphere()        // Key rendering logic
✓ calcSpherePos()       // Mathematical calculation

// Lower Priority - May skip if simple
○ setSpeed()            // Simple setter
○ setRenderDistance()   // Simple setter
```

---

## Running Tests

### Local Testing

```bash
# Build tests
mkdir build && cd build
cmake ..
make

# Run all tests
./tests/ParticleViewerTests

# Run specific test suite
./tests/ParticleViewerTests --gtest_filter=CameraTest.*

# Run specific test
./tests/ParticleViewerTests --gtest_filter=CameraTest.MoveForward_IncreasesPosition

# Run with verbose output
./tests/ParticleViewerTests --gtest_verbose=1

# Generate coverage report (if configured)
make coverage
```

### CI Testing

Tests run automatically on every pull request:
- All tests must pass for PR approval
- Coverage reports are generated and displayed
- Test failures block merging

### Test Performance

- Individual tests should complete in < 100ms
- Full test suite should complete in < 10 seconds
- If tests are slow, consider:
  - Reducing setup overhead
  - Using simpler test data
  - Mocking expensive operations

---

## Best Practices Summary

1. **Follow AAA Pattern**: Always structure tests with Arrange-Act-Assert
2. **One Reason to Fail**: Each test should verify one logical concept
3. **Descriptive Names**: Test names should clearly describe what they test
4. **Use Mocks**: Don't require real OpenGL/GPU for unit tests
5. **Test Edge Cases**: Don't just test the happy path
6. **Keep Tests Simple**: Tests should be easier to understand than the code they test
7. **Independent Tests**: Tests should not depend on each other
8. **Fast Feedback**: Tests should run quickly

---

## Example: Complete Test File

```cpp
/*
 * CameraTests.cpp
 *
 * Unit tests for the Camera class following AAA pattern
 * and single-assertion principle.
 */

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"
#include "mocks/MockOpenGL.hpp"

class CameraTestFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Common setup for all camera tests
        MockOpenGL::reset();
    }
    
    void TearDown() override
    {
        // Cleanup after each test
    }
    
    // Helper methods
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
};

// Basic functionality tests
TEST_F(CameraTestFixture, Constructor_InitializesDefaultPosition)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Assert
    EXPECT_EQ(camera.cameraPos, glm::vec3(0.0f, 0.0f, 3.0f));
}

TEST_F(CameraTestFixture, MoveForward_UpdatesPositionAlongFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.speed = 1.0f;
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 expectedPos = initialPos + camera.cameraFront * camera.speed;
    
    // Act
    camera.moveForward();
    
    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

// Edge case tests
TEST_F(CameraTestFixture, Pitch_Above89Degrees_IsClampedTo89)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Act
    camera.lookUp(100.0f);  // Try to exceed limit
    camera.update(0.016f);
    
    // Assert
    EXPECT_LE(camera.pitch, 89.0f);
}
```

---

## References

- [Google Test Documentation](https://google.github.io/googletest/)
- [AAA Pattern](http://wiki.c2.com/?ArrangeActAssert)
- [Test Driven Development by Example](https://www.amazon.com/Test-Driven-Development-Kent-Beck/dp/0321146530)

---

## Questions or Feedback?

These standards are living documents. If you have suggestions for improvements:
1. Open an issue on GitHub
2. Discuss in pull request reviews
3. Contact the maintainers

Remember: Good tests are as important as good code!
