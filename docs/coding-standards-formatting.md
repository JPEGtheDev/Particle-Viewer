---
title: "Code Formatting Standards"
description: "Indentation, brace style, spacing, line length, and include order rules enforced by clang-format."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, formatting, clang-format]
related:
  - "CODING_STANDARDS.md"
  - "coding-standards-naming.md"
---

# Code Formatting Standards

All formatting is enforced by **clang-format** via `.clang-format`. Run it before every commit.

## Indentation

- 4 spaces per level — NO tabs
- Indent `case` labels in switch statements
- Indent preprocessor directives before the hash

## Line Length

- Maximum: **120 characters**
- Break long lines at logical points

## Brace Style

- **Allman** for functions, classes, structs, and namespaces — opening brace on new line
- **K&R** for control structures — opening brace on same line

```cpp
class ParticleSystem
{
public:
    void initialize()
    {
    }
};

void processParticles()
{
    if (condition) {
        // body
    }
    for (int i = 0; i < count; i++) {
        // body
    }
}
```

## Spacing

- One space after control keywords (`if`, `for`, `while`)
- No spaces inside parentheses
- Spaces around binary operators
- One space before trailing comments

## Include Order

clang-format sorts includes into these groups:

1. Standard C++ headers (`<iostream>`, `<vector>`)
2. System headers (`<stdio.h>`)
3. External library headers (`<glm/glm.hpp>`)
4. Project headers (`"particle.hpp"`)

## GLAD/GLFW Ordering

GLAD MUST come before GLFW. GLAD populates OpenGL function pointers that GLFW headers depend on.

```cpp
// CORRECT
#include "glad/glad.h"       // NOLINT(llvm-include-order)
#include <GLFW/glfw3.h>      // NOLINT(llvm-include-order)

// WRONG — do not put GLFW first
#include <GLFW/glfw3.h>
#include "glad/glad.h"
```

Use `// NOLINT(llvm-include-order)` to suppress the clang-tidy warning for this intentional deviation.

## Related

- [Coding Standards](CODING_STANDARDS.md) — parent document and full ToC
- [coding-standards-naming.md](coding-standards-naming.md) — naming conventions for types, methods, variables
- [coding-standards-tools.md](coding-standards-tools.md) — clang-format and clang-tidy usage
