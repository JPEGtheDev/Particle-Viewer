---
title: "Test Assertions and Naming"
description: "Single assertion principle, when to split tests, and the UnitName_StateUnderTest_ExpectedResult naming pattern."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, naming, assertions, gtest]
related:
  - "TESTING_STANDARDS.md"
  - "testing-standards-aaa.md"
---

# Test Assertions and Naming

## Single Assertion Principle

Each test MUST verify **one logical concept**. Multiple assertions are acceptable only when they all relate to the same behavior.

**One concept, one assertion:**
```cpp
TEST(CameraTest, MoveForward_IncreasesPositionAlongFrontVector)
{
    Camera camera(800, 600);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.speed = 1.0f;
    camera.moveForward();
    EXPECT_EQ(camera.cameraPos.z, -1.0f);
}
```

**Multiple assertions — same concept (acceptable):**
```cpp
TEST(CameraTest, Update_CalculatesFrontVector)
{
    Camera camera(800, 600);
    camera.update(0.016f);
    // All three verify the same "front vector" concept
    EXPECT_FLOAT_EQ(camera.cameraFront.x, 0.0f);
    EXPECT_FLOAT_EQ(camera.cameraFront.y, 0.0f);
    EXPECT_FLOAT_EQ(camera.cameraFront.z, -1.0f);
}
```

If you cannot describe the test in one sentence without "and", split it.

## Test Naming Convention

**Pattern:** `UnitName_StateUnderTest_ExpectedResult`

- **UnitName** — the method or behavior under test
- **StateUnderTest** — the condition or input scenario (omit for default behavior)
- **ExpectedResult** — what MUST happen

```cpp
// Specific condition
TEST(CameraTest, SetRenderDistance_WithZero_ClampsToMinimum)
TEST(ShaderTest, Constructor_WithInvalidPath_PrintsError)

// Default behavior (StateUnderTest omitted)
TEST(CameraTest, Constructor_InitializesDefaultPosition)
TEST(CameraTest, SetupCam_ReturnsValidViewMatrix)
```

## Test Suite Naming

- Use class name + "Test": `CameraTest`, `ShaderTest`
- For utility tests: `GraphicsUtilsTest`, `MatrixMathTest`

## Anti-Patterns

```cpp
// WRONG — too vague
TEST(CameraTest, SpeedWorks)
TEST(ShaderTest, TestShader)

// WRONG — two independent behaviors in one test
TEST(CameraTest, MoveAndLook)
{
    camera.moveForward();
    EXPECT_NE(camera.cameraPos, glm::vec3(0.0f));
    camera.lookUp(10.0f);
    EXPECT_GT(camera.pitch, 0.0f); // different behavior — split this
}

// CORRECT — split into independent tests
TEST(CameraTest, MoveForward_ChangesPosition) { }
TEST(CameraTest, LookUp_IncreasesPitch) { }
```

## Related

- [Testing Standards](TESTING_STANDARDS.md) — parent ToC stub
- [testing-standards-aaa.md](testing-standards-aaa.md) — Arrange-Act-Assert structure
- [testing-standards-organization.md](testing-standards-organization.md) — test file layout
