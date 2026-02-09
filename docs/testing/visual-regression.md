# Visual Regression Testing Guide

This document explains how to write, run, and maintain visual regression tests for Particle-Viewer.

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Writing Visual Regression Tests](#writing-visual-regression-tests)
4. [Running Tests](#running-tests)
5. [CI Integration](#ci-integration)
6. [Interpreting Test Failures](#interpreting-test-failures)
7. [Updating Baselines](#updating-baselines)
8. [Best Practices](#best-practices)

---

## Overview

Visual regression testing catches unintended rendering changes by comparing images pixel-by-pixel. The infrastructure consists of:

- **PixelComparator** (`src/testing/PixelComparator.hpp`) — RGBA image comparison with configurable tolerance
- **ImageConverter** (`src/ImageConverter.hpp`) — Image format conversion (PPM ↔ PNG)
- **VisualTestHelpers** (`tests/visual-regression/VisualTestHelpers.hpp`) — Test fixture, macros, and helper functions
- **GitHub Actions Workflow** (`.github/workflows/visual-regression.yml`) — CI pipeline with artifact upload

### Data Types

| Type | Purpose | Location |
|------|---------|----------|
| `Image` | RGBA pixel buffer (4 bytes/pixel) | `PixelComparator.hpp` |
| `ComparisonResult` | Match status, similarity, diff image | `PixelComparator.hpp` |
| `PpmData` | Parsed PPM file (RGB, 3 bytes/pixel) | `ImageConverter.hpp` |
| `ConversionResult` | Success/error for format conversion | `ImageConverter.hpp` |

The `Image` struct is the base type for all visual regression operations. It stores RGBA data in row-major order and provides `valid()` and `empty()` checks.

---

## Architecture

```
src/
├── testing/
│   └── PixelComparator.hpp/.cpp    # Image comparison engine
├── ImageConverter.hpp/.cpp          # PPM ↔ PNG conversion

tests/
└── visual-regression/
    ├── VisualTestHelpers.hpp        # Fixture + macros + helpers
    └── VisualRegressionTests.cpp    # Example tests

.github/workflows/
└── visual-regression.yml            # CI workflow
```

### Comparison Workflow

```
1. Create/load baseline image (Image struct)
2. Create/capture current image (Image struct)
3. Compare with PixelComparator
4. On failure: save diff/baseline/current as PNG artifacts
5. CI uploads artifacts and posts PR comment
```

---

## Writing Visual Regression Tests

### Using the Test Fixture

The `VisualRegressionTest` fixture provides:
- A pre-configured `PixelComparator` instance
- Output directories for diffs and artifacts
- `assertVisualMatch()` — compares images and saves diffs on failure
- `assertVisualMatchTolerant()` — uses default tolerant threshold

```cpp
#include "visual-regression/VisualTestHelpers.hpp"

TEST_F(VisualRegressionTest, RenderScene_SolidBackground_MatchesBaseline)
{
    // Arrange
    Image baseline = createTestImage(64, 64, 30, 30, 30);
    Image current = createTestImage(64, 64, 30, 30, 30);

    // Act & Assert
    assertVisualMatch(baseline, current, "solid_background");
}
```

### Using Test Macros

For simpler comparisons without the fixture:

```cpp
TEST(MyTest, Rendering_MatchesExpected)
{
    // Arrange
    Image expected = createTestImage(16, 16, 255, 0, 0);
    Image actual = createTestImage(16, 16, 255, 0, 0);

    // Act & Assert
    EXPECT_VISUAL_MATCH(expected, actual, 0.0f);       // Exact match
    EXPECT_VISUAL_MATCH(expected, actual, 2.0f/255.0f); // ±2 tolerance
}
```

### Available Macros

| Macro | Description |
|-------|-------------|
| `EXPECT_VISUAL_MATCH(baseline, current, tolerance)` | Non-fatal visual comparison |
| `ASSERT_VISUAL_MATCH(baseline, current, tolerance)` | Fatal visual comparison |

### Helper Functions

| Function | Description |
|----------|-------------|
| `createTestImage(w, h, r, g, b, a)` | Create solid-color RGBA image |
| `createGradientImage(w, h, r1, g1, b1, r2, g2, b2)` | Create horizontal gradient |
| `writeImageToPPM(path, image)` | Save RGBA Image as PPM |
| `writeImageToPNG(path, image)` | Save RGBA Image as PNG |
| `loadImageFromPPM(path)` | Load PPM file as RGBA Image |

---

## Running Tests

### Run All Tests (Including Visual Regression)

```bash
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
./build/tests/ParticleViewerTests
```

### Run Only Visual Regression Tests

```bash
./build/tests/ParticleViewerTests --gtest_filter="VisualRegressionTest.*:VisualMacroTest.*:VisualHelperTest.*"
```

### Run with Xvfb (Headless, Matching CI)

```bash
xvfb-run -a ./build/tests/ParticleViewerTests \
  --gtest_filter="VisualRegressionTest.*" \
  --gtest_output=xml:test_results.xml
```

---

## CI Integration

### Workflow: `.github/workflows/visual-regression.yml`

The workflow runs automatically on every PR and supports manual dispatch:

1. **Install dependencies** — CMake, GLFW, GLM, Mesa, Xvfb
2. **Build** — Compiles test executable
3. **Run tests** — Under Xvfb for headless OpenGL
4. **Upload artifacts** — Test results XML and diff images (on failure)
5. **Post PR comment** — Summary table with pass/fail counts and artifact links

### Artifacts

On test failure, these artifacts are uploaded:

| Artifact | Contents |
|----------|----------|
| `visual-regression-results` | Test XML output and console log |
| `visual-regression-diffs` | `*_diff.png`, `*_baseline.png`, `*_current.png` |

### Debugging CI Failures

1. Check the PR comment for the test summary
2. Download diff artifacts from the workflow run
3. Compare `*_baseline.png` with `*_current.png`
4. The `*_diff.png` highlights differing pixels in red
5. Check `visual-test-output.txt` for detailed similarity percentages

---

## Interpreting Test Failures

When a visual regression test fails, the `assertVisualMatch()` method provides:

```
Visual regression detected for 'test_name':
  Similarity: 95.5%
  Diff pixels: 234 / 4096
  Diff bounds: [10,5] to [45,30]
  Diff image: diffs/test_name_diff.png
  Baseline: artifacts/test_name_baseline.png
  Current: artifacts/test_name_current.png
```

- **Similarity**: Percentage of matching pixels (100% = identical)
- **Diff pixels**: Count of pixels that differ beyond tolerance
- **Diff bounds**: Bounding box of the region with differences
- **Diff image**: PNG with red overlay on differing pixels

---

## Updating Baselines

When a visual change is intentional:

1. Run the tests to generate `*_current.png` artifacts
2. Verify the new rendering is correct
3. Replace the baseline with the new current image
4. Re-run tests to confirm they pass

---

## Best Practices

### Test Design

- **Use the Image struct** as the base type for all image data
- **Follow AAA pattern**: Arrange (create images), Act & Assert (compare)
- **Name tests descriptively**: `RenderScene_Particles_MatchesBaseline`
- **Keep test images small** (< 64×64 for unit tests) for speed
- **Use exact match (0.0 tolerance)** for synthetic test data
- **Use tolerant match** only for real GPU-rendered content where minor variations are expected

### Tolerance Guidelines

| Scenario | Recommended Tolerance |
|----------|----------------------|
| Synthetic test images | `0.0f` (exact) |
| Software-rendered (Mesa) | `1.0f / 255.0f` (±1) |
| Cross-platform GPU | `2.0f / 255.0f` (±2) |
| Anti-aliased content | `5.0f / 255.0f` (±5) |

### AAA Pattern for Visual Tests

Follow the project's [Testing Standards](TESTING_STANDARDS.md):

```cpp
TEST_F(VisualRegressionTest, RenderParticles_DefaultConfig_MatchesBaseline)
{
    // Arrange
    Image baseline = loadImageFromPPM("baselines/particles_default.ppm");
    Image current = captureFramebuffer();  // Future: from FramebufferCapture

    // Act & Assert
    assertVisualMatch(baseline, current, "particles_default");
}
```

**Important**: If there is no meaningful Arrange step (e.g., no setup beyond creating images), omit the `// Arrange` comment rather than combining it with Act. See [Testing Standards](TESTING_STANDARDS.md) for details.
