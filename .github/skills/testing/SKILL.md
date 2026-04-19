---
name: testing
description: Use when writing or reviewing any test for Particle-Viewer.
---

## Iron Law

```
NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST
```

Violating the letter of this rule is violating the spirit of this rule.

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
  - YOU MUST SEE THE TEST FAIL. Observe the failure message. Confirm it fails
    for the reason you expect — not due to a compile error or wrong test setup.
  - If it passes immediately: you are testing existing behavior OR the test is wrong.
    Fix the test before writing any production code.

GREEN: Write minimal implementation
  - Simplest code to make the test pass
  - No extra features, no "while I'm here" refactoring
  - Run the full suite: ./build/tests/ParticleViewerTests
  - ALL tests must pass (not just the new one)
  - GREEN = permission to refactor. NOT permission to stop. Proceed to REFACTOR.

REFACTOR: Clean up
  - Remove duplication
  - Improve names
  - Never add behavior during refactor
  - Tests must stay green throughout
  - Apply the Behavior Preservation Gate: run tests before each change, run after each change
```

**Agile Alarm Bell:** "Let's refactor without writing tests first" is the most dangerous phrase pair in software. Refactoring without a test suite to hold behavior constant is not refactoring — it is reckless restructuring. Stop. Write characterization tests first. Then refactor.

---

## Step 1: Determine Test Type

Classify the test before writing it:
- **Unit test** — single class/function in isolation → `tests/core/`
- **Integration test** — component interactions → `tests/integration/`
- **Visual regression test** — pixel comparison → load `visual-regression-testing` skill
- **Test review** — check existing tests against standards → apply checklist in Step 5

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

### Visual Regression Tests → see `visual-regression-testing` skill

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
- [ ] Group related configuration into structs/POCOs instead of flat variables
- [ ] Resource cleanup: GL objects deleted in destructors/cleanup, check for leaks
- [ ] Tests compile and pass
- [ ] For visual regression tests: see visual-regression-testing skill checklist

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

## Test Size Taxonomy (SE@G Model)

Apply this taxonomy when classifying tests and deciding where they belong:

| Size | Resource use | Scope | Directory |
|------|-------------|-------|-----------|
| **Small** | No I/O, no network, no filesystem, no external processes, no OpenGL context | Single unit in memory | `tests/` root or subdirectory |
| **Medium** | Localhost I/O permitted (files, sockets), no external services, no real OpenGL | Component interactions, file I/O | `tests/integration/` |
| **Large** | Real OpenGL context, real GPU, external processes, full system | End-to-end rendering, visual output | `tests/visual-regression/` |

**Classification gate:** Before writing a test, classify it. If a "unit test" uses a real file, it is Medium. If it uses a real OpenGL context, it is Large. Misclassified tests in the wrong directory produce slow/unreliable CI runs.

## The Depended-Upon Behavior Rule

Any behavior that your code **relies upon** must have a test. This applies to:
- Your own functions (don't assume they work; prove it)
- Libraries you depend on (test the integration point, not the library internals)
- Configuration assumptions (if the behavior would surprise you if it changed, test it)

The inverse: if behavior changes and no test breaks, that behavior was untested. It is not safe to change — it is merely unverified to be safe. Add the test now.

## Coincidence Articulation

Tests that mirror the implementation's structure detect nothing — they fail together or pass together regardless of correctness.

**Rule:** The test must reason about the problem independently from the implementation.
- Test input/output contracts, not internal data structures
- Test what the function is supposed to do, not how it currently does it
- If the test would pass for any implementation that uses the same algorithm, it is not testing a contract — it is testing an implementation coincidence

Signal: if modifying the test file requires looking at the source file, the test is likely mirroring implementation structure. The test should be written from requirements, not from reading the implementation.

- Use `EXPECT_*` (non-fatal) for most assertions; use `ASSERT_*` (fatal) only when a test cannot meaningfully continue after failure
- If a test seems to test external library behavior, focus on the wrapper/integration instead
- If Arrange and Act seem identical, the test might be a constructor test — put expected values in Arrange, constructor call in Act

---

## CI Pipeline Rules

For CI workflow rules (artifact uploads, permissions, PR comments), see the `workflow` skill (`.github/skills/workflow/`).
