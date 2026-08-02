---
title: "Particle-Viewer Testing Docs Index"
description: "Index of documentation files in docs/testing/ -- integration test guides, visual regression testing, test conventions, and worked examples."
domain: testing
subdomain: ""
tags: [testing, visual-regression, integration-testing, index]
related:
  - "../INDEX.md"
  - "../TESTING_STANDARDS.md"
---

# Particle-Viewer Testing Docs Index

Catalog of every documentation file under `docs/testing/`. Project-wide test standards (AAA pattern, naming, mocking, coverage) live in [../TESTING_STANDARDS.md](../TESTING_STANDARDS.md).

---

## Integration Testing

| File | Covers |
|------|--------|
| `integration-tests.md` | Overview and table of contents for integration testing. |
| `integration-tests-mocking.md` | MockOpenGL setup, coverage considerations, and troubleshooting for integration tests. |
| `integration-tests-patterns.md` | Test patterns, fixture structure, and how to add new integration tests. |

## Visual Regression Testing

| File | Covers |
|------|--------|
| `visual-regression.md` | Overview and table of contents for visual regression testing. |
| `visual-regression-authoring.md` | How to write, structure, and maintain visual regression tests using the VisualTestHelpers fixture and baseline workflow. |
| `visual-regression-ci.md` | How to run visual regression tests locally and in CI, and how to interpret test failures and artifacts. |
| `qualitative-visual-analysis.md` | Render-and-inspect debugging: the agent uses its vision capability to analyze rendered screenshots directly. |

## Conventions & Examples

| File | Covers |
|------|--------|
| `test-conventions.md` | Project test conventions: naming, file organization, test double taxonomy, test sizes, and design principles. |
| `testing-examples.md` | Concrete correct/incorrect test examples: AAA pattern, naming, mocking, assertions, and key type references. |

---

## Related

- [Docs Index](../INDEX.md) -- top-level documentation index
- [Testing Standards](../TESTING_STANDARDS.md) -- project-wide test guidelines and AAA pattern
