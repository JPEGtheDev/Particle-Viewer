---
title: "Mocking Guidelines"
description: "When and how to mock OpenGL and other external dependencies in Particle-Viewer tests."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, mocking, opengl, gtest]
related:
  - "TESTING_STANDARDS.md"
  - "testing-standards-coverage.md"
---

# Mocking Guidelines

## Why Mock

Mocking is required for testing graphics code without a GPU or OpenGL context.

Mock when the dependency is:

- Outside your control (OpenGL, GLFW, system calls)
- Slow (database queries, network)
- Non-deterministic (random number generators, timestamps)
- Requires special hardware (GPU)

## Mock Design Rules

1. **Track calls** — record which functions were called and with what parameters
2. **Return predictable values** — consistent, testable return values
3. **Provide a reset method** — clear all call counts before each test
4. **Minimal interface** — only mock what the test needs

```cpp
class MockOpenGL
{
  public:
    static int createProgramCalls;
    static int useProgramCalls;
    static GLuint nextProgramId;

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

    static void reset()
    {
        createProgramCalls = 0;
        useProgramCalls = 0;
        nextProgramId = 1;
    }

  private:
    static GLuint lastUsedProgram;
};
```

## What to Mock

**MUST mock:**
- External dependencies (OpenGL, GLFW)
- File I/O operations
- System calls and hardware interfaces

**MUST NOT mock:**
- The class under test — test the real implementation
- Simple data structures or value objects
- Pure functions with no side effects

> You CAN mock your own helper classes when they are dependencies of the unit under test.

## Usage in Tests

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

## Related

- [Testing Standards](TESTING_STANDARDS.md) — parent ToC stub
- [testing-standards-coverage.md](testing-standards-coverage.md) — what to test and coverage targets
- [testing-standards-organization.md](testing-standards-organization.md) — mock file location in tests/mocks/
