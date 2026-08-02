---
title: "Testing Examples and Patterns"
description: "Concrete correct/incorrect test examples: AAA pattern, naming, mocking, assertions, and key type references for Particle Viewer tests."
domain: testing
subdomain: examples
tags: [testing, examples, aaa, mocking]
related:
  - "../TESTING_STANDARDS.md"
  - "../testing-standards-aaa.md"
  - "test-conventions.md"
---

# Testing Examples and Patterns

This reference provides concrete examples of correct and incorrect test patterns for Particle-Viewer. Load this when writing or reviewing tests.

---

## Arrange-Act-Assert (AAA) Pattern -- Critical Rules

1. **NEVER combine phases.** Do not write `// Arrange & Act` or `// Act & Assert`. Each phase gets its own comment and section, with no exception -- see [testing-standards-aaa.md](../testing-standards-aaa.md) for the canonical rule.
2. **If no Arrange is needed**, omit `// Arrange` entirely -- start with `// Act`.
3. **Move expected values to Arrange** as named variables, not inline in Assert.
4. **One logical concept per test** -- split if testing multiple behaviors.

---

## AAA Pattern -- Correct Examples

### Unit Test: Camera Movement

Quoted from the `CameraTest` fixture in `tests/core/CameraTests.cpp` (`SCREEN_WIDTH`/`SCREEN_HEIGHT` are fixture constants, 800/600):

```cpp
TEST_F(CameraTest, MoveForward_IncreasesPositionAlongFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(1.0f);
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 expectedPos = initialPos + camera.cameraFront * 1.0f;

    // Act
    camera.moveForward();

    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}
```

### Unit Test: Constructor with Named Expected Values

From the `ImageStructTest` suite in `tests/testing/ImageTests.cpp`:

```cpp
TEST(ImageStructTest, Constructor_WithDimensions_SetsWidthAndHeight)
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

### Unit Test: No Arrange Needed (Omit It)

From the `ImageTest` suite in `tests/testing/PixelComparatorTests.cpp` (an equivalent test also exists as `ImageStructTest.DefaultConstructor_CreatesEmptyImage` in `tests/testing/ImageTests.cpp`):

```cpp
TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Act
    Image img;

    // Assert
    EXPECT_TRUE(img.empty());
}
```

### Unit Test: PixelComparator Exact Match

From the `PixelComparatorTest` suite in `tests/testing/PixelComparatorTests.cpp`. `createSolidImage()` is a file-local helper (not a shared header) -- see [Visual Regression Test Helpers](#visual-regression-test-helpers) below:

```cpp
TEST(PixelComparatorTest, Compare_IdenticalImages_Matches)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_TRUE(result.matches);
}
```

### Unit Test: PixelComparator Tolerance Match

```cpp
TEST(PixelComparatorTest, Compare_SmallDiffWithinTolerance_Matches)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 101, 100, 100, 255);

    // Act - tolerance of 1/255 allows +/-1 difference
    ComparisonResult result = comparator.compare(img1, img2, 1.0f / 255.0f);

    // Assert
    EXPECT_TRUE(result.matches);
}
```

### Visual Regression: Using Production Particle Class

Adapted from `RenderSingleParticle_CenteredView_MatchesBaseline` in `tests/visual-regression/RenderingRegressionTests.cpp` (fixture `RenderingRegressionTest`, a Large/real-OpenGL test per the Test Size Taxonomy in [test-conventions.md](test-conventions.md)):

```cpp
TEST_F(RenderingRegressionTest, RenderSingleParticle_CenteredView_MatchesBaseline)
{
    // Arrange
    Shader particleShader(vertexPath.c_str(), fragmentPath.c_str());
    std::vector<glm::vec4> data = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
    Particle particles(1, data.data());  // Use production Particle class
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 3000.0f);

    // Act
    glContext_.bindFramebuffer();
    renderParticle(particles, particleShader, view, projection);
    Image currentImage = glContext_.captureFramebuffer();

    // Assert
    ASSERT_TRUE(currentImage.valid());
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, VRTestConfig::PARTICLE_TOLERANCE, true);
    EXPECT_TRUE(result.matches);
}
```

### Image Save/Load: Round-Trip (RGB preserved, alpha discarded)

Adapted from `Save_PPM_RoundTrip_PreservesPixelData` in the `ImageIOTest` fixture (`tests/testing/ImageTests.cpp`; `test_dir_` is created in `SetUp()` and removed in `TearDown()`):

```cpp
TEST_F(ImageIOTest, Save_PPM_RoundTrip_PreservesPixelData)
{
    // Arrange
    Image original(2, 2);
    original.pixels[0] = 255;
    original.pixels[1] = 128;
    original.pixels[2] = 64;
    original.pixels[3] = 255;
    std::string path = test_dir_ + "/roundtrip.ppm";
    original.save(path, ImageFormat::PPM);

    // Act
    Image loaded = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(loaded.pixels[0], 255u);
    EXPECT_EQ(loaded.pixels[1], 128u);
    EXPECT_EQ(loaded.pixels[2], 64u);
    EXPECT_EQ(loaded.pixels[3], 255u); // Alpha restored to 255
}
```

---

## AAA Pattern -- INCORRECT Examples (Do NOT Follow)

### [-] Combined Arrange & Act

```cpp
// BAD: "Arrange & Act" combined
TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Arrange & Act  <-- WRONG: Don't combine
    Image image;

    // Assert
    EXPECT_TRUE(image.empty());
}
```

**Fix:** Omit `// Arrange` entirely, start with `// Act`.

