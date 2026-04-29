---
title: "Code Organization and Comments"
description: "Header file layout, source file rules, class member ordering, and comment conventions for Particle-Viewer C++ code."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, organization, comments]
related:
  - "CODING_STANDARDS.md"
  - "coding-standards-naming.md"
---

# Code Organization and Comments

## Header Files

- Use include guards: `<PROJECT>_<PATH>_<FILE>_H`
- Order: includes → forward declarations → class/function declarations

```cpp
#ifndef PARTICLE_VIEWER_PARTICLE_SYSTEM_H
#define PARTICLE_VIEWER_PARTICLE_SYSTEM_H

#include <vector>
#include "particle.hpp"

class ParticleSystem { };

#endif // PARTICLE_VIEWER_PARTICLE_SYSTEM_H
```

## Source Files

- Include the corresponding header first
- Keep functions ≤50 statements
- Limit parameters to <5

## Class Member Order

1. Public → Protected → Private
2. Within each section: type definitions → constructors/destructors → methods → data members

```cpp
class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();
    void initialize();

private:
    void updatePositions();
    std::vector<Particle> particles;
    int particle_count;
};
```

## Comments

Write self-documenting code. Use comments to explain **why**, not **what**.

- Single-line: `//`
- Multi-line: `/* */`

### File Header

```cpp
/*
 * particle_system.cpp
 *
 * Implementation of the particle system that manages
 * N-body simulation particles and rendering.
 */
```

### Function Documentation

Document complex and public API functions:

```cpp
/**
 * Initializes the particle system.
 * @param particle_count Number of particles to create
 * @return true if initialization succeeded
 */
bool initializeParticles(int particle_count);
```

### Inline Comments

Use sparingly for non-obvious logic:

```cpp
// Barnes-Hut approximation for large particle counts
force = calculateGravitationalForce(particle, theta);
```

## Related

- [Coding Standards](CODING_STANDARDS.md) — parent document and full ToC
- [coding-standards-naming.md](coding-standards-naming.md) — naming conventions
- [coding-standards-modern-cpp.md](coding-standards-modern-cpp.md) — modern C++ idioms
