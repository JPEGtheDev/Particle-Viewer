---
title: "Particle-Viewer Docs Index"
description: "Index of all documentation files in docs/ -- coding standards, testing standards, release process, architecture, and process/planning guides."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, docs, index]
related:
  - "PROJECT_CONTEXT.md"
  - "testing/INDEX.md"
---

# Particle-Viewer Docs Index

Catalog of every documentation file directly under `docs/`. Integration-test and visual-regression-test guides live under `docs/testing/` -- see [testing/INDEX.md](testing/INDEX.md) for that sub-index.

---

## Coding Standards

| File | Covers |
|------|--------|
| `CODING_STANDARDS.md` | C++ coding standards: formatting, naming, organization, and tooling enforced by clang-format and clang-tidy. |
| `coding-standards-formatting.md` | Indentation, brace style, spacing, line length, and include order rules enforced by clang-format. |
| `coding-standards-naming.md` | PascalCase, camelCase, snake_case, and UPPER_CASE naming rules for types, methods, variables, and files. |
| `coding-standards-organization.md` | Header file layout, source file rules, class member ordering, and comment conventions. |
| `coding-standards-modern-cpp.md` | C++20 idioms required in the project: smart pointers, RAII, auto, range-based for, and const correctness. |
| `coding-standards-tools.md` | clang-format and clang-tidy usage, CI enforcement, and testing expectations. |

## Testing Standards

| File | Covers |
|------|--------|
| `TESTING_STANDARDS.md` | AAA pattern, naming conventions, mocking, and coverage expectations for all tests. |
| `testing-standards-aaa.md` | AAA pattern rules and examples for structuring all tests. |
| `testing-standards-assertions.md` | Single assertion principle, when to split tests, and the UnitName_StateUnderTest_ExpectedResult naming pattern. |
| `testing-standards-organization.md` | Directory structure, file layout, and commands for running tests locally and in CI. |
| `testing-standards-mocking.md` | When and how to mock OpenGL and other external dependencies in tests. |
| `testing-standards-coverage.md` | Minimum coverage targets, what to test vs. skip, and priority guidelines. |

## Testing Sub-Index

| File | Covers |
|------|--------|
| `testing/INDEX.md` | Sub-index of integration-test and visual-regression-test guides in `docs/testing/`. |

## Release Process

| File | Covers |
|------|--------|
| `RELEASE_PROCESS.md` | Automated semantic versioning and release pipeline using conventional commits and GitHub Actions. |
| `release-process-automation.md` | How GitHub Actions detects source changes, calculates versions, and creates releases. |
| `release-process-operations.md` | Zero-manual release requirements, workflow configuration, troubleshooting guide, and best practices. |
| `release-process-versioning.md` | How commit types map to MAJOR/MINOR/PATCH version bumps in the release pipeline. |
| `CONVENTIONAL_COMMITS.md` | Commit message format rules and type/scope conventions. |

## Architecture & Subsystems

| File | Covers |
|------|--------|
| `ARCHITECTURE.md` | Layered inward-dependency model, file-to-layer mapping, dependency rules, and Barricade Model zones. |
| `CONTROLLER_MAPPING.md` | Gamepad button layout and input mappings, covering camera, playback, and file controls with keyboard equivalents. |
| `IMGUI_INTEGRATION.md` | Patterns for integrating Dear ImGui -- FetchContent setup, SDL3 event order, menu system, and debug overlay positioning. |
| `WINDOW_MANAGEMENT.md` | How to configure window resolution, fullscreen mode, and persistence settings. |

## Process & Planning

| File | Covers |
|------|--------|
| `PROJECT_CONTEXT.md` | What Particle Viewer is and is not: scope, architecture overview, component inventory, tech stack, and refactor priorities. |
| `PROJECT_SCOPE.md` | Scope gate: what the project is (viewer) and is not (simulator), and the project-context loading guide. |
| `DONE_DEFINITION.md` | What each completion stage means for a desktop application with no deploy stage. |
| `ESTIMATION_EXAMPLES.md` | Validated real-world effort estimates from completed stories, for calibrating new estimates against actual outcomes. |
| `VERIFICATION_COMMANDS.md` | Build, test, and format commands to run before every commit. |
| `DEBUGGING.md` | Debug commands for build, test filtering, visual regression diffs, and CI reproduction. |

---

## Related

- [Project Context](PROJECT_CONTEXT.md) -- what Particle Viewer is and how these docs fit together
- [Testing Sub-Index](testing/INDEX.md) -- integration-test and visual-regression-test guides