### [-] Combined Act & Assert

```cpp
// BAD: "Act & Assert" combined
TEST(PixelComparatorTest, Compare_IdenticalImages_Matches)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 0, 0, 255);

    // Act & Assert  <-- WRONG: Don't combine
    EXPECT_TRUE(comparator.compare(img1, img2, 0.0f).matches);
}
```

**Fix:** Separate into `// Act` (run comparison) and `// Assert` (check result).

### [-] Inline Expected Values in Assert

```cpp
// BAD: Magic numbers in assert
TEST(ImageTest, Constructor_SetsSize)
{
    // Act
    Image image(16, 32);

    // Assert
    EXPECT_EQ(image.width, 16);   // <-- WRONG: 16 is a magic number
    EXPECT_EQ(image.height, 32);  // <-- WRONG: 32 is a magic number
}
```

**Fix:** Put expected values as named variables in Arrange.

### [-] Testing External Library Behavior

```cpp
// BAD: Testing std::vector, not our code
TEST(ImageTest, PixelVector_Resizes)
{
    // Arrange
    std::vector<uint8_t> pixels(100);

    // Act
    pixels.resize(200);

    // Assert
    EXPECT_EQ(pixels.size(), 200u);  // <-- WRONG: Testing std::vector
}
```

**Fix:** Only test YOUR code -- wrapper logic, integration, or behavior you own.

### [-] EXPECT_LE When the Value Is Deterministic

```cpp
// BAD: LE masks regressions where the code under-enqueues
EXPECT_LE(callCount, window);  // passes even if callCount is 0 or window-1
```

**Fix:** Use `EXPECT_EQ` when the value is deterministic. Reserve `EXPECT_LE`/`EXPECT_GE` only for inherently non-deterministic results (e.g., timing, OS scheduling).

```cpp
// GOOD: fails immediately if cap logic changes
EXPECT_EQ(callCount, window);
```

### [-] Binary File Test Missing Required Record Count

When testing that a binary reader correctly handles a frame-N seek, a file with fewer than N+1 records causes `fseek` to go past EOF, so `fread` returns 0 (EOF/truncated), not a record with the wrong field. This means the test ends up exercising the truncation branch instead of the intended branch (e.g., field mismatch):

```cpp
// BAD: single-record file; seeking to frame=1 goes past EOF -> fread returns 0
// (tests truncation, not the intended field-mismatch path)
FILE* f = fopen(path.c_str(), "wb");
float r0[4] = {1.0f, 2.0f, 3.0f, 0.0f};  // frame 0 only
fwrite(r0, sizeof(float), 4, f);
fclose(f);
getCOM(/*frame=*/1, out);  // seeks to byte 16 -- past EOF
```

