---
title: "Test Organization and Running Tests"
description: "Directory structure, file layout, and commands for running Particle-Viewer tests locally and in CI."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, organization, ci, gtest]
related:
  - "TESTING_STANDARDS.md"
  - "testing-standards-mocking.md"
---

# Test Organization and Running Tests

## Directory Structure

```
tests/
├── core/               — unit tests for src/ classes
│   ├── CameraTests.cpp
│   ├── ShaderTests.cpp
│   └── ParticleTests.cpp
├── integration/        — multi-component tests
│   ├── DataLoadingPipelineTests.cpp
│   └── ShaderPipelineTests.cpp
├── testing/            — tests for test utilities
│   ├── PixelComparatorTests.cpp
│   └── ImageTests.cpp
├── visual-regression/  — headless visual output tests
│   └── VisualRegressionTests.cpp
└── mocks/
    ├── MockOpenGL.hpp
    └── MockOpenGL.cpp
```

## File Layout

Each test file MUST:

1. Include necessary headers
2. Include mock headers
3. Group related tests in the same suite
4. Order tests: basic → complex → edge cases

```cpp
#include <gtest/gtest.h>
#include "camera.hpp"
#include "mocks/MockOpenGL.hpp"

TEST(CameraTest, Constructor_InitializesDefaultValues) { }
TEST(CameraTest, MoveForward_IncreasesPosition) { }
TEST(CameraTest, Pitch_Above89_IsClamped) { }
```

## Running Tests Locally

```bash
# Build
mkdir build && cd build && cmake .. && make

# Run all tests
./tests/ParticleViewerTests

# Run a specific suite
./tests/ParticleViewerTests --gtest_filter=CameraTest.*

# Run a specific test
./tests/ParticleViewerTests --gtest_filter=CameraTest.MoveForward_IncreasesPosition

# Verbose output
./tests/ParticleViewerTests --gtest_verbose=1
```

## CI Testing

Tests run automatically on every pull request:

- All tests MUST pass before PR approval
- Coverage reports are generated and displayed
- Failures block merging

## Performance Requirements

- Individual tests: MUST complete in < 100ms
- Full test suite: MUST complete in < 10 seconds
- If slow: reduce setup overhead, use simpler test data, mock expensive operations

## Related

- [Testing Standards](TESTING_STANDARDS.md) — parent ToC stub
- [testing-standards-mocking.md](testing-standards-mocking.md) — mocking design and OpenGL mocks
- [testing-standards-coverage.md](testing-standards-coverage.md) — coverage targets and priorities
