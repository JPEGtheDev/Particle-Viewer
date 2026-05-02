---
title: "Coding Standards"
description: "C++ coding standards for Particle-Viewer: formatting, naming, organization, and tooling enforced by clang-format and clang-tidy."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, formatting, naming]
related:
  - "TESTING_STANDARDS.md"
  - "coding-standards-formatting.md"
  - "coding-standards-naming.md"
  - "coding-standards-organization.md"
  - "coding-standards-modern-cpp.md"
  - "coding-standards-tools.md"
---

# Particle-Viewer Coding Standards

Standards for all C++ code in this project, enforced by **clang-format** and **clang-tidy**. All contributors MUST follow these standards.

## Contents

| Concept | File | Coverage |
|---------|------|----------|
| Formatting | [coding-standards-formatting.md](coding-standards-formatting.md) | Indentation, braces, spacing, include order, GLAD/GLFW ordering |
| Naming | [coding-standards-naming.md](coding-standards-naming.md) | PascalCase, camelCase, snake_case for types, methods, variables |
| Organization | [coding-standards-organization.md](coding-standards-organization.md) | Header guards, source layout, class member ordering, comments |
| Modern C++ | [coding-standards-modern-cpp.md](coding-standards-modern-cpp.md) | Smart pointers, RAII, auto, range-based for, const correctness |
| Tools | [coding-standards-tools.md](coding-standards-tools.md) | clang-format/tidy usage, CI enforcement, testing expectations |

## Philosophy

This project follows the [Microsoft C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) as a foundation for clean, safe, and maintainable code.

## Quick Reference

| Rule | Value |
|------|-------|
| Indentation | 4 spaces, no tabs |
| Max line length | 120 characters |
| Functions/classes | Allman braces |
| Control structures | K&R braces |
| Types | `PascalCase` |
| Methods | `camelCase` |
| Variables | `snake_case` |
| Constants | `UPPER_CASE` |

## Related

- [Testing Standards](TESTING_STANDARDS.md) — testing expectations and patterns
- [coding-standards-formatting.md](coding-standards-formatting.md) — indentation, braces, spacing, include order
- [coding-standards-naming.md](coding-standards-naming.md) — PascalCase, camelCase, snake_case conventions