**Fix:** Write N+1 records so `fseek` lands within the file and `fread` succeeds:

```cpp
// GOOD: two records; seeking to frame=1 reads the second record's w field
FILE* f = fopen(path.c_str(), "wb");
float r0[4] = {0, 0, 0, 0};        // padding for frame 0
float r1[4] = {1.0f, 2.0f, 3.0f, 99.0f};  // frame 1, w=99 (wrong type to trigger mismatch)
fwrite(r0, sizeof(float), 4, f);
fwrite(r1, sizeof(float), 4, f);
fclose(f);
getCOM(/*frame=*/1, out);  // reads r1; w=99 != frame=1 -> mismatch branch
```

### [-] Vague Test Name

```cpp
// BAD: No clear behavior being tested
TEST(CameraTest, WorksProperly)     // <-- WRONG
TEST(ShaderTest, TestShader)        // <-- WRONG
TEST(CameraTest, Update)            // <-- WRONG
```

**Fix:** Use `UnitName_StateUnderTest_ExpectedResult` format.

### [-] Duplicating Production Logic in Test Helpers

```cpp
// BAD: Test helper class recreates Particle's cube generation logic
class ParticleRenderer {
    void createDefaultCube() {
        // Duplicated from Particle() constructor  <-- WRONG
        for (int i = 0; i < 64000; i++) {
            particleData_[i] = glm::vec4(i % 40 * 1.25, ...);
        }
    }
};
```

**Fix:** Use `Particle` directly -- tests stay in sync with production code automatically.

```cpp
// GOOD: Use production class
Particle particles;  // Default cube, same as app
// Or with custom data:
std::vector<glm::vec4> data = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
Particle particles(1, data.data());
```

---

## Key Types Reference

### Particle (src/particle.hpp)

```cpp
class Particle
{
  public:
    long n;                              // number of bodies
    GLuint instanceVBO;                  // GL instance buffer
    std::vector<glm::vec4> translations; // positions (x, y, z, colorValue)
    std::vector<glm::vec4> velocities;   // velocity data

    Particle();                                        // Default: 64,000 particles in 40x40x40 grid
    Particle(long number_of_bodies, const glm::vec4* positions); // Custom data (copies)

    void changeTranslations(long count, const glm::vec4* new_positions);
    void changeVelocities(const glm::vec4* new_velocities);
    void pushVBO();            // Upload translations to GPU
    void setUpInstanceArray(); // Configure vertex attribs for instancing
};
```

### Image (src/Image.hpp)

```cpp
class Image
{
  public:
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> pixels; // RGBA, 4 bytes per pixel

    Image();                           // Default: empty
    Image(uint32_t w, uint32_t h);    // Allocates w*h*4 bytes, zeroed
    bool empty() const;
    bool valid() const;

    // Save to file (drops alpha, writes RGB)
    bool save(const std::string& path, ImageFormat format) const;

    // Load from file (sets alpha to 255)
    static Image load(const std::string& path, ImageFormat format);
};

enum class ImageFormat { PPM, PNG };
```

### ComparisonResult (src/testing/PixelComparator.hpp)

```cpp
struct ComparisonResult
{
    bool matches;
    float similarity;        // 0.0 to 1.0 (1.0 = identical)
    uint32_t diff_pixels;
    uint32_t total_pixels;
    BoundingBox diff_bounds;
    Image diff_image;        // Red overlay on differing pixels
    std::string error;       // Non-empty on error (dimension mismatch, etc.)
};
```

### PixelComparator (src/testing/PixelComparator.hpp)

```cpp
class PixelComparator
{
  public:
    PixelComparator();
    ComparisonResult compare(const Image& baseline, const Image& current,
                            float tolerance, bool generate_diff = false) const;
    void setMode(ComparisonMode mode);  // EXACT or TOLERANT
    ComparisonMode getMode() const;
};
```

