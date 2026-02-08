# Copilot Instructions for Particle-Viewer

This document helps GitHub Copilot coding agent work efficiently with the Particle-Viewer repository.

## Project Overview

Particle-Viewer is a C++ OpenGL-based viewer for N-Body simulations. It allows viewing 3D particle simulations, taking screenshots, and rendering simulation videos. The project follows modern C++ practices, Microsoft C++ Core Guidelines, and uses automated semantic versioning.

**Primary Technologies:**
- C++11 with OpenGL for 3D rendering
- CMake for build configuration
- Google Test for unit testing
- GLFW and GLM libraries (must be installed locally)
- Embedded libraries: stb, Tiny File Dialogs, GLAD

## For Copilot Agents Creating Pull Requests

**CRITICAL REQUIREMENT**: When creating a PR, the PR title MUST follow conventional commits format because:
- This project uses squash merging (all commits in a PR become one commit)
- The PR title becomes the final commit message on the master branch
- The commit message determines the semantic version bump (feat → minor, fix → patch, feat! → major)

**PR Title Format**: `<type>[optional scope]: <description>`

**Examples of Valid PR Titles:**
- `feat: add particle color customization`
- `fix(camera): correct rotation calculation in moveAround method`
- `docs: update build instructions for Flatpak`
- `refactor(renderer): simplify shader loading logic`
- `feat!: redesign configuration API` (breaking change)

**Invalid PR Titles** (will fail CI):
- ❌ "Add particle color customization" (missing type)
- ❌ "Update camera rotation" (missing type)
- ❌ "Implement Flatpak distribution with automated CI/CD pipeline" (missing type)

When you create a PR, always start the title with the appropriate conventional commit type.

## For Copilot Agents Working on Existing Pull Requests

**DO NOT MODIFY THE PR TITLE** unless explicitly instructed by the user.

- The PR title represents the **overall feature or fix**, not individual tasks
- When working on subtasks within a PR, do NOT change the PR title to match the current task
- The PR title is the squash commit message and should remain stable throughout the PR lifecycle
- Only update the PR description to track progress on individual tasks

