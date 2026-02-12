# Copilot Instructions for Particle-Viewer

This document helps GitHub Copilot coding agent work efficiently with the Particle-Viewer repository.

## Project Overview

Particle-Viewer is a C++ OpenGL-based viewer for N-Body simulations. It allows viewing 3D particle simulations, taking screenshots, and rendering simulation videos. The project follows modern C++ practices, Microsoft C++ Core Guidelines, and uses automated semantic versioning.

**Primary Technologies:**
- C++20 with OpenGL for 3D rendering
- CMake for build configuration
- Google Test for unit testing
- GLFW and GLM libraries (must be installed locally)
- Dear ImGui for GUI menus and debug tools (downloaded via CMake FetchContent)
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

**Invalid PR Titles** (will produce an incorrect squash commit message and version bump):
- ❌ "Add particle color customization" (missing type)
- ❌ "Update camera rotation" (missing type)
- ❌ "Implement Flatpak distribution with automated CI/CD pipeline" (missing type)

When you create a PR, always start the title with the appropriate conventional commit type so the release workflow can compute the correct semantic version bump.

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
- CMake 3.24 or higher
- C++20 compatible compiler (GCC >= 10, Clang >= 11, MSVC 2019+/VS2022)
- OpenGL development libraries
- GLFW 3.3+ and GLM (must be installed on system)
- clang-format and clang-tidy (for code quality checks)

