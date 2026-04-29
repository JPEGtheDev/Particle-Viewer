---
title: "Code Quality Tools"
description: "clang-format and clang-tidy usage, CI enforcement, and testing expectations for Particle-Viewer."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, clang-format, clang-tidy, testing]
related:
  - "CODING_STANDARDS.md"
  - "TESTING_STANDARDS.md"
---

# Code Quality Tools

## clang-format

Automatically formats code according to `.clang-format`.

```bash
# Format a single file
clang-format -i src/particle.cpp

# Format all C++ files (run before every commit)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

## clang-tidy

Static analysis: checks style violations, bugs, and modernization opportunities.

```bash
# Analyze a file
clang-tidy src/particle.cpp -- -Isrc/glad/include

# Auto-fix (use with caution)
clang-tidy -fix src/particle.cpp -- -Isrc/glad/include
```

## CI Enforcement

GitHub Actions checks every pull request:

- Verifies all files match clang-format style
- Runs clang-tidy analysis
- Fails the build on violations — PRs MUST pass before merging

**Local pre-PR check:**

```bash
# Check formatting (dry-run)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# Run tidy checks
clang-tidy src/*.cpp -- -Isrc/glad/include
```

## Testing Expectations

The project uses **Google Test** for all tests:

1. **Unit Tests** — test core classes in isolation using mocks (`tests/core/`)
2. **Integration Tests** — test component interactions (`tests/integration/`)
3. **Visual Regression Tests** — compare rendered output to baselines (`tests/visual-regression/`)
4. Test naming: `UnitName_StateUnderTest_ExpectedResult`
5. All tests MUST follow Arrange-Act-Assert structure

For full testing guidelines, see `docs/TESTING_STANDARDS.md`.

## References

- [Microsoft C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)

## Related

- [Coding Standards](CODING_STANDARDS.md) — parent document and full ToC
- [Testing Standards](TESTING_STANDARDS.md) — full testing guidelines and patterns
- [coding-standards-formatting.md](coding-standards-formatting.md) — formatting rules enforced by clang-format