**Example Workflow:**
- PR title: `feat: add Flatpak distribution support` ✓ (keep this throughout)
- Task 1: Update build scripts ✓ (don't change PR title)
- Task 2: Add CI/CD workflow ✓ (don't change PR title)
- Task 3: Update documentation ✓ (don't change PR title)

The PR title reflects the high-level change. Individual commits and tasks are implementation details.

## Build System

### Building the Project

```bash
# Standard build (portable, works with any generator)
cmake -B build -S .
cmake --build build

# With all compiler warnings and coverage (development mode)
cmake -B build -S . -Dalloutput=ON
cmake --build build

# Run the application
./build/Viewer

# Install (copies binary and shader assets to bin/)
cmake --install build
```

### Build Requirements
- CMake 3.18 or higher
- C++11 compatible compiler with `-std=c++11 -march=native` flags
- OpenGL development libraries
- GLFW 3.3+ and GLM (must be installed on system)
- clang-format and clang-tidy (for code quality checks)

### Pull Request Requirements
- **REQUIRED**: PR title MUST use [Conventional Commits](https://www.conventionalcommits.org/) format (e.g., `feat: add feature`, `fix(scope): bug fix`)
  - **CRITICAL**: When squash merging, the PR title becomes the commit message that determines semantic version bumps
  - Example PR titles: `feat: add particle color picker`, `fix(camera): correct rotation`, `docs: update README`
- **REQUIRED**: All commits must use [Conventional Commits](https://www.conventionalcommits.org/) format (e.g., `feat:`, `fix:`, `docs:`)
- Code must pass clang-format checks
- Unit tests must pass
- Build must succeed on all platforms
- See [Version Management and Commits](#version-management-and-commits) section for full conventional commit guidelines

### Important Build Notes
- Shader files in `src/shaders/` are automatically copied to `Viewer-Assets/shaders/` during build
- Version priority: (1) PROJECT_VERSION CMake variable if set (e.g., `-DPROJECT_VERSION=1.2.3`), (2) git tags if no variable is provided (format: `v0.1.0`), or (3) fallback to `0.0.0` if neither exists
- Tests are enabled by default via `BUILD_TESTS=ON` option

## Testing

### Running Tests

```bash
# Build and run all tests
cd build
make
./tests/ParticleViewerTests

# Run specific test suite
./tests/ParticleViewerTests --gtest_filter=CameraTest.*

# Run specific test
./tests/ParticleViewerTests --gtest_filter=CameraTest.MoveForward_IncreasesPosition
```

### Test Structure and Standards

**CRITICAL: Read `docs/TESTING_STANDARDS.md` before writing tests.** Key requirements:

1. **Arrange-Act-Assert Pattern**: Every test MUST follow AAA structure
2. **Single Assertion Principle**: Each test verifies one logical concept
3. **Test Naming**: Use format `UnitName_StateUnderTest_ExpectedResult`
4. **Mock External Dependencies**: Use `MockOpenGL` for OpenGL calls (see `tests/mocks/`)
5. **Do NOT test external libraries**: Only test YOUR code, not std:: or third-party libraries

**Test Organization:**
- `tests/core/` - Unit tests for core classes (Camera, Shader, Particle, SettingsIO)
- `tests/mocks/` - Mock implementations (MockOpenGL, etc.)
- Each test file corresponds to a core implementation unit (e.g., `CameraTests.cpp` for `camera.hpp`)

**Example Test:**
```cpp
TEST(CameraTest, MoveForward_IncreasesZPosition)
{
    // Arrange
    Camera camera(800, 600);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.setSpeed(1.0f);
    
    // Act
    camera.moveForward();
    
    // Assert
    EXPECT_LT(camera.cameraPos.z, 0.0f);
}
```

**Coverage Target**: ≥80% for tested modules, prioritizing core logic, public APIs, and error handling.

## Code Standards

### Formatting (clang-format)

**Auto-formatted via `.clang-format`** - DO NOT manually format code.

```bash
# Format all C++ files
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting (CI check)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror
```

**Key Rules:**
- 4 spaces indentation (no tabs)
- 120 character line length
- Allman style for functions/classes (brace on new line)
- K&R style for control structures (brace on same line)
- Include order: C++ standard → system → external libs → project headers

### Naming Conventions (clang-tidy + style guide)

- **Classes/Structs/Enums** (clang-tidy enforced): `PascalCase` (e.g., `ParticleSystem`, `RenderSettings`)
- **Functions/Methods** (preferred, not enforced by clang-tidy): `camelCase` (e.g., `initializeBuffers`, `getParticleCount`)
- **Variables/Parameters** (clang-tidy enforced): `snake_case` (e.g., `particle_count`, `delta_time`)
- **Constants/Macros** (clang-tidy enforced): `UPPER_CASE` (e.g., `MAX_PARTICLES`, `PI`)
- **File Names**: `snake_case` (e.g., `particle_system.hpp`, `particle_system.cpp`)
- **Include Guards**: `<PROJECT>_<PATH>_<FILE>_H` (e.g., `PARTICLE_VIEWER_PARTICLE_SYSTEM_H`)

### Static Analysis (clang-tidy)

```bash
# Analyze main translation unit
clang-tidy src/main.cpp -- -Isrc/glad/include

# Or analyze a header using the compilation database (from the build/ directory):
# clang-tidy src/particle.hpp -p build

# Auto-fix (use carefully)
clang-tidy -fix src/main.cpp -- -Isrc/glad/include
```

Configuration in `.clang-tidy` enforces:
- Microsoft C++ Core Guidelines
- Modern C++ practices (smart pointers, nullptr, auto, range-based loops)
- Const correctness
- Function complexity limits (≤50 statements, <5 parameters recommended)

### CI Checks

**GitHub Actions automatically check:**
- Code formatting with clang-format
- Static analysis with clang-tidy (runs in CI as a non-blocking, informational check)
- Unit tests pass
- Build succeeds on multiple platforms

**All blocking checks (build, tests, formatting) MUST pass before merging PRs. clang-tidy is currently advisory and does not block merges.**

## Version Management and Commits

### Automated Semantic Versioning

**Zero-manual releases** - versions are fully automated via GitHub Actions.

**DO:**
- Use [Conventional Commits](https://www.conventionalcommits.org/) format for ALL commits
- Push to master triggers automatic release
- Let workflow determine version bumps

**DON'T:**
- Manually create git tags
- Manually edit versions in CMakeLists.txt (version comes from git tags)
- Create manual releases

### Conventional Commit Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer]
```

**Types and Version Bumps:**
- `feat:` → MINOR version bump (0.1.0 → 0.2.0)
- `fix:` → PATCH version bump (0.1.0 → 0.1.1)
- `<type>!:` (e.g., `feat!:`, `fix!:`) or `BREAKING CHANGE:` → MAJOR version bump (0.1.0 → 1.0.0)
- Other types (`docs:`, `test:`, `chore:`, etc.) → PATCH version bump (default fallback)

**Note**: The release workflow detects `feat:`, `fix:`, and any `<type>!:` breaking changes. Commits without these types still trigger a PATCH bump to ensure every push to master creates a release.

**Examples:**
```bash
# Patch release
git commit -m "fix: resolve memory leak in particle system"

# Minor release
git commit -m "feat: add particle color customization"

# Major release (breaking change)
git commit -m "feat!: redesign configuration API"
# OR with footer
git commit -m "feat: redesign configuration API

BREAKING CHANGE: Config file format changed from JSON to YAML"
```

**Commit Tips:**
- Present tense ("add" not "added")
- Lowercase after colon
- No trailing period
- Keep description concise and clear

## Source Code Organization

### Directory Structure

```
src/
├── *.cpp, *.hpp        # Main source files
├── shaders/            # GLSL shader files (auto-copied during build)
├── glad/               # GLAD OpenGL loader (embedded)
└── tinyFileDialogs/    # File dialog library (embedded)

tests/
├── core/               # Unit tests for main classes
├── mocks/              # Mock implementations
└── CMakeLists.txt      # Test build configuration

docs/
├── CODING_STANDARDS.md      # Detailed code standards
├── TESTING_STANDARDS.md     # Testing best practices
├── RELEASE_PROCESS.md       # Release automation details
└── CONVENTIONAL_COMMITS.md  # Commit format quick reference

.github/
├── workflows/          # CI/CD pipelines
├── copilot-instructions.md  # This file
└── skills/             # Custom Copilot skills
```

### Key Source Files

- `main.cpp` - Application entry point
- `camera.hpp` - Header-only camera system for 3D navigation
- `shader.hpp` - Header-only shader program management
- `particle.hpp` - Header-only particle data structures
- `settingsIO.hpp` - Header-only configuration file I/O
- `osFile.hpp` - OS-specific file operations
- `clutter.hpp` - Utility functions

## Common Tasks and Commands

### Code Quality Checks (Before Committing)

```bash
# Format all code
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting (what CI runs)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# Run static analysis
clang-tidy src/main.cpp -- -Isrc/glad/include

# Build and test
cmake -B build -S .
cmake --build build
./build/tests/ParticleViewerTests
```

### Adding a New Feature

1. Create feature branch (if not using Copilot PR)
2. Make code changes following naming conventions
3. Add unit tests in `tests/core/` following AAA pattern
4. Run clang-format on changed files
5. Build and verify tests pass
6. Commit with conventional commit format (e.g., `feat: add new feature`)
7. CI checks will run automatically on PR

### Fixing a Bug

1. Write a failing test that reproduces the bug
2. Fix the code
3. Verify test now passes
4. Run full test suite
5. Format code with clang-format
6. Commit with format: `fix: description of what was fixed`

## Common Pitfalls and Solutions

### Build Issues

**Problem**: Missing GLFW or GLM libraries
**Solution**: Install system-wide: `sudo apt-get install libglfw3-dev libglm-dev` (Ubuntu/Debian) or equivalent for your OS

**Problem**: OpenGL headers not found
**Solution**: Install OpenGL development packages and ensure `OpenGL::GL` target is available

**Problem**: Shader files not found at runtime
**Solution**: Run from build directory or ensure `Viewer-Assets/shaders/` directory exists with shader files

### Test Issues

**Problem**: Tests fail due to missing OpenGL context
**Solution**: Tests should use `MockOpenGL` from `tests/mocks/` - NEVER require actual GPU/OpenGL

**Problem**: Test name doesn't follow convention
**Solution**: Use format `UnitName_StateUnderTest_ExpectedResult` (e.g., `MoveForward_IncreasesPosition`)

### Code Style Issues

**Problem**: CI formatting check fails
**Solution**: Run `find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i` before committing

**Problem**: clang-tidy warnings
**Solution**: Review `.clang-tidy` configuration and fix issues or suppress false positives with `// NOLINT`

### Version/Release Issues

**Problem**: Need to create a release
**Solution**: Just push conventional commits to master - release is automatic. Use `feat:` for minor, `fix:` for patch, `feat!:` for major.

**Problem**: Wrong version bump occurred
**Solution**: Verify commit message uses correct conventional commit type. Manual override via workflow_dispatch if needed.

## Additional Resources

- **Full Coding Standards**: `docs/CODING_STANDARDS.md`
- **Testing Standards**: `docs/TESTING_STANDARDS.md`
- **Release Process**: `docs/RELEASE_PROCESS.md`
- **Conventional Commits Quick Ref**: `docs/CONVENTIONAL_COMMITS.md`
- **Microsoft C++ Core Guidelines**: https://isocpp.github.io/CppCoreGuidelines/
- **Google Test Docs**: https://google.github.io/googletest/

## Project-Specific Patterns

### Error Handling
- Check return values from OpenGL and file I/O operations
- Log errors to console with descriptive messages
- Use assertions for preconditions and invariants

### Memory Management
- Prefer stack allocation over heap when possible
- Use RAII for resource management
- Smart pointers for dynamic memory (when C++11 features are used)
- Let OpenGL manage buffer lifecycle

### OpenGL Usage
- Check OpenGL errors after major operations
- Use GLAD for OpenGL function loading
- Shaders are loaded from `Viewer-Assets/shaders/` directory
- Vertex data should use modern VBO/VAO patterns

## Tips for Efficient Work

1. **Read documentation first**: Check `docs/` before making assumptions
2. **Follow existing patterns**: Look at existing code for consistency (e.g., how other tests are written)
3. **Use tools**: Let clang-format handle formatting, don't do it manually
4. **Test incrementally**: Run tests frequently during development
5. **Commit properly**: Use conventional commits - your commit messages become release notes
6. **Check CI early**: Run local formatting/tidy checks before pushing to catch CI failures early

## When in Doubt

- Consult `docs/CODING_STANDARDS.md` for code style questions
- Consult `docs/TESTING_STANDARDS.md` for test-writing questions
- Check existing similar code for patterns
- Run clang-format and clang-tidy to catch issues
- Build and test locally before pushing
