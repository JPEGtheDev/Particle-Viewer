---
title: "Verification Commands"
description: "Build, test, and format commands to run before every commit in Particle Viewer."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, process, verification, build, testing]
related:
  - "DONE_DEFINITION.md"
  - "DEBUGGING.md"
---

# Particle-Viewer Verification Commands

## The Verification Commands

> **Note:** These commands use Particle Viewer's build system (CMake) and test runner (`ParticleViewerTests`, built on GoogleTest).

```bash
# Build
cmake --build build

# Tests
./build/tests/ParticleViewerTests

# Run a specific test
./build/tests/ParticleViewerTests --gtest_filter=TestSuite.TestName

# Format check
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# Full pre-commit gate (run all three before every commit)
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
cmake --build build && ./build/tests/ParticleViewerTests
```

## Related

- [Done Definition](DONE_DEFINITION.md) -- the stage vocabulary that references this gate
- [Debugging Reference](DEBUGGING.md) -- debug commands for build, test filtering, and CI reproduction
