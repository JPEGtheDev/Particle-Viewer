# Particle-Viewer Coding Standards

This document defines the coding standards for the Particle-Viewer project. These standards are enforced through automated tools (clang-format and clang-tidy) and should be followed by all contributors.

## Table of Contents

1. [Philosophy](#philosophy)
2. [Code Formatting](#code-formatting)
3. [Naming Conventions](#naming-conventions)
4. [Code Organization](#code-organization)
5. [Comments and Documentation](#comments-and-documentation)
6. [Modern C++ Practices](#modern-cpp-practices)
7. [Testing Expectations](#testing-expectations)
8. [Tools and Enforcement](#tools-and-enforcement)

---

## Philosophy

This project follows the **Microsoft C++ Core Guidelines** as a foundation for writing clean, safe, and maintainable code. The automated formatting and linting tools ensure consistency across the codebase.

**Key Principles:**
- **Readability First:** Code is read more often than it is written
- **Consistency:** Follow established patterns in the codebase
- **Safety:** Prefer safe constructs over unsafe alternatives
- **Simplicity:** Keep functions and classes focused on a single responsibility

---

## Code Formatting

Code formatting is handled automatically by **clang-format** using the `.clang-format` configuration file.

### Indentation
- Use **4 spaces** for indentation (no tabs)
- Indent case labels in switch statements
- Indent preprocessor directives before the hash

### Line Length
- Maximum line length: **120 characters**
- Break long lines at logical points
- Align continuation lines for readability

### Braces
- **Allman style** for functions, classes, structs, and namespaces (opening brace on new line)
- **K&R style** for control structures (opening brace on same line)

```cpp
// Functions and classes: Allman style
class ParticleSystem
{
public:
    void initialize()
    {
        // Function body
    }
};

// Control structures: K&R style
void processParticles()
{
    if (condition) {
        // if body
    } else {
        // else body
    }
    
    for (int i = 0; i < count; i++) {
        // loop body
    }
}
```

### Spacing
- One space after control structure keywords (`if`, `for`, `while`)
- No spaces inside parentheses
- Spaces around binary operators
- One space before trailing comments

```cpp
// Good
if (x > 0) {
    result = x + y;
}

// Bad
if(x>0){
    result=x+y;
}
```

### Include Order
Includes are automatically sorted and grouped:
1. Standard C++ headers (`<iostream>`, `<vector>`, etc.)
2. System headers (`<stdio.h>`)
3. External library headers (`<glm/glm.hpp>`, `<GLFW/glfw3.h>`)
4. Project headers (`"particle.hpp"`)

```cpp
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "particle.hpp"
#include "camera.hpp"
```

---

## Naming Conventions

Naming conventions are enforced by **clang-tidy** and follow Microsoft C++ style.

### Classes, Structs, Enums, and Types
- **PascalCase** (capitalize first letter of each word)

```cpp
class ParticleSystem { };
struct RenderSettings { };
enum class DrawMode { };
using VertexBuffer = std::vector<Vertex>;
```

### Functions and Methods
- **camelCase** (lowercase first letter, capitalize subsequent words) - *Note: clang-tidy enforcement relaxed*

> **Note:** While the project aims for camelCase naming for methods and functions, clang-tidy's `readability-identifier-naming` check doesn't support true camelCase (lowercase first letter). The configuration uses `aNy_CasE` for methods to avoid false positives while allowing the existing code style. New code should follow camelCase convention where possible.

```cpp
class Renderer
{
public:
    void initializeBuffers();    // camelCase (preferred)
    void drawParticles();         // camelCase (preferred)
    int getParticleCount() const; // camelCase (preferred)
private:
    void updateVertexData();
};
```

### Variables and Parameters
- **snake_case** (all lowercase with underscores)

```cpp
int particle_count = 0;
float delta_time = 0.0f;
glm::vec3 camera_position;

void setPosition(float x_pos, float y_pos, float z_pos)
{
    local_position = glm::vec3(x_pos, y_pos, z_pos);
}
```

### Constants and Macros
- **UPPER_CASE** with underscores

```cpp
const int MAX_PARTICLES = 100000;
constexpr float PI = 3.14159f;
#define WINDOW_WIDTH 1920
```

### File Names
- **snake_case** (all lowercase with underscores)
- Use `.hpp` for header files, `.cpp` for source files

```
particle_system.hpp
particle_system.cpp
render_settings.hpp
```

### Namespaces
- **snake_case** (all lowercase with underscores)

```cpp
namespace particle_viewer {
namespace rendering {
    // ...
}
}
```

---

## Code Organization

### Header Files
- Use include guards with format: `<PROJECT>_<PATH>_<FILE>_H`
- Order: includes, forward declarations, class/function declarations

```cpp
#ifndef PARTICLE_VIEWER_PARTICLE_SYSTEM_H
#define PARTICLE_VIEWER_PARTICLE_SYSTEM_H

#include <vector>
#include "particle.hpp"

class ParticleSystem
{
    // ...
};

#endif // PARTICLE_VIEWER_PARTICLE_SYSTEM_H
```

### Source Files
- Include corresponding header first
- Keep functions focused and small (≤ 50 statements maximum, fewer is better)
- Limit function parameters (< 5 recommended)

### Class Organization
Order class members by access level and category:
1. Public members
2. Protected members
3. Private members

Within each section:
1. Type definitions
2. Constructors/Destructors
3. Methods
4. Data members

```cpp
class ParticleSystem
{
public:
    // Constructors
    ParticleSystem();
    ~ParticleSystem();
    
    // Public methods
    void initialize();
    void update(float delta_time);
    
private:
    // Private methods
    void updatePositions();
    
    // Private data
    std::vector<Particle> particles;
    int particle_count;
};
```

---

## Comments and Documentation

### General Guidelines
- Write self-documenting code (clear names over comments)
- Use comments to explain **why**, not **what**
- Keep comments up-to-date with code changes
- Use `//` for single-line comments, `/* */` for multi-line

### File Headers
Include a brief header comment in each file:

```cpp
/*
 * particle_system.cpp
 *
 * Implementation of the particle system that manages
 * N-body simulation particles and rendering.
 */
```

### Function Comments
Document complex or public API functions:

```cpp
/**
 * Initializes the particle system with the specified number of particles.
 * 
 * @param particle_count Number of particles to create
 * @param initial_positions Array of initial positions (must be same size as particle_count)
 * @return true if initialization succeeded, false otherwise
 */
bool initializeParticles(int particle_count, const glm::vec3* initial_positions);
```

### Inline Comments
Use sparingly for complex logic:

```cpp
// Calculate gravitational force using Barnes-Hut approximation
// for performance with large particle counts
force = calculateGravitationalForce(particle, theta);
```

---

## Modern C++ Practices

Follow modern C++11/14/17 idioms:

### Memory Management
- Prefer smart pointers over raw pointers
- Use RAII (Resource Acquisition Is Initialization)

```cpp
// Good: automatic cleanup
std::unique_ptr<ParticleSystem> system = std::make_unique<ParticleSystem>();

// Avoid: manual memory management
ParticleSystem* system = new ParticleSystem();
// ... (easy to forget delete)
delete system;
```

### Prefer `nullptr` over `NULL`
```cpp
Particle* particle = nullptr; // Good
Particle* particle = NULL;    // Avoid
```

### Use `auto` for type inference
```cpp
auto particles = std::vector<Particle>{};
auto it = particles.begin();
```

### Range-based for loops
```cpp
// Good: use non-const reference for modifying elements
for (auto& particle : particles) {
    particle.update();
}

// Good: use const reference for read-only access
for (const auto& particle : particles) {
    renderParticle(particle);
}

// Avoid: index-based loops when range-based is clearer
for (size_t i = 0; i < particles.size(); i++) {
    particles[i].update();
}
```

### Const correctness
- Mark methods `const` if they don't modify object state
- Use `const` references for parameters that shouldn't be copied

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

---

## Testing Expectations

While formal unit testing infrastructure is still being developed, follow these practices:

1. **Manual Testing**: Test your changes manually before submitting
2. **Build Verification**: Ensure code builds without warnings on multiple platforms
3. **Smoke Tests**: Verify basic functionality works (application starts, renders, etc.)
4. **Edge Cases**: Consider boundary conditions and error cases

Future additions:
- Unit tests for core algorithms
- Integration tests for rendering pipeline
- Performance benchmarks

---

## Tools and Enforcement

### clang-format
Automatically formats code according to `.clang-format` configuration.

**Usage:**
```bash
# Format a single file
clang-format -i src/particle.cpp

# Format all C++ files
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### clang-tidy
Static analysis tool that checks for style violations, bugs, and modernization opportunities.

**Usage:**
```bash
# Analyze a file
clang-tidy src/particle.cpp -- -Isrc/glad/include

# Auto-fix issues (use with caution)
clang-tidy -fix src/particle.cpp -- -Isrc/glad/include
```

### CI Integration
GitHub Actions workflow automatically checks code formatting on pull requests:
- Verifies all files match clang-format style
- Runs clang-tidy analysis
- Fails the build if style violations are found

**Local pre-PR check:**
```bash
# Check formatting (dry-run)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# Run tidy checks
clang-tidy src/*.cpp -- -Isrc/glad/include
```

---

## References

- [Microsoft C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)

---

## Questions or Suggestions?

If you have questions about these standards or suggestions for improvements, please:
1. Open an issue on GitHub
2. Discuss in pull request comments
3. Contact the maintainers

Remember: These standards exist to help us write better code together. If a rule doesn't make sense in a specific context, discuss it with the team!