### Visual Regression Test Helpers

Shared constants and path-resolution helpers for visual regression (Large, real-OpenGL) tests live in `tests/visual-regression/VRTestCommon.hpp`:

```cpp
namespace VRTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;
static const uint32_t RENDER_HEIGHT = 720;
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f;
static const float MAX_DIFF_RATIO = 0.0001f;
static const std::string BASELINES_DIR = "baselines";
} // namespace VRTestConfig

std::string getShaderPath(const std::string& shaderName);
std::string getBaselinePath(const std::string& baselineName);
```

There is no shared image-construction helper header. Tests that need a solid-color `Image` build one with a small file-local helper -- see `createSolidImage()` in `tests/testing/PixelComparatorTests.cpp` (used only within that file, not shared across test binaries):

```cpp
static Image createSolidImage(uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Image img(width, height);
    for (size_t i = 0; i < img.pixels.size(); i += 4) {
        img.pixels[i] = r;
        img.pixels[i + 1] = g;
        img.pixels[i + 2] = b;
        img.pixels[i + 3] = a;
    }
    return img;
}
```

---

## Test Directory Structure

```
tests/
+-- core/               # Unit tests (CameraTests.cpp, ParticleTests.cpp, etc.)
+-- integration/        # Multi-component tests
+-- testing/            # Tests for PixelComparator, Image
+-- visual-regression/  # Visual comparison tests (uses production Particle class)
|   +-- baselines/      # Baseline images (committed, never modified by tests)
|   +-- VRTestCommon.hpp        # Shared VRTestConfig constants, getShaderPath/getBaselinePath
|   +-- RenderingRegressionTests.cpp
+-- mocks/              # MockOpenGL.hpp/.cpp
+-- stb_image_write_impl.cpp
+-- CMakeLists.txt
```

---

## AAA Pattern -- Template (Three Phases)

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

---

## Particle-Viewer (PV) Naming Examples

Use format: `UnitName_StateUnderTest_ExpectedResult`

- `Compare_IdenticalImages_ReturnsMatch`
- `ParsePPM_InvalidFile_ReturnsEmptyData`

---

## The Depended-Upon Behavior Rule

Any behavior that your code **relies upon** must have a test. This applies to:
- Your own functions (don't assume they work; prove it)
- Libraries you depend on (test the integration point, not the library internals)
- Configuration assumptions (if the behavior would surprise you if it changed, test it)

The inverse: if behavior changes and no test breaks, that behavior was untested. It is not safe to change -- it is merely unverified to be safe. Add the test now.

---

## Coincidence Articulation

Tests that mirror the implementation's structure detect nothing -- they fail together or pass together regardless of correctness.

**Rule:** The test must reason about the problem independently from the implementation.
- Test input/output contracts, not internal data structures
- Test what the function is supposed to do, not how it currently does it
- If the test would pass for any implementation that uses the same algorithm, it is not testing a contract -- it is testing an implementation coincidence

Signal: if modifying the test file requires looking at the source file, the test is mirroring implementation structure. The test MUST be written from requirements, not from reading the implementation.

- Use `EXPECT_*` (non-fatal) for most assertions; use `ASSERT_*` (fatal) only when a test cannot meaningfully continue after failure
- If a test seems to test external library behavior, focus on the wrapper/integration instead
- If Arrange and Act are identical, the test is a constructor test -- put expected values in Arrange, constructor call in Act

---

## Agile Alarm Bell

**Agile Alarm Bell:** "Let's refactor without writing tests first" is the most dangerous phrase pair in software. Refactoring without a test suite to hold behavior constant is not refactoring -- it is reckless restructuring. Stop. Write characterization tests first. Then refactor.

---

## Related

- [Testing Standards](../TESTING_STANDARDS.md) -- Project-wide test guidelines and AAA pattern
- [Test Structure: Arrange-Act-Assert](../testing-standards-aaa.md) -- Canonical AAA rules (this file's AAA section defers to it)
- [Test Conventions](test-conventions.md) -- Naming, file organization, and test double taxonomy
