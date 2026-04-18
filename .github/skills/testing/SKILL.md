---
name: testing
description: Write tests for Particle-Viewer following AAA pattern, naming conventions, and project testing standards. Use when writing unit tests, integration tests, visual regression tests, or reviewing test code. Covers Google Test patterns, mock usage, and visual comparison testing.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.3"
  category: testing
  project: Particle-Viewer
---

## Iron Law

```
NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST
```

Write the test. Watch it fail. THEN write code.

If you wrote code before the test: **Delete it. Start over.** No exceptions.
- Don't keep it as "reference"
- Don't "adapt" it while writing tests
- Don't look at it while writing tests
- Delete means delete

**Announce at start:** "I am using the testing skill to [write/review/fix] [specific test description]."

---

# Instructions for Agent

## How This Skill is Invoked

In VS Code, users will activate this skill by:
- Typing `@workspace /testing [description]` in Copilot Chat
- Or asking: "Write a test for [feature]", "Add visual regression tests", "Review my test code"

When activated, write tests that strictly follow the project's testing standards.

---

## Core Approach: Standards-First Testing

**Every test MUST follow the Arrange-Act-Assert (AAA) pattern with all three phases as separate sections.**

Before writing any test, **load and review** [references/TESTING_EXAMPLES.md](references/TESTING_EXAMPLES.md) for concrete examples of correct and incorrect patterns.

---

## TDD Cycle

**RED → GREEN → REFACTOR. In that order. Every time.**

```
RED: Write one failing test
  - Use TEST() or TEST_F() macro
  - Name: UnitName_Condition_ExpectedResult
  - Test one behavior only
  - Run it: ./build/tests/ParticleViewerTests --gtest_filter=Suite.TestName
  - Confirm it fails with expected message (NOT a compile error — fix those first)
  - If it passes immediately: you're testing existing behavior. Fix the test.

GREEN: Write minimal implementation
  - Simplest code to make the test pass
  - No extra features, no "while I'm here" refactoring
  - Run the full suite: ./build/tests/ParticleViewerTests
  - ALL tests must pass (not just the new one)

REFACTOR: Clean up
  - Remove duplication
  - Improve names
  - Never add behavior during refactor
  - Tests must stay green throughout
```

---

## Step 1: Determine Test Type

Ask the user what they need:

> "What would you like to test? I support:
> 1. **Unit tests** — Test a single class/function in isolation
> 2. **Integration tests** — Test component interactions
> 3. **Visual regression tests** — Compare images pixel-by-pixel
> 4. **Test review** — Check existing tests against our standards"

---

## Step 2: Write Tests Following AAA Pattern

### The Three Phases (ALWAYS separate)

Every test MUST have three distinct comment sections:

```cpp
TEST(SuiteName, MethodName_Condition_ExpectedResult)
{
    // Arrange
    // Set up test preconditions, create objects, define expected values

    // Act
    // Execute the single operation being tested

    // Assert
    // Verify the outcome
}
```

### Critical Rules

1. **NEVER combine phases.** Do not write `// Arrange & Act` or `// Act & Assert`. Each phase gets its own comment and section.
   - **Exception:** `// Act & Assert` is acceptable only for `EXPECT_NO_THROW`/`EXPECT_THROW` tests where the action IS the assertion.
2. **If no Arrange is needed**, omit `// Arrange` entirely — start with `// Act`.
3. **Move expected values to Arrange** as named variables, not inline in Assert.
4. **One logical concept per test** — split if testing multiple behaviors.

### Naming Convention

Use format: `UnitName_StateUnderTest_ExpectedResult`

Examples:
- `MoveForward_IncreasesZPosition`
- `Compare_IdenticalImages_ReturnsMatch`
- `ParsePPM_InvalidFile_ReturnsEmptyData`

---

## Step 3: Choose the Right Test Pattern

### Unit Tests (tests/core/)

Test a single class method in isolation. Use MockOpenGL for OpenGL calls.

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

### Integration Tests (tests/integration/)

Test component interactions. May involve multiple classes.

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

### Visual Regression Tests (tests/visual-regression/)

Use production classes (e.g., `Particle`) directly — **never duplicate production logic in test helpers**.
Use `PixelComparator` and the `Image` class for pixel comparison.

```cpp
TEST_F(RenderingRegressionTest, RenderDefaultCube_AngledView_MatchesBaseline)
{
    // Arrange
    Shader particleShader(vertexPath.c_str(), fragmentPath.c_str());
    Particle particles;  // Uses production Particle class directly
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 3000.0f);

    // Act
    glContext_.bindFramebuffer();
    renderParticle(particles, particleShader, view, projection);
    Image currentImage = glContext_.captureFramebuffer();

    // Assert
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, tolerance, true);
    EXPECT_TRUE(result.matches);
}
```

---

## Step 4: Apply Test File Organization

### Directory Structure

- `tests/core/` — Unit tests for `src/*.hpp` classes
- `tests/integration/` — Multi-component tests
- `tests/testing/` — Tests for test utilities (PixelComparator, Image)
- `tests/visual-regression/` — Visual regression tests
- `tests/mocks/` — Mock implementations

### File Naming

Each test file matches its source: `CameraTests.cpp` tests `camera.hpp`.

### Test Ordering

Within a file, order tests: basic → complex, common → edge cases.

