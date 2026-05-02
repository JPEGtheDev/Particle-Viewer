---
title: "Test Structure: Arrange-Act-Assert"
description: "AAA pattern rules and examples for structuring all Particle-Viewer tests."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, aaa, gtest, patterns]
related:
  - "TESTING_STANDARDS.md"
  - "testing-standards-assertions.md"
---

# Test Structure: Arrange-Act-Assert

## Philosophy

**A test MUST have only one reason to fail.** When a test fails, the cause MUST be immediately clear.

**Goals:**
- **Reliability**: Tests MUST pass consistently when code is correct
- **Clarity**: Failures MUST identify exactly what broke
- **Speed**: Tests MUST run quickly (individual test < 100ms)

## The AAA Pattern

All tests MUST follow **Arrange-Act-Assert**:

- **Arrange** — set up preconditions: create objects, initialize data, configure mocks, declare expected values
- **Act** — execute the code under test (one call)
- **Assert** — verify the outcome: return values, state changes, side effects

```cpp
TEST(CameraTest, SetupCam_ReturnsViewMatrix)
{
    // Arrange
    Camera camera(800, 600);
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

## AAA Rules

- DO NOT combine Arrange and Act into `// Arrange & Act`
- DO NOT combine Act and Assert into `// Act & Assert`
- If no meaningful Arrange step exists, omit the `// Arrange` comment — start directly with `// Act`
- Put expected values as named variables in Arrange, not inline in Assert

```cpp
// CORRECT — no arrange step needed
TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Act
    Image image;

    // Assert
    EXPECT_TRUE(image.empty());
}

// CORRECT — expected values declared in Arrange
TEST(ImageTest, Constructor_WithDimensions_SetsWidthAndHeight)
{
    // Arrange
    uint32_t expected_width = 16;
    uint32_t expected_height = 32;

    // Act
    Image image(expected_width, expected_height);

    // Assert
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
}
```

## Related

- [Testing Standards](TESTING_STANDARDS.md) — parent ToC stub and overview
- [testing-standards-assertions.md](testing-standards-assertions.md) — single assertion principle and test naming