### Pull Request Requirements
- **REQUIRED**: PR title MUST use [Conventional Commits](https://www.conventionalcommits.org/) format (e.g., `feat: add feature`, `fix(scope): bug fix`)
  - **CRITICAL**: When squash merging, the PR title becomes the commit message that determines semantic version bumps
  - Example PR titles: `feat: add particle color picker`, `fix(camera): correct rotation`, `docs: update README`
- **REQUIRED**: All commits must use [Conventional Commits](https://www.conventionalcommits.org/) format (e.g., `feat:`, `fix:`, `docs:`)
- **REQUIRED**: Code MUST be formatted with clang-format before committing (see [Code Quality Checks](#code-quality-checks-before-committing))
- Code must pass clang-format checks in CI
- Unit tests must pass
- Build must succeed on all platforms
- See [Version Management and Commits](#version-management-and-commits) section for full conventional commit guidelines

### Important Build Notes
- Shader files in `src/shaders/` are automatically copied to `Viewer-Assets/shaders/` during build
- Version priority: (1) PROJECT_VERSION CMake variable if set (e.g., `-DPROJECT_VERSION=1.2.3`), (2) git tags if no variable is provided (format: `v0.1.0`), or (3) fallback to `0.0.0` if neither exists
- Tests are enabled by default via `BUILD_TESTS=ON` option

### Dependency Upgrades
- **Upgrades are allowed** as long as all tests still pass and the build succeeds
- When upgrading, verify with confidence that existing tests catch regressions
- **Visual regression tests are a hard requirement** — they must pass and must NOT be modified unless there is an explicit notification to the approver showing the new output
- Upgrades may require test updates (e.g., API changes in GoogleTest) — this is acceptable

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

# Run visual regression tests (headless - no display/GPU required)
xvfb-run -a ./tests/ParticleViewerTests --gtest_filter="RenderingRegressionTest.*"
```

### Test Structure and Standards

**CRITICAL: Read `docs/TESTING_STANDARDS.md` before writing tests.** For detailed guidelines, examples, and patterns, use the `testing` skill (`.github/skills/testing/`).

Key requirements:

1. **Arrange-Act-Assert Pattern**: Every test MUST follow AAA structure with all three phases separate
2. **Single Assertion Principle**: Each test verifies one logical concept
3. **Test Naming**: Use format `UnitName_StateUnderTest_ExpectedResult`
4. **Mock External Dependencies**: Use `MockOpenGL` for OpenGL calls (see `tests/mocks/`)
5. **Do NOT test external libraries**: Only test YOUR code, not std:: or third-party libraries

**Test Organization:**
- `tests/core/` - Unit tests for core classes (Camera, Shader, Particle, SettingsIO)
- `tests/integration/` - Integration tests for component interactions
- `tests/testing/` - Tests for testing utilities (PixelComparator, Image)
- `tests/visual-regression/` - Visual regression tests (run headless with Xvfb, no GPU needed)
- `tests/mocks/` - Mock implementations (MockOpenGL, etc.)
- Each test file corresponds to a core implementation unit (e.g., `CameraTests.cpp` for `camera.hpp`)

**Coverage Target**: ≥80% for tested modules, prioritizing core logic, public APIs, and error handling.

## Code Standards

### Formatting (clang-format)

**Auto-formatted via `.clang-format`** - DO NOT manually format code.

**⚠️ CRITICAL: ALWAYS run clang-format on ALL changed files BEFORE committing!**

```bash
# Format all C++ files (run this before EVERY commit)
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

### CI Pipeline Rules

**NEVER commit or push from a pipeline.** For detailed CI/CD rules and patterns, use the `workflow` skill (`.github/skills/workflow/`).

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
├── ui/                 # ImGui menu system (imgui_menu.hpp/cpp)
├── testing/            # Testing utilities (PixelComparator)
├── shaders/            # GLSL shader files (auto-copied during build)
├── glad/               # GLAD OpenGL loader (embedded)
└── tinyFileDialogs/    # File dialog library (embedded)

tests/
├── core/               # Unit tests for main classes
├── integration/        # Integration tests for component interactions
├── testing/            # Tests for testing utilities
├── visual-regression/  # Visual regression test fixture and tests
├── mocks/              # Mock implementations
└── CMakeLists.txt      # Test build configuration

docs/
├── CODING_STANDARDS.md      # Detailed code standards
├── TESTING_STANDARDS.md     # Testing best practices
├── RELEASE_PROCESS.md       # Release automation details
├── CONVENTIONAL_COMMITS.md  # Commit format quick reference
└── testing/
    ├── integration-tests.md     # Integration test guide
    └── visual-regression.md     # Visual regression test guide

.github/
├── workflows/          # CI/CD pipelines
├── copilot-instructions.md  # This file
└── skills/             # Custom Copilot skills
    ├── testing/        # Test writing (AAA, naming, visual regression)
    ├── workflow/       # CI/CD pipeline rules and patterns
    ├── documentation/  # Documentation conventions and templates
    └── user-story-generator/  # INVEST-aligned story creation
```

### Skill Architecture

**Rule: Minimize duplication across skills.** Each skill owns one domain. Skills reference each other instead of repeating content.

| Skill | Domain | Reference From |
|-------|--------|----------------|
| `testing` | Test writing, AAA pattern, mocking | `docs/TESTING_STANDARDS.md` |
| `workflow` | CI/CD pipelines, artifacts, permissions | CI Pipeline Rules section |
| `documentation` | Docs conventions, linking, formatting | This section |
| `user-story-generator` | Story creation, INVEST framework | Project planning |
| `self-evaluation` | End-of-session review, lessons learned | This section |

When a skill needs rules from another domain, it references the other skill by path rather than duplicating the rules.

### Key Source Files

- `main.cpp` - Application entry point (creates and runs `ViewerApp`)
- `viewer_app.hpp/.cpp` - Main application class; owns window, rendering pipeline, and all state
- `camera.hpp` - Header-only camera system for 3D navigation
- `shader.hpp` - Header-only shader program management
- `particle.hpp` - Header-only particle data structures (uses `std::vector<glm::vec4>`)
- `settingsIO.hpp` - Header-only configuration file I/O
- `osFile.hpp` - OS-specific file operations
- `debugOverlay.hpp` - Camera debug overlay with FPS counter (top-right corner)
- `ui/imgui_menu.hpp/.cpp` - ImGui main menu bar (File, View menus)
- `Image.hpp/.cpp` - Core RGBA image class with save/load (PPM, PNG)
- `testing/PixelComparator.hpp/.cpp` - Pixel-by-pixel image comparison for visual regression testing

## Common Tasks and Commands

### Code Quality Checks (Before Committing)

**⚠️ MANDATORY: Run these checks before EVERY commit to avoid CI failures!**

```bash
# 1. Format ALL code (REQUIRED before committing)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# 2. Verify formatting (CI check - must pass)
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# 3. Run static analysis (informational)
clang-tidy src/main.cpp -- -Isrc/glad/include

# 4. Build and test
cmake -B build -S .
cmake --build build
./build/tests/ParticleViewerTests
```

**Pre-commit Checklist:**
- [ ] ✅ Run `clang-format -i` on all changed files
- [ ] ✅ Verify `clang-format --dry-run -Werror` passes
- [ ] ✅ Build succeeds
- [ ] ✅ All tests pass
- [ ] ✅ Use conventional commit message format

### Adding a New Feature

1. Create feature branch (if not using Copilot PR)
2. Make code changes following naming conventions
3. Add unit tests in `tests/core/` following AAA pattern
4. **⚠️ REQUIRED: Run `clang-format -i` on ALL changed files**
5. Build and verify tests pass
6. Verify formatting with `clang-format --dry-run -Werror`
7. Commit with conventional commit format (e.g., `feat: add new feature`)
8. CI checks will run automatically on PR

### Fixing a Bug

1. Write a failing test that reproduces the bug
2. Fix the code
3. Verify test now passes
4. Run full test suite
5. **⚠️ REQUIRED: Format code with `clang-format -i` on changed files**
6. Verify formatting with `clang-format --dry-run -Werror`
7. Commit with format: `fix: description of what was fixed`

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

**Problem**: Multiple GLFWContexts in tests cause segfaults
**Solution**: `~GLFWContext()` calls `glfwTerminate()`, which kills all GLFW state. Never create/destroy multiple GLFWContext objects in a single test. For multi-resolution testing, use different-sized `FramebufferCapture` objects within a single GL context instead.

**Problem**: Visual regression baseline fails with 1-pixel diff across Mesa versions
**Solution**: Different Mesa/llvmpipe versions may produce sprite-boundary rounding differences. Use a `MAX_DIFF_RATIO` (e.g., 0.01%) instead of requiring 100% pixel match. Always assert artifact `save()` results so debug images are not silently lost.

**Problem**: Test name doesn't follow convention
**Solution**: Use format `UnitName_StateUnderTest_ExpectedResult` (e.g., `MoveForward_IncreasesPosition`)

### Code Style Issues

**Problem**: CI formatting check fails
**Solution**: Run `find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i` before committing

**Problem**: clang-tidy warnings
**Solution**: Review `.clang-tidy` configuration and fix issues or suppress false positives with `// NOLINT`

**Problem**: Header file not self-contained (relies on transitive includes)
**Solution**: Every header must include all headers it directly uses. Don't rely on other headers pulling in dependencies. Mark functions defined in headers as `inline` to avoid multiple-definition linker errors.

### Version/Release Issues

**Problem**: Need to create a release
**Solution**: Just push conventional commits to master - release is automatic. Use `feat:` for minor, `fix:` for patch, `feat!:` for major.

**Problem**: Wrong version bump occurred
**Solution**: Verify commit message uses correct conventional commit type. Manual override via workflow_dispatch if needed.

## Additional Resources

- **Full Coding Standards**: `docs/CODING_STANDARDS.md`
- **Testing Standards**: `docs/TESTING_STANDARDS.md`
- **Visual Regression Testing**: `docs/testing/visual-regression.md`
- **Visual Regression Camera Positioning**: `docs/visual-regression/camera-positioning-lessons-learned.md` ⚠️
- **Integration Tests**: `docs/testing/integration-tests.md`
- **Release Process**: `docs/RELEASE_PROCESS.md`
- **Conventional Commits Quick Ref**: `docs/CONVENTIONAL_COMMITS.md`
- **Microsoft C++ Core Guidelines**: https://isocpp.github.io/CppCoreGuidelines/
- **Google Test Docs**: https://google.github.io/googletest/

## Project-Specific Patterns

### Data Organization
- Group related member variables into POCOs/structs (e.g., `WindowConfig`, `RenderResources`, `SphereParams`)
- Structs should provide their own defaults to reduce constructor initializer lists
- Use structs for vertex data instead of raw float arrays (e.g., `QuadVertex` with named x, y, u, v fields)

### Error Handling
- Check return values from OpenGL and file I/O operations
- Log errors to console with descriptive messages
- Use assertions for preconditions and invariants
- Open binary data files with `"rb"` mode (not `"r"`) for cross-platform correctness

### Memory Management
- Prefer stack allocation over heap when possible
- Use RAII for resource management
- Smart pointers for dynamic memory (when C++11 features are used)
- Clean up ALL GL resources in destructors (VAOs, VBOs, FBOs, RBOs, textures)
- Prevent copy of classes that own GL resources (delete copy ctor/assignment)

### OpenGL Usage
- Check OpenGL errors after major operations
- Use GLAD for OpenGL function loading — check return value of `gladLoadGLLoader`
- Shaders are loaded from `Viewer-Assets/shaders/` directory
- Vertex data should use modern VBO/VAO patterns
- Bounds-check GLFW key callbacks (GLFW_KEY_UNKNOWN is -1)
- Prefer `glGetIntegerv(GL_VIEWPORT, ...)` over cached viewport values in render paths where the viewport may change (e.g., window resize, FBO switches)
- `gl_PointSize` is silently clamped by `GL_POINT_SIZE_RANGE` (max 256px on Mesa/llvmpipe). When testing resolution-independent scaling, choose camera distances that keep computed point sizes under this limit at **all** target resolutions including 4K.

### ImGui Integration

**Architecture:**
- Dear ImGui is downloaded via CMake FetchContent (not vendored). Since ImGui has no `CMakeLists.txt`, use `FetchContent_Declare()` with `SOURCE_SUBDIR` pointing to a non-existent path plus `FetchContent_MakeAvailable()`, then manually add source files to the target. This avoids the `FetchContent_Populate()` deprecation warning.
- ImGui renders to the **default framebuffer** (after the FBO blit), so it naturally does not appear in FBO-based screenshots or frame recordings.
- GLFW callbacks: Set ViewerApp's GLFW callbacks **before** calling `ImGui_ImplGlfw_InitForOpenGL(window, true)`. ImGui saves and chains to existing callbacks, ensuring the chain is: ImGui → ViewerApp.

**Menu System (`src/ui/`):**
- `MenuState` tracks visibility and debug mode; `MenuActions` communicates triggered actions back to ViewerApp
- F1 toggles menu visibility, F3 toggles debug mode at runtime
- `menu_state_.debug_mode` is the runtime source of truth (initialized from `--debug-camera` CLI flag)

**Debug Overlay Positioning:**
- The debug overlay is an ImGui window positioned via `ImGui::SetNextWindowPos()` with `ImGui::GetFrameHeight()` to dynamically offset below the menu bar.
- Uses `ImGuiWindowFlags_AlwaysAutoResize` for automatic sizing and anchors to the top-right corner.
- When adding new overlays, account for the menu bar height to avoid z-order collisions.

### Visual Regression Tests ⚠️ IMPORTANT

**CRITICAL: Visual regression tests DO NOT require a display or GPU**
- Tests run headless using Xvfb (X virtual framebuffer)
- No physical display or GPU required - works in CI environments
- Use: `xvfb-run -a ./tests/ParticleViewerTests --gtest_filter="RenderingRegressionTest.*"`
- Tests use software rendering (Mesa/llvmpipe) for consistent cross-platform results
- When tests are skipped due to missing DISPLAY, simply run with `xvfb-run -a`

**Use production classes in tests:**
- Visual regression tests should use `Particle` directly, not duplicate its logic in test helpers
- This ensures tests stay in sync with production code automatically

**Camera Positioning Pitfall:**
When creating visual regression tests using debug camera output:

❌ **DON'T**: Blindly copy debug coordinates
- Debug shows camera state during user interaction
- May be zoomed in/out for inspection, not ideal framing
- Distance may not produce desired composition

✓ **DO**: Calculate camera distance for composition
1. Extract viewing **direction** from debug (angle preserved)
2. Calculate **distance** based on desired viewport coverage
3. Formula: `distance = subject_size / (coverage_% * tan(FOV/2))`
4. Typical scale factors: 1.5x - 2.0x from initial distance

**Key lesson**: Debug output shows WHERE you're looking, not WHAT you're framing.

See `docs/visual-regression/camera-positioning-lessons-learned.md` for full analysis and formulas.

## Tips for Efficient Work

1. **Read documentation first**: Check `docs/` before making assumptions
2. **Follow existing patterns**: Look at existing code for consistency (e.g., how other tests are written)
3. **Use tools**: Let clang-format handle formatting, don't do it manually
4. **Test incrementally**: Run tests frequently during development
5. **Commit properly**: Use conventional commits - your commit messages become release notes
6. **⚠️ ALWAYS format before committing**: Run `clang-format -i` on ALL changed files BEFORE every commit to avoid CI failures
7. **Check CI early**: Run local formatting/tidy checks before pushing to catch CI failures early
8. **Self-review before finishing**: Run code_review tool, check AAA pattern compliance, verify no resource leaks, ensure documentation is updated for any architectural changes
9. **Run self-evaluation**: Before finalizing any session, run the `self-evaluation` skill (`.github/skills/self-evaluation/`) to capture lessons learned and improve project knowledge

## When in Doubt

- Consult `docs/CODING_STANDARDS.md` for code style questions
- Consult `docs/TESTING_STANDARDS.md` for test-writing questions
- Check existing similar code for patterns
- **⚠️ ALWAYS run `clang-format -i` on changed files before committing**
- Run clang-format and clang-tidy to catch issues
- Build and test locally before pushing