---

## Step 5: Self-Review Checklist

Before presenting tests, verify:

- [ ] Every test has separate `// Arrange`, `// Act`, `// Assert` comments (no `// Arrange & Act`)
- [ ] Test name follows `UnitName_StateUnderTest_ExpectedResult` pattern
- [ ] Expected values are named variables in Arrange (not inline literals in Assert)
- [ ] One logical concept per test
- [ ] External dependencies are mocked (OpenGL, file I/O)
- [ ] No testing of external libraries (std::, third-party code)
- [ ] Visual regression tests use production classes (Particle, Camera) — no duplicated test helpers
- [ ] Group related configuration into structs/POCOs instead of flat variables
- [ ] Resource cleanup: GL objects deleted in destructors/cleanup, check for leaks
- [ ] Test `SetUp()` ensures all output directories exist (artifacts/, baselines/, diffs/)
- [ ] Artifact `save()` return values are checked, not silently ignored
- [ ] Visual test resolution matches viewer defaults (1280x720) unless specifically testing other resolutions
- [ ] Tests compile and pass

---

## Red Flags — STOP

If you catch yourself thinking any of these, STOP and start over with RED:

- Writing implementation code before writing a test
- "I'll write tests after to verify it works"
- "The visual regression test will cover this"
- "It's too complex to unit test with MockOpenGL" (MockOpenGL exists for exactly this)
- "I already manually tested it"
- "Tests after achieve the same goals"
- "Just this once" or "This is different because..."
- Test passes immediately without seeing it fail first
- "I need to get the implementation right before I know what to test"

**All of these mean: Delete any code written before the test. Start over with RED.**

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "Too simple to test" | Simple code breaks. The test takes 30 seconds. |
| "I'll write tests after" | Tests passing immediately after implementation prove nothing. |
| "Visual regression test will cover it" | Visual tests are slow and test pixels, not logic. Unit test the logic. |
| "Too complex to test in isolation" | That's a design signal. Simplify the interface. MockOpenGL is there for GL calls. |
| "Already manually tested it" | Manual testing is ad-hoc. No record, can't re-run, misses edge cases. |
| "Tests after achieve the same goals" | Tests-after answer "what does this do?" Tests-first answer "what SHOULD this do?" |
| "Deleting X hours of work is wasteful" | Sunk cost. Keeping untested code is technical debt. |
| "TDD slows me down" | TDD is faster than debugging production failures. |

---

## Key Design Principles (Learned from Review Feedback)

1. **Use production classes in tests.** Visual regression tests should use `Particle` directly instead of re-implementing particle creation logic in a test helper class. This ensures tests stay in sync with production code.

2. **Group related data into POCOs/structs.** When a test or test helper has many flat member variables, group them into domain-specific structs (e.g., `RenderConfig`, `CameraSetup`). This mirrors the production code pattern.

3. **Clean up GL resources.** Every test that creates GL objects (VAOs, VBOs, FBOs, textures) must clean them up. Check for leaks in `cleanup()` / destructors.

4. **Binary file I/O.** Always open binary data files with `"rb"` mode (not `"r"`) for cross-platform correctness.

5. **Ensure output directories exist.** In test `SetUp()`, create all output directories (artifacts/, baselines/, diffs/) before tests run. Check `save()` return values so failures are actionable, not silent.

6. **Visual test resolution.** Use the viewer's default resolution (1280x720) for visual regression tests unless specifically testing other resolutions. Non-default resolutions can cause warping and scaling artifacts.

7. **Camera positioning for visual tests.** Don't blindly copy debug camera coordinates — debug shows interactive state, not ideal test framing. Extract the viewing **direction** from debug output, then calculate **distance** based on desired viewport coverage: `distance = subject_size / (coverage_% * tan(FOV/2))`. See `docs/visual-regression/camera-positioning-lessons-learned.md`.

---

## Self-Evaluation

After completing test work, run the `self-evaluation` skill (`.github/skills/self-evaluation/`) to capture any new testing patterns learned during the session.

For mock-related anti-patterns, see [references/testing-anti-patterns.md](references/testing-anti-patterns.md).

---

## Key Types for Visual Regression

| Type | Location | Purpose |
|------|----------|---------|
| `Image` | `src/Image.hpp` | RGBA pixel buffer with save/load (PPM, PNG) |
| `ComparisonResult` | `src/testing/PixelComparator.hpp` | Match status, similarity, diff image |
| `PixelComparator` | `src/testing/PixelComparator.hpp` | Pixel comparison engine |
| `ImageFormat` | `src/Image.hpp` | Format enum (PPM, PNG) for Image::save/load |
| `Particle` | `src/particle.hpp` | Production particle data (std::vector<glm::vec4>) |

---

## Common Edge Cases to Handle

- Ask if the user wants `EXPECT_*` (non-fatal) or `ASSERT_*` (fatal) assertions
- For visual tests: recommend `0.0f` tolerance for synthetic data, `2.0f/255.0f` for GPU-rendered
- If a test seems to test external library behavior, suggest focusing on the wrapper/integration instead
- If Arrange and Act seem identical, the test might be a constructor test — put expected values in Arrange, constructor call in Act

---

## CI Pipeline Rules

For CI workflow rules (artifact uploads, permissions, PR comments), see the `workflow` skill (`.github/skills/workflow/`).
