---
title: "Visual Regression Testing Guide"
description: "Overview and table of contents for visual regression testing in Particle-Viewer."
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, opengl, c++]
related:
  - "../TESTING_STANDARDS.md"
  - "visual-regression-authoring.md"
  - "visual-regression-ci.md"
---

# Visual Regression Testing Guide

Visual regression testing catches unintended rendering changes by comparing images pixel-by-pixel. The infrastructure consists of:

- **Image** (`src/Image.hpp`) — Core RGBA image class with `save()`/`load()` for PPM and PNG formats
- **PixelComparator** (`src/testing/PixelComparator.hpp`) — RGBA image comparison with configurable tolerance
- **VisualTestHelpers** (`tests/visual-regression/VisualTestHelpers.hpp`) — Test fixture, macros, and helper functions
- **GitHub Actions Workflow** (`.github/workflows/unit-tests.yml`) — CI pipeline with artifact uploads

### Data Types

| Type | Purpose | Location |
|------|---------|----------|
| `Image` | RGBA pixel buffer with save/load (PPM, PNG) | `src/Image.hpp` |
| `ImageFormat` | Format enum (PPM, PNG) for Image::save/load | `src/Image.hpp` |
| `ComparisonResult` | Match status, similarity, diff image | `PixelComparator.hpp` |

The `Image` class stores RGBA data in row-major order, provides `valid()` and `empty()` checks, and handles format-specific I/O via `Image::save()` and `Image::load()`.

### Architecture

```text
src/
├── Image.hpp/.cpp                   # Core RGBA image class with save/load
├── testing/
│   └── PixelComparator.hpp/.cpp    # Image comparison engine
tests/
└── visual-regression/
    ├── VisualTestHelpers.hpp        # Fixture + macros + test image helpers
    └── VisualRegressionTests.cpp    # Visual regression tests
.github/workflows/
└── unit-tests.yml                   # CI workflow (visual-regression job)
```

## Contents

| Guide | What it covers |
|-------|----------------|
| [Writing Tests](visual-regression-authoring.md) | Fixture, macros, helpers, best practices, updating baselines |
| [Running & CI](visual-regression-ci.md) | Running tests locally, CI integration, interpreting failures |

## Related

- [Testing Standards](../TESTING_STANDARDS.md) — Project-wide test guidelines and AAA pattern
- [Writing Tests](visual-regression-authoring.md) — How to author visual regression tests
- [Running & CI](visual-regression-ci.md) — Running tests and CI integration
