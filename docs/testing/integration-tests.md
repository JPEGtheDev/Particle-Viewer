---
title: "Integration Tests Guide"
description: "Overview and table of contents for integration testing in Particle-Viewer."
domain: testing
subdomain: integration-testing
tags: [testing, integration-testing, gtest, mocks, c++]
related:
  - "../TESTING_STANDARDS.md"
  - "integration-tests-patterns.md"
  - "integration-tests-mocking.md"
---

# Integration Tests Guide

Integration tests verify that multiple components of the Particle-Viewer work correctly together. Unlike unit tests, they test data flow through the viewer pipeline:

- **Data Loading Pipeline**: SettingsIO reads file → Particle receives translations → VBO updated
- **Shader Pipeline**: Load shader source → compile vertex+fragment → link program
- **Frame Playback**: Load frame N → render → advance → load frame N+1

### Directory Structure

```
tests/
├── core/          # Unit tests for individual classes
├── integration/   # Integration tests for pipelines
│   ├── DataLoadingPipelineTests.cpp
│   ├── ShaderPipelineTests.cpp
│   └── FramePlaybackTests.cpp
├── mocks/         # MockOpenGL.hpp/.cpp
└── CMakeLists.txt
```

### Build and Run All Tests

```bash
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
./build/tests/ParticleViewerTests
```

## Contents

| Guide | What it covers |
|-------|----------------|
| [Test Patterns](integration-tests-patterns.md) | Fixture structure, pipeline patterns, adding new tests |
| [Mocking & Troubleshooting](integration-tests-mocking.md) | MockOpenGL setup, coverage notes, common issues |

## Related

- [Testing Standards](../TESTING_STANDARDS.md) — Project-wide test guidelines and AAA pattern
- [Test Patterns](integration-tests-patterns.md) — How to write and structure integration tests
- [Mocking & Troubleshooting](integration-tests-mocking.md) — MockOpenGL and debugging guidance
