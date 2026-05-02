---
title: "Visual Regression Testing: CI and Running Tests"
description: "How to run visual regression tests locally and in CI, and how to interpret test failures and artifacts."
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, ci, github-actions, c++]
related:
  - "visual-regression.md"
  - "visual-regression-authoring.md"
  - "../TESTING_STANDARDS.md"
---

# Visual Regression Testing: CI and Running Tests

## Running Tests Locally

### Run All Tests

```bash
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
./build/tests/ParticleViewerTests
```

### Run Only Visual Regression Tests

```bash
./build/tests/ParticleViewerTests --gtest_filter="VisualRegressionTest.*:VisualMacroTest.*:VisualHelperTest.*"
```

### Run Headless with Xvfb (Matches CI)

```bash
xvfb-run -a ./build/tests/ParticleViewerTests \
  --gtest_filter="VisualRegressionTest.*" \
  --gtest_output=xml:test_results.xml
```

## CI Integration

The visual regression job is defined in `.github/workflows/unit-tests.yml` and runs **after unit tests pass**:

1. **Unit tests pass** — The `test` job must succeed first
2. **Install dependencies** — CMake, GLFW, GLM, Mesa, Xvfb
3. **Build** — Compiles test executable
4. **Run tests** — Under Xvfb for headless OpenGL (`VisualRegressionTest.*` only)
5. **Upload artifacts** — Images and test results retained for 30 days
6. **Post PR comment** — Summary table with pass/fail counts and artifact download link

### Artifacts

| Artifact | Contents | When uploaded |
|----------|----------|---------------|
| `visual-regression-images` | `*_current.png` | Always |
| `visual-regression-images` | `*_diff.png`, `*_baseline.png` | On failure only |
| `visual-regression-results` | XML output and console log | Always |

> **Note:** GitHub strips inline `data:image/png;base64,...` from PR comments. Download images from the artifact link in the PR comment.

## Interpreting Test Failures

When a test fails, `assertVisualMatch()` outputs:

```
Visual regression detected for 'test_name':
  Similarity: 95.5%
  Diff pixels: 234 / 4096
  Diff bounds: [10,5] to [45,30]
  Diff image: diffs/test_name_diff.png
  Baseline: artifacts/test_name_baseline.png
  Current: artifacts/test_name_current.png
```

| Field | Meaning |
|-------|---------|
| Similarity | Percentage of matching pixels (100% = identical) |
| Diff pixels | Count of pixels differing beyond tolerance |
| Diff bounds | Bounding box of the changed region |
| Diff image | PNG with red overlay on differing pixels |

### Debugging CI Failures

1. Check the PR comment for the test summary and artifact link
2. Download `visual-regression-images` from the workflow run
3. Inspect `*_diff.png` — differing pixels are highlighted in red
4. Compare `*_baseline.png` with `*_current.png` side-by-side
5. Check `visual-test-output.txt` for detailed similarity percentages

If the difference is intentional (e.g., a rendering improvement), follow the [baseline update workflow](visual-regression-authoring.md#updating-baselines).

## Related

- [Visual Regression Overview](visual-regression.md) — Architecture and data types
- [Writing Tests](visual-regression-authoring.md) — Fixture, macros, helpers, and best practices
- [Testing Standards](../TESTING_STANDARDS.md) — Project-wide test guidelines
