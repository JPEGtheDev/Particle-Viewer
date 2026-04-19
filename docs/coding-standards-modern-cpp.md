---
title: "Modern C++ Practices"
description: "C++20 idioms required in Particle-Viewer: smart pointers, RAII, auto, range-based for, and const correctness."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, standards, cpp, modern-cpp, raii]
related:
  - "CODING_STANDARDS.md"
  - "coding-standards-organization.md"
---

# Modern C++ Practices

This project targets **C++20**. Apply these idioms in all new code.

## Memory Management

USE smart pointers. DO NOT use manual `new`/`delete`.

```cpp
// CORRECT — automatic cleanup
std::unique_ptr<ParticleSystem> system = std::make_unique<ParticleSystem>();

// WRONG — manual memory management
ParticleSystem* system = new ParticleSystem();
delete system;
```

USE RAII: acquire in constructor, release in destructor.

## nullptr

USE `nullptr`. DO NOT use `NULL`.

```cpp
Particle* particle = nullptr; // CORRECT
Particle* particle = NULL;    // WRONG
```

## auto

USE `auto` for type inference where the type is obvious from context:

```cpp
auto particles = std::vector<Particle>{};
auto it = particles.begin();
```

## Range-Based For Loops

USE range-based `for` instead of index-based loops:

```cpp
// Modifying — use non-const reference
for (auto& particle : particles) {
    particle.update();
}

// Read-only — use const reference
for (const auto& particle : particles) {
    renderParticle(particle);
}
```

## Const Correctness

- Mark methods `const` when they do not modify object state
- USE `const` references for read-only parameters

```cpp
class Particle
{
public:
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }
private:
    glm::vec3 position;
};
```

## Related

- [Coding Standards](CODING_STANDARDS.md) — parent document and full ToC
- [coding-standards-organization.md](coding-standards-organization.md) — header layout and class member ordering
- [coding-standards-tools.md](coding-standards-tools.md) — clang-tidy enforcement of these practices
