---
title: "Layer Architecture"
description: "Layered inward-dependency model, file-to-layer mapping, dependency rules, and Barricade Model zones for Particle Viewer."
domain: architecture
subdomain: layers
tags: [architecture, layers, dependencies, review]
related:
  - "CODING_STANDARDS.md"
---

# Particle-Viewer Layer Architecture

The Particle-Viewer 4-layer inward-dependency model and common violations.

## The Layer Architecture

Particle-Viewer uses a strict inward-dependency model. Code in layer N may only depend on layers <= N. Never the reverse.

```
Layer 4 (outermost): main.cpp
  Entry point only -- instantiates ViewerApp and calls Run()

Layer 3: ViewerApp
  Orchestrator -- owns all runtime state, delegates to layers below
  Files: viewer_app.hpp, viewer_app.cpp

Layer 2: UI (User Interface), Graphics, Camera, Shader, Particle
  Domain components -- each owns its own logic, no cross-component reach
  Files: ui/imgui_menu.hpp, ui/imgui_menu.cpp,
         graphics/SDL3Context.hpp, graphics/SDL3Context.cpp,
         camera.hpp, shader.hpp, particle.hpp

Layer 1 (innermost): Abstractions and utilities
  Files: graphics/IOpenGLContext.hpp, Image.hpp, Image.cpp
  Test utility (not production): src/testing/PixelComparator.hpp, src/testing/PixelComparator.cpp
```

The file lists above are illustrative anchor points for each layer, not an exhaustive inventory of `src/` (47 `.hpp`/`.cpp` files as of this writing). Most files not named here -- the center-of-mass (COM) cache and calculator utilities, the marching-cubes renderer, the threaded executor, gamepad input, and similar helpers -- still follow the same inward-dependency rule based on what they import, they are just not individually enumerated in this diagram.

### What Each Layer May Depend On

| Layer | May depend on |
|-------|--------------|
| 4 (main.cpp) | Layer 3 only |
| 3 (ViewerApp) | Layers 1-2 |
| 2 (UI/Graphics/Camera/Shader/Particle) | Layer 1 only |
| 1 (IOpenGLContext, Image) | No Particle-Viewer layers |

---

## The Barricade Model

Within this layer model, data also has a validation state: a **dirty zone**, where data has not yet been checked, and a **clean zone**, where data has already passed validation. The **barricade** is the boundary between them.

Data starts out dirty when it comes from file I/O, user input, or any other external source. It becomes clean only after an input handler has parsed and validated it -- that parsing and validation is the barricade itself. Once data is past the barricade, clean-zone code should not re-validate it: a Layer 2 class that null-checks or empty-checks data it received from `ViewerApp` is redoing work the barricade already did, and that redundant check is itself a sign the barricade was skipped or misplaced.

For Particle-Viewer, the validated domain objects living in the clean zone are `ViewerApp` state, `Camera`, `Shader`, and `Particle`.

## Additional Gate Items (Particle-Viewer Specific)

Run these checks in addition to verifying dependency direction and layer boundaries for every new class or refactor:

1. Does domain code reimplement window/context lifecycle management (creating a context, making it current, swapping buffers, tracking close state) instead of going through `IOpenGLContext`? (VIOLATION -- per-frame draw and state calls like `glDrawArrays` or `glUniform*` are expected directly in Shader/Particle/ViewerApp and are not, by themselves, a violation; event polling is handled directly via `SDL_PollEvent` in `viewer_app.cpp` and is not part of this rule)
2. Does `ViewerApp` orchestrate or implement? (must orchestrate only -- rendering logic belongs in Shader/Particle classes)
3. Does `src/testing/PixelComparator` acquire OpenGL state directly, rather than receiving an `Image`? (VIOLATION)
4. Do any UI files (`ui/`) reach into `graphics/` internals beyond `IOpenGLContext`? (VIOLATION)

---

## Common Violations (Particle-Viewer Specific)

These are the violations most likely to appear in this codebase. Check for each explicitly.

| Violation | Example | Fix |
|-----------|---------|-----|
| Context/window lifecycle (creation, current, swap, close-tracking) reimplemented outside IOpenGLContext | Calling `SDL_GL_SwapWindow` or `SDL_GL_MakeCurrent` directly instead of `context.swapBuffers()` / `context.makeCurrent()` | Route those lifecycle operations through the injected `IOpenGLContext` so tests can substitute `MockOpenGLContext`; event polling (`SDL_PollEvent`) is a separate, already-direct pattern this rule does not cover |
| ViewerApp implementing rendering | Loop logic or shader setup inside `ViewerApp::Render()` | Extract to `Shader` or `Particle` class |
| Production code importing test code | `#include "../../tests/mocks/MockOpenGL.hpp"` in `src/` | Remove; mocks are test-only |
| PixelComparator acquiring GL state | `glReadPixels(...)` inside `PixelComparator` | Pass a pre-captured `Image` to `PixelComparator` |
| UI reaching into graphics internals | `#include "graphics/SDL3Context.hpp"` in `ui/imgui_menu.hpp` | Depend on `IOpenGLContext` interface only |
| Clean-zone code re-validating already-validated data | A Layer 2 class null-checking or empty-checking data it received from `ViewerApp` | Trust data past the barricade; move any missing validation into the input handler that produced it |

---

## Red Flags -- STOP (Particle-Viewer Specific)

Run these checks in addition to watching for any class that imports, calls, or names a type from an outer layer:

- "It's simpler to just call SDL directly here instead of going through the context interface" -> Stop. That bypasses `IOpenGLContext` and breaks headless/mock testing. Flag it.
- "ViewerApp is the orchestrator so it's fine to put logic there" -> Stop. Orchestration means delegation. Rendering logic belongs in domain classes.

---

## Rationalization Prevention (Particle-Viewer Specific)

| Excuse | Reality |
|--------|---------|
| "ViewerApp needed to bypass IOpenGLContext for performance" | `IOpenGLContext` only covers window/context lifecycle, not per-frame draw calls, so there is nothing to bypass -- `glDrawArrays`, `glUniform*`, and similar calls already belong directly in Shader/Particle/ViewerApp |
| "This class already got validated data, but I'll check it again to be safe" | Redundant checks in the clean zone hide where validation actually belongs; if the data could still be invalid here, the barricade is misplaced -- fix the barricade, don't add a second one |

---

## Related

- [CODING_STANDARDS.md](CODING_STANDARDS.md) -- C++ formatting, naming, and organization rules applied within each layer
- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) -- project scope, component inventory, and tech stack this layer model organizes
