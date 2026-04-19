---
title: "Coverage Expectations"
description: "Minimum coverage targets, what to test vs. skip, and priority guidelines for Particle-Viewer test coverage."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, coverage, gtest, standards]
related:
  - "TESTING_STANDARDS.md"
  - "testing-standards-mocking.md"
---

# Coverage Expectations

## Target: ≥80%

Aim for at least 80% code coverage for tested modules. Focus on:

- Core logic and algorithms
- Public APIs
- Error handling paths
- Edge cases and boundary conditions

## Test YOUR Code — Not External Libraries

The goal is to verify that **your** code works correctly. DO NOT test third-party libraries.

**MUST test:**
- Your public methods and APIs
- Your complex algorithms and calculations
- Your error handling and validation
- Your state transitions
- Your integration points (how you use external APIs)

**DO NOT test:**
- External library functionality (e.g., `std::vector::push_back()`)
- Framework behavior (e.g., Google Test assertions)
- Mock infrastructure — testing mocks is testing test code, not production code
- Simple getters/setters with no validation logic
- Trivial forwarding functions

```cpp
// CORRECT — testing YOUR Shader class
TEST(ShaderTest, Constructor_WithValidFiles_InitializesProgram)
{
    Shader shader("vertex.vs", "fragment.fs");
    EXPECT_NE(shader.Program, 0u);
}

// WRONG — testing mock infrastructure
TEST(ShaderTest, MockOpenGL_CanBeInitialized)
{
    MockOpenGL::reset();
    EXPECT_EQ(MockOpenGL::createProgramCalls, 0);
}
```

## Priority Tiers

| Priority | What | Examples |
|----------|------|---------|
| High | Complex logic, public API | `update()`, `setupCam()`, `clampPitch()` |
| Medium | Main paths and edge cases | `KeyReader()`, `calcSpherePos()` |
| Low / Skip | Simple setters | `setSpeed()`, `setRenderDistance()` |

## Best Practices

1. Follow AAA Pattern in every test
2. One reason to fail per test
3. Use descriptive names
4. Use mocks — DO NOT require real OpenGL/GPU
5. Test edge cases, not just the happy path
6. Keep tests independent — DO NOT share state between tests

## Related

- [Testing Standards](TESTING_STANDARDS.md) — parent ToC stub
- [testing-standards-mocking.md](testing-standards-mocking.md) — mock design principles
- [testing-standards-aaa.md](testing-standards-aaa.md) — AAA structure
