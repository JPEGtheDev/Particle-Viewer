---
title: "Naming Conventions"
description: "PascalCase, camelCase, snake_case, and UPPER_CASE naming rules for types, methods, variables, and files in Particle-Viewer."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, naming, clang-tidy]
related:
  - "CODING_STANDARDS.md"
  - "coding-standards-formatting.md"
---

# Naming Conventions

Naming conventions are enforced by **clang-tidy** and follow Microsoft C++ style.

## Types, Structs, Enums

**PascalCase** — capitalize first letter of each word:

```cpp
class ParticleSystem { };
struct RenderSettings { };
enum class DrawMode { };
using VertexBuffer = std::vector<Vertex>;
```

## Functions and Methods

**camelCase** — lowercase first letter, capitalize subsequent words:

```cpp
void initializeBuffers();
void drawParticles();
int getParticleCount() const;
```

> clang-tidy's `readability-identifier-naming` does not support true camelCase. The config uses `aNy_CasE` to avoid false positives. New code MUST still follow camelCase.

## Variables and Parameters

**snake_case** — all lowercase with underscores:

```cpp
int particle_count = 0;
float delta_time = 0.0f;
glm::vec3 camera_position;
```

## Constants and Macros

**UPPER_CASE** with underscores:

```cpp
const int MAX_PARTICLES = 100000;
constexpr float PI = 3.14159f;
```

## File Names

**snake_case**, `.hpp` for headers, `.cpp` for source:

```
particle_system.hpp
particle_system.cpp
```

## Namespaces

**snake_case**:

```cpp
namespace particle_viewer {
namespace rendering { }
}
```

## Summary Table

| Entity | Style | Example |
|--------|-------|---------|
| Class/Struct/Enum | PascalCase | `ParticleSystem` |
| Function/Method | camelCase | `drawParticles()` |
| Variable/Parameter | snake_case | `particle_count` |
| Constant/Macro | UPPER_CASE | `MAX_PARTICLES` |
| File | snake_case | `particle_system.hpp` |
| Namespace | snake_case | `particle_viewer` |

## Related

- [Coding Standards](CODING_STANDARDS.md) — parent document and full ToC
- [coding-standards-formatting.md](coding-standards-formatting.md) — brace style, spacing, include order
- [coding-standards-tools.md](coding-standards-tools.md) — clang-tidy enforcement
