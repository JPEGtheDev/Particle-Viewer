---
title: "Test Conventions"
description: "Project test conventions: naming, file organization, test double taxonomy, test sizes, and design principles for Particle Viewer tests."
domain: testing
subdomain: conventions
tags: [testing, conventions, mocking, organization]
related:
  - "../TESTING_STANDARDS.md"
  - "integration-tests.md"
  - "testing-examples.md"
---

# Particle-Viewer Test Conventions

Project-specific testing patterns, examples, and design principles for Particle-Viewer.

---

## Unit Test Example (Camera)

See [Unit Test: Camera Movement](testing-examples.md#unit-test-camera-movement) in testing-examples.md for the canonical example: `TEST_F(CameraTest, MoveForward_IncreasesPositionAlongFrontVector)`, quoted there from `tests/core/CameraTests.cpp`.

---

## Integration Test Example (SettingsIO)

Quoted from the `DataLoadingPipelineTest` fixture in `tests/integration/DataLoadingPipelineTests.cpp` (`posPath`/`statsPath`/`comPath` are fixture members pointing at files written by the fixture's `createTestDataFiles()` helper in `SetUp()`; `NUM_PARTICLES` is a fixture enum constant, 50):

```cpp
TEST_F(DataLoadingPipelineTest, LoadSettings_ThenReadFrame_PopulatesParticleData)
{
    // Arrange: Create SettingsIO to read the settings file
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0 through the pipeline
    settings.readPosVelFile(0, &part, false);

    // Assert: Particle data is populated with correct count
    EXPECT_EQ(part.n, NUM_PARTICLES);
}
```

---

## Test File Organization

- `tests/core/` -- Unit tests for `src/*.hpp` classes
- `tests/integration/` -- Multi-component tests
- `tests/testing/` -- Tests for test utilities (PixelComparator, Image)
- `tests/visual-regression/` -- Visual regression tests
- `tests/mocks/` -- Mock implementations

File naming: each test file matches its source -- `CameraTests.cpp` tests `camera.hpp`.

---

## Key Design Principles (Learned from Review Feedback)

1. **Use production classes in tests.** Visual regression tests MUST use `Particle` directly instead of re-implementing particle creation logic in a test helper class. This ensures tests stay in sync with production code.

2. **Group related data into POCOs (Plain Old C++ Objects -- plain data structs)/structs.** When a test or test helper has many flat member variables, group them into domain-specific structs (e.g., `RenderConfig`, `CameraSetup`). This mirrors the production code pattern.

3. **Clean up GL resources.** Every test that creates GL objects (VAOs [Vertex Array Objects], VBOs [Vertex Buffer Objects], FBOs [Framebuffer Objects], textures) must clean them up. Check for leaks in `cleanup()` / destructors.

4. **Binary file I/O.** Always open binary data files with `"rb"` mode (not `"r"`) for cross-platform correctness.

5. **Ensure output directories exist.** In test `SetUp()`, create all output directories (artifacts/, baselines/, diffs/) before tests run. Check `save()` return values so failures are actionable, not silent.

6. **Visual test resolution.** Use the viewer's default resolution (1280x720) for visual regression tests unless specifically testing other resolutions. Non-default resolutions can cause warping and scaling artifacts.

7. **Camera positioning for visual tests.** Don't blindly copy debug camera coordinates -- debug shows interactive state, not ideal test framing. Extract the viewing **direction** from debug output, then calculate **distance** based on desired viewport coverage and the camera's FOV (Field of View): `distance = subject_size / (coverage_% * tan(FOV/2))`. See `docs/visual-regression/camera-positioning-lessons-learned.md`.

8. **Every bug fix requires a regression test.** Write a test that reproduces the bug (fails before the fix). Fix the code. Confirm the test now passes. A bug fixed without a test is a bug scheduled for a return visit.

---

For project test runner commands, see [Verification Commands](../VERIFICATION_COMMANDS.md).

---

## Test Double Taxonomy

Use the **least sophisticated double** that answers your question. Reaching for `EXPECT_CALL` when a stub suffices is over-engineering.

| Double | Behavior | Verifies Calls? | When to Use |
|--------|----------|-----------------|-------------|
| **Stub** | No-op methods; returns null/zero/false | No | You need `IOpenGLContext` to not crash; you don't care what it called |
| **Fake** | Returns programmable values via setters | No | You need to control what `glGetError()` returns without interaction verification |
| **Mock** | Returns values AND verifies call expectations | Yes -- test fails if expected calls are not made | You must assert `glDrawArrays` was called exactly once with specific arguments |
| **Shunt / SelfShunt** | The test fixture itself implements the interface | Inspected in teardown | Lowest setup overhead when the fixture plays both collaborator and verifier |

**Project reality:** Particle-Viewer does not use Google Mock -- `tests/CMakeLists.txt` links only `gtest` and `gtest_main`, and a repo-wide grep for `MOCK_METHOD`/`EXPECT_CALL` returns zero hits under `--include="*.cpp" --include="*.hpp"`. Every test double here is hand-rolled. For example, `MockOpenGLContext` (`tests/mocks/MockOpenGLContext.hpp`) is a hand-rolled **Fake**, not a Mock: it subclasses `IOpenGLContext` and returns programmable values via setters (`setTime()`, `setContentScale()`, `setSwapInterval()`), and it exposes call counters (`getSwapCount()`, `getPollCount()`) that a test reads and asserts on manually -- there is no `EXPECT_CALL`/`Times()` machinery verifying calls automatically.

**Key principle:** Mock the *role* (interface), not the concrete object. `MockOpenGLContext` implements `IOpenGLContext` -- stable across implementation changes.

**When to add interaction verification:** If the question is "does this run without crashing?", a stub suffices. Only add a counter that a test asserts on (as `MockOpenGLContext` does) when the interaction itself is the behavior under test.

---

For directory layout and file naming conventions, see **Test File Organization** above.

### Test Ordering

Within a file, order tests: basic -> complex, common -> edge cases.

---

## Test Size Taxonomy (Software Engineering at Google Model)

Apply this taxonomy when classifying tests and deciding where they belong:

| Size | Resource use | Scope | Directory |
|------|-------------|-------|-----------|
| **Small** | No I/O, no network, no filesystem, no external processes, no OpenGL context | Single unit in memory | `tests/` root or subdirectory |
| **Medium** | Localhost I/O permitted (files, sockets), no external services, no real OpenGL | Component interactions, file I/O | `tests/integration/` |
| **Large** | Real OpenGL context, real GPU, external processes, full system | End-to-end rendering, visual output | `tests/visual-regression/` |

**Classification gate:** Before writing a test, classify it. If a "unit test" uses a real file, it is Medium. If it uses a real OpenGL context, it is Large. Misclassified tests in the wrong directory produce slow/unreliable CI runs.

---

## Additional Rationalization Prevention Rows

These are project-specific additions to the general set of testing rationalization patterns used across the team's testing practice (not duplicated here):

| Excuse | Reality |
|--------|---------|
| "Tests after achieve the same goals" | Tests-after answer "what does this do?" Tests-first answer "what SHOULD this do?" |
| "Deleting X hours of work is wasteful" | Sunk cost. Keeping untested code is technical debt. |
| "The bug was a one-off, no regression test needed" | One-off bugs recur after the next refactoring. A regression test takes 5 minutes; a re-investigation takes hours. |

Additional Red Flags -- STOP:

- "Tests after achieve the same goals"
- "Just this once" or "This is different because..."
- "I need to get the implementation right before I know what to test"

---

## Related

- [Testing Standards](../TESTING_STANDARDS.md) -- Project-wide test guidelines and AAA pattern
- [Integration Tests Guide](integration-tests.md) -- Overview and table of contents for integration testing
- [Testing Examples and Patterns](testing-examples.md) -- Worked test examples quoted from real test source, including the canonical Camera example this file points to
