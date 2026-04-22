# Particle-Viewer Test Conventions

Project-specific testing patterns, examples, and design principles for Particle-Viewer.

---

## Unit Test Example (Camera)

```cpp
TEST(CameraTest, MoveForward_DefaultSpeed_IncreasesPosition)
{
    // Arrange
    Camera camera(800, 600);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.setSpeed(1.0f);

    // Act
    camera.moveForward();

    // Assert
    EXPECT_LT(camera.cameraPos.z, 0.0f);
}
```

---

## Integration Test Example (SettingsIO)

```cpp
TEST(DataLoadingPipelineTest, LoadSettings_ValidFile_PopulatesParticles)
{
    // Arrange
    SettingsIO settings;
    std::string test_file = createTestSettingsFile();

    // Act
    bool result = settings.loadSettings(test_file);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_GT(settings.getParticleCount(), 0u);
}
```

---

## Test File Organization

- `tests/core/` — Unit tests for `src/*.hpp` classes
- `tests/integration/` — Multi-component tests
- `tests/testing/` — Tests for test utilities (PixelComparator, Image)
- `tests/visual-regression/` — Visual regression tests
- `tests/mocks/` — Mock implementations

File naming: each test file matches its source — `CameraTests.cpp` tests `camera.hpp`.

---

## Key Design Principles (Learned from Review Feedback)

1. **Use production classes in tests.** Visual regression tests MUST use `Particle` directly instead of re-implementing particle creation logic in a test helper class. This ensures tests stay in sync with production code.

2. **Group related data into POCOs/structs.** When a test or test helper has many flat member variables, group them into domain-specific structs (e.g., `RenderConfig`, `CameraSetup`). This mirrors the production code pattern.

3. **Clean up GL resources.** Every test that creates GL objects (VAOs, VBOs, FBOs, textures) must clean them up. Check for leaks in `cleanup()` / destructors.

4. **Binary file I/O.** Always open binary data files with `"rb"` mode (not `"r"`) for cross-platform correctness.

5. **Ensure output directories exist.** In test `SetUp()`, create all output directories (artifacts/, baselines/, diffs/) before tests run. Check `save()` return values so failures are actionable, not silent.

6. **Visual test resolution.** Use the viewer's default resolution (1280x720) for visual regression tests unless specifically testing other resolutions. Non-default resolutions can cause warping and scaling artifacts.

7. **Camera positioning for visual tests.** Don't blindly copy debug camera coordinates — debug shows interactive state, not ideal test framing. Extract the viewing **direction** from debug output, then calculate **distance** based on desired viewport coverage: `distance = subject_size / (coverage_% * tan(FOV/2))`. See `docs/visual-regression/camera-positioning-lessons-learned.md`.

8. **Every bug fix requires a regression test.** Write a test that reproduces the bug (fails before the fix). Fix the code. Confirm the test now passes. A bug fixed without a test is a bug scheduled for a return visit.

---

For project test runner commands, see the `execution` skill.
