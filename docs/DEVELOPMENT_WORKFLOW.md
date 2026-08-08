---
title: "Development Workflow"
description: "Feature-addition, feature-removal, and bug-fix workflow patterns for Particle-Viewer, imported from the development harness."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, process, workflow, testing]
related:
  - "TESTING_STANDARDS.md"
  - "CODING_STANDARDS.md"
---

# Development Workflow

These workflow patterns were extracted from the development harness's (story-to-ship) `code-quality` skill reference file (`skills/code-quality/references/oop/cpp/cpp-toolchain.md`) and are kept here verbatim, since they depend on Particle-Viewer specifics (`Camera`, `viewer_app.cpp`, gamepad input, `isRenderingSphere()`) that do not belong in a project-neutral skill reference.

## New Feature Workflow
1. **Scan the class interface** -- before writing integration code that calls methods on an existing class, verify which members are public/private. Classes like `Camera` have a mix; don't assume public.
2. Make code changes following naming conventions
3. Add unit tests in `tests/core/` (see `docs/TESTING_STANDARDS.md`)
4. Run `clang-format -i` on ALL changed files
5. Build and verify tests pass
6. Commit: `feat: description`

## Removing Features / User-Requested Changes
When removing a gamepad feature or call site from `viewer_app.cpp` at user request, **do not also delete the supporting `Camera` public method**. The Camera API is stable across sessions; call sites in `viewer_app` change frequently with user preferences. Removing `isRenderingSphere()` when L3/R3 was dropped meant restoring it when L3/R3 came back one session later. Only remove a Camera method if it is architecturally wrong, not merely unused at the current moment.

## Bug Fix Workflow
1. Write a failing test that reproduces the bug
2. Fix the code
3. Verify test passes, run full suite
4. Run `clang-format -i` on changed files
5. Commit: `fix: description`

## Related

- [Testing Standards](TESTING_STANDARDS.md) -- AAA pattern, naming conventions, mocking, and coverage expectations for all tests
- [Coding Standards](CODING_STANDARDS.md) -- formatting and naming conventions
