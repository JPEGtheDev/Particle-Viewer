---
title: "Testing Standards"
description: "AAA pattern, naming conventions, mocking, and coverage expectations for all Particle-Viewer tests."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, testing, standards, gtest, aaa]
related:
  - "CODING_STANDARDS.md"
  - "testing-standards-aaa.md"
  - "testing-standards-assertions.md"
  - "testing-standards-organization.md"
  - "testing-standards-mocking.md"
  - "testing-standards-coverage.md"
---

# Particle-Viewer Testing Standards

Standards and best practices for all tests in this project, using **Google Test**. All tests MUST follow these standards.

## Contents

| Concept | File | Coverage |
|---------|------|----------|
| AAA Pattern | [testing-standards-aaa.md](testing-standards-aaa.md) | Philosophy, Arrange-Act-Assert structure, AAA rules |
| Assertions & Naming | [testing-standards-assertions.md](testing-standards-assertions.md) | Single assertion principle, test splitting, naming convention |
| Organization | [testing-standards-organization.md](testing-standards-organization.md) | Directory structure, file layout, running tests, CI |
| Mocking | [testing-standards-mocking.md](testing-standards-mocking.md) | When/how to mock, MockOpenGL design, what NOT to mock |
| Coverage | [testing-standards-coverage.md](testing-standards-coverage.md) | ≥80% target, what to test vs. skip, priority tiers |

## Core Principle

**A test MUST have only one reason to fail.** When a test fails, the cause MUST be immediately clear.

## Quick Reference

| Rule | Value |
|------|-------|
| Structure | Arrange → Act → Assert (never combined) |
| Name pattern | `UnitName_StateUnderTest_ExpectedResult` |
| Suite name | `ClassName` + `Test` (e.g., `CameraTest`) |
| Coverage target | ≥80% for tested modules |
| Individual test runtime | < 100ms |
| Full suite runtime | < 10 seconds |
| OpenGL | MUST mock — DO NOT require real GPU |

## Related

- [Coding Standards](CODING_STANDARDS.md) — formatting and naming conventions
- [testing-standards-aaa.md](testing-standards-aaa.md) — Arrange-Act-Assert structure and rules
- [testing-standards-coverage.md](testing-standards-coverage.md) — coverage targets and priorities
