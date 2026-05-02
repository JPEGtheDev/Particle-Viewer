---
title: "Writing Visual Regression Tests"
description: "How to write, structure, and maintain visual regression tests using the VisualTestHelpers fixture and baseline workflow."
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, opengl, gtest, c++]
related:
  - "visual-regression.md"
  - "visual-regression-ci.md"
  - "../TESTING_STANDARDS.md"
---

# Writing Visual Regression Tests

## Using the Test Fixture

`VisualRegressionTest` provides a pre-configured `PixelComparator`, output directories, and comparison helpers:

```cpp
#include "visual-regression/VisualTestHelpers.hpp"

TEST_F(VisualRegressionTest, RenderScene_SolidBackground_MatchesBaseline)
{
    // Arrange
    Image baseline = createTestImage(64, 64, 30, 30, 30);
    Image current  = createTestImage(64, 64, 30, 30, 30);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);

    // Assert
    EXPECT_TRUE(result.matches);
}
```

- `assertVisualMatch()` — compares and saves diffs on failure
- `assertVisualMatchTolerant()` — uses the default tolerant threshold

## Macros

| Macro | Description |
|-------|-------------|
| `EXPECT_VISUAL_MATCH(baseline, current, tolerance)` | Non-fatal visual comparison |
| `ASSERT_VISUAL_MATCH(baseline, current, tolerance)` | Fatal visual comparison |

## Helper Functions

| Function | Description |
|----------|-------------|
| `createTestImage(w, h, r, g, b, a)` | Solid-color RGBA image |
| `createGradientImage(w, h, r1, g1, b1, r2, g2, b2)` | Horizontal gradient image |
| `Image::save(path, format)` | Save image to file (PPM or PNG) |
| `Image::load(path, format)` | Load image from file (PPM or PNG) |

## Best Practices

- Follow AAA pattern: Arrange (create images), Act (compare), Assert (check result)
- **Never combine Act and Assert** into `// Act & Assert`
- Name tests descriptively: `RenderScene_Particles_MatchesBaseline`
- Keep test images small (< 64×64) for speed
- Use exact match (`0.0f` tolerance) for synthetic data
- Use tolerant match only for GPU-rendered content where minor variations are expected

### Tolerance Guidelines

| Scenario | Recommended Tolerance |
|----------|-----------------------|
| Synthetic test images | `0.0f` (exact) |
| Software-rendered (Mesa) | `1.0f / 255.0f` (±1) |
| Cross-platform GPU | `2.0f / 255.0f` (±2) |
| Anti-aliased content | `5.0f / 255.0f` (±5) |

### AAA Pattern Example

```cpp
TEST_F(VisualRegressionTest, RenderParticles_DefaultConfig_MatchesBaseline)
{
    // Arrange
    Image baseline = loadImageFromPPM("baselines/particles_default.ppm");
    Image current  = captureFramebuffer();

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);

    // Assert
    EXPECT_TRUE(result.matches);
}
```

If there is no meaningful Arrange step, omit the `// Arrange` comment rather than combining it with Act.

## Updating Baselines

When a visual change is intentional:

1. Run tests to generate `*_current.png` artifacts
2. Verify the new rendering is correct
3. Replace the baseline with the new current image
4. Re-run tests to confirm they pass

## Related

- [Visual Regression Overview](visual-regression.md) — Architecture and data types
- [Running & CI](visual-regression-ci.md) — Running tests and CI integration
- [Testing Standards](../TESTING_STANDARDS.md) — AAA pattern and project-wide test guidelines
