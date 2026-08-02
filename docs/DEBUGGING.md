---
title: "Debugging Reference"
description: "Debug commands for build, test filtering, visual regression diffs, and CI reproduction in Particle Viewer."
domain: debugging
subdomain: commands
tags: [debugging, commands, testing, ci, visual-regression]
related:
  - "TESTING_STANDARDS.md"
  - "VERIFICATION_COMMANDS.md"
---

# Particle-Viewer Debug Reference

Project-specific commands and layer taxonomy for debugging Particle-Viewer.

---

## Project-Specific Commands

For the full build, format, and test-suite commands, see [Verification Commands](VERIFICATION_COMMANDS.md).

```bash
# Build failure -- read the first 50 lines (root error is usually near the top)
cmake --build build 2>&1 | head -50
```

- Test failure -- run the specific failing test in isolation; see [Verification Commands](VERIFICATION_COMMANDS.md) for the `--gtest_filter` invocation.
- Visual regression -- examine the diff image. Diffs are written to `artifacts/*.png`, relative to the working directory the test binary is invoked from.
- CI failure -- reproduce locally with the build-and-test command in [Verification Commands](VERIFICATION_COMMANDS.md) before touching code.

---

## Layer Taxonomy

```
User Input (SDL3 (Simple DirectMedia Layer 3) events)
    v
ViewerApp (main app logic, state machine)
    v
UI (User Interface) layer (ImGui menu -- imgui_menu.hpp/cpp)
    v
Graphics layer (IOpenGLContext, SDL3Context)
    v
OpenGL driver (via GLAD (OpenGL Loader-Generator library))
    v
Shader (GLSL (OpenGL Shading Language) -- Viewer-Assets/shaders/)
    v
GPU (Graphics Processing Unit) output
```

When debugging, identify which layer first produces incorrect behavior. Fix at that layer -- not at the symptom layer above it.

---

## Related

- [Testing Standards](TESTING_STANDARDS.md) -- test structure and coverage expectations referenced when a test fails
- [Verification Commands](VERIFICATION_COMMANDS.md) -- full build, test, and format command set to run before every commit
