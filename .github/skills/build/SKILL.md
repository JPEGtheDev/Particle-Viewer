---
name: build
description: Build, configure, and troubleshoot the Particle-Viewer CMake project. Use when building, adding dependencies, configuring CMake options, troubleshooting build failures, or managing Flatpak packaging.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.2"
  category: build
  project: Particle-Viewer
---

## Iron Law

```
BUILD MUST PASS LOCALLY BEFORE EVERY PUSH
```

`cmake --build build` must exit 0 locally before pushing. No exceptions.

**Announce at start:** "I am using the build skill to [build/configure/troubleshoot] [description]."

---

# Instructions for Agent

## How This Skill is Invoked

In VS Code, users will activate this skill by:
- Typing `@workspace /build [description]` in Copilot Chat
- Or asking: "How do I build?", "Fix the build", "Add a dependency", "Set up Flatpak"

When activated, use the commands and patterns below.

---

## Core Principle: CMake Is the Single Source of Build Truth

All build configuration lives in `CMakeLists.txt`. No manual file copying, no build scripts beyond convenience wrappers.

---

## Step 1: Build Commands

```bash
# Standard build
cmake -B build -S .
cmake --build build

# Development mode (all warnings + coverage)
cmake -B build -S . -Dalloutput=ON
cmake --build build

# Run the application
./build/Viewer

# Install (copies binary + shader assets to bin/)
cmake --install build
```

---

## Step 2: Build Requirements

| Requirement | Minimum Version |
|-------------|----------------|
| CMake | 3.24 |
| C++ compiler | C++20 (GCC ≥ 10, Clang ≥ 11, MSVC 2019+) |
| OpenGL | Development libraries (`OpenGL::GL` target) |
| GLM | System-installed |
| SDL3 | Auto-downloaded via FetchContent (local) or Flatpak module |
| clang-format | For code formatting |
| clang-tidy | For static analysis (advisory) |

---

## Step 3: Dependency Management

### SDL3
- **Local builds:** `CMakeLists.txt` calls `find_package(SDL3)` first, falls back to `FetchContent` if not found.
- **Flatpak builds:** SDL3 is built as a standalone Flatpak module (before the app) so the Freedesktop SDK pkg-config paths detect `SDL_X11` and `SDL_WAYLAND`. FetchContent is NOT used in Flatpak.

### Dear ImGui
- Always downloaded via `FetchContent`.

### Embedded Libraries (no management needed)
- `src/glad/` — GLAD OpenGL loader
- `src/tinyFileDialogs/` — File dialog library
- `src/stb_*.h` — stb single-header libraries

### Dependency Upgrades
- Allowed as long as all tests pass and build succeeds
- Visual regression tests are a hard requirement — must pass without modification unless the approver is explicitly shown the new output
- Test updates for API changes (e.g., GoogleTest) are acceptable

---

## Step 4: Build Notes

- **Shaders:** Files in `src/shaders/` are auto-copied to `Viewer-Assets/shaders/` during build.
- **Tests:** Enabled by default (`BUILD_TESTS=ON`). Run with `./build/tests/ParticleViewerTests`.
- **Version:** See `versioning` skill for how version is resolved at build time.

---

## Step 5: Troubleshooting

**Missing SDL3:**
Run `cmake -B build -S .` — SDL3 downloads automatically via FetchContent.

**OpenGL headers not found:**
Install OpenGL development packages. Ensure `OpenGL::GL` CMake target is available.

**Shader files not found at runtime:**
Run from the build directory, or ensure `Viewer-Assets/shaders/` exists alongside the binary.

**Flatpak build issues:**
See the `workflow` skill and [FLATPAK_GL_GOTCHAS.md](../workflow/references/FLATPAK_GL_GOTCHAS.md).

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "Small change, can't break the build" | Even single-line changes break builds. Run `cmake --build build` first. |
| "CI will catch it if the build fails" | CI is a safety net. Don't push broken builds intentionally. |
| "It built fine yesterday" | Dependencies change. State changes. Build locally before every push. |
| "The CMake error is too confusing to parse" | Read the first 10 lines of error output — that's always where the root cause is. |
| "I'll fix the FetchContent pin later" | Unpinned dependencies produce non-reproducible builds. Pin to tag/commit now. |
| "The tests don't need to be in the build target" | All test targets must build cleanly. No orphaned test binaries. |

---

## Red Flags — STOP

If you catch yourself thinking any of these, stop and follow the rule:
- "I'll push and see if CI builds it"
- "This small change couldn't break CMake"
- "The dependency version doesn't really matter"
- "I don't understand the CMake error but it probably works"
- "I'll update the dependency pin in a follow-up"
- About to push without running `cmake --build build` locally

**All of these mean: Run `cmake --build build` locally. Read the full output. Fix errors before pushing.**

---

## Reference

- Flatpak manifest: `flatpak/org.particleviewer.ParticleViewer.yaml`
- Flatpak GL/SDL3 gotchas: `.github/skills/workflow/references/FLATPAK_GL_GOTCHAS.md`
- Build scripts: `scripts/linuxBuildAndInstall.sh`, `scripts/build-flatpak.sh`
