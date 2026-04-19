---
name: architecture-review
description: Use when adding new classes, refactoring code, or reviewing PRs for Particle-Viewer.
---

## Iron Law

```
DEPENDENCIES FLOW INWARD — INNER LAYERS NEVER DEPEND ON OUTER LAYERS
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST verify dependency direction for every new class and every refactor. A layer violation in a PR means the PR is NOT mergeable until it is fixed. No exceptions.

**Announce at start:** "I am using the architecture-review skill to review [component/file]."

---

## The Layer Architecture

Particle-Viewer uses a strict inward-dependency model. Code in layer N may only depend on layers ≤ N. Never the reverse.

```
Layer 4 (outermost): main.cpp
  Entry point only — instantiates ViewerApp and calls Run()

Layer 3: ViewerApp
  Orchestrator — owns all runtime state, delegates to layers below
  Files: viewer_app.hpp, viewer_app.cpp

Layer 2: UI, Graphics, Camera, Shader, Particle
  Domain components — each owns its own logic, no cross-component reach
  Files: ui/imgui_menu.hpp, ui/imgui_menu.cpp,
         graphics/SDL3Context.hpp, graphics/SDL3Context.cpp,
         camera.hpp, shader.hpp, particle.hpp

Layer 1 (innermost): Abstractions and utilities
  Files: graphics/IOpenGLContext.hpp, Image.hpp, Image.cpp
  Test utility (not production): src/testing/PixelComparator.hpp, src/testing/PixelComparator.cpp
```

### What Each Layer May Depend On

| Layer | May depend on |
|-------|--------------|
| 4 (main.cpp) | Layer 3 only |
| 3 (ViewerApp) | Layers 1–2 |
| 2 (UI/Graphics/Camera/Shader/Particle) | Layer 1 only |
| 1 (IOpenGLContext, Image) | No Particle-Viewer layers |

---

## Hyrum's Law Gate

> "With a sufficient number of users of an API, it does not matter what you promise in the contract: all observable behaviors of your system will be depended upon by somebody."

Before modifying **any observable behavior** of an existing component — including undocumented behaviors, error messages, return value nuances, timing, side effects — ask:

**"What else could currently be depending on this?"**

This includes:
- Tests that happen to rely on the current error message text
- Callers that rely on the current return value for edge cases not covered by docs
- Visual tests that rely on the current rendering output being exactly pixel-perfect

**Gate:** If you cannot enumerate all call sites that might depend on the behavior being changed, dispatch an explore agent to find them before proceeding. Changing behavior without knowing who relies on it is a silent breaking change.

This does not mean "never change behavior." It means: be deliberate. Know what you're breaking. Break it intentionally, with all dependents updated in the same commit.

---

## Barricade Model

The codebase has a **dirty zone** (data that has not been validated) and a **clean zone** (data that has passed all validation). The barricade is the boundary between them.

**Rules:**
- Code in the clean zone never handles dirty data. If it receives data, that data is already validated.
- Validation happens at the barricade, not scattered through clean-zone code
- The barricade for this project: data enters dirty through file I/O, user input, and external APIs. It becomes clean after parsing and validation in the input handlers.

| Zone | Contains | May assume |
|------|----------|-----------|
| Dirty | File bytes, user input strings, socket data | Nothing — validate everything |
| Barricade | Parser, validator, loader functions | Converts dirty → clean |
| Clean | ViewerApp state, Camera, Shader, Particle | Data is valid — no defensive checks needed |

**Violation signal:** A class in Layer 2 (Camera, Shader, Particle) checking for null or empty data it received from ViewerApp. That check belongs at the barricade, not in the clean zone.

---

Run every item for each file under review:

- [ ] Does the new or modified class belong to a defined layer?
- [ ] Does it import or call code from an outer layer? (VIOLATION if yes)
- [ ] Does `ViewerApp` orchestrate or implement? (must orchestrate only — rendering logic belongs in Shader/Particle classes)
- [ ] Does raw OpenGL (`glXxx()`, `glXxx_ext()`) appear outside of `IOpenGLContext` implementations? (VIOLATION)
- [ ] Do any `src/` files import from `tests/`? (VIOLATION — production code must never depend on test code)
- [ ] Does `src/testing/PixelComparator` acquire OpenGL state directly, rather than receiving an `Image`? (VIOLATION)
- [ ] Do any UI files (`ui/`) reach into `graphics/` internals beyond `IOpenGLContext`? (VIOLATION)
- [ ] Are there circular `#include` dependencies between any two files in the same layer?

✓ All pass → verdict: CLEAN
✗ Any fail → verdict: VIOLATIONS FOUND — document every failure in the Review Report

---

## Common Violations (Particle-Viewer Specific)

These are the violations most likely to appear in this codebase. Check for each explicitly.

| Violation | Example | Fix |
|-----------|---------|-----|
| Raw GL outside IOpenGLContext | `glDrawArrays(...)` in `viewer_app.cpp` | Move into `SDL3Context` behind an `IOpenGLContext` method |
| ViewerApp implementing rendering | Loop logic or shader setup inside `ViewerApp::Render()` | Extract to `Shader` or `Particle` class |
| Production code importing test code | `#include "../../tests/mocks/MockOpenGL.hpp"` in `src/` | Remove; mocks are test-only |
| PixelComparator acquiring GL state | `glReadPixels(...)` inside `PixelComparator` | Pass a pre-captured `Image` to `PixelComparator` |
| UI reaching into graphics internals | `#include "graphics/SDL3Context.hpp"` in `ui/imgui_menu.hpp` | Depend on `IOpenGLContext` interface only |

---

## Review Report Format

```markdown
## Architecture Review: [file/component]

### Layer Assignment
Component belongs to: Layer [N] ([name])
Expected dependencies: Layers ≤[N]
Actual dependencies found: [list includes/calls]

### Violations Found
| File | Line | Violation | Fix |
|------|------|-----------|-----|

### Verdict: CLEAN / VIOLATIONS FOUND
```

A verdict of VIOLATIONS FOUND means the PR is NOT mergeable until every row in the violations table is resolved.

---

## Dispatch Pattern

For PR reviews or major refactors:

1. Dispatch 1 `architecture-reviewer.md` agent per changed file (parallel) — use `agent_type: "code-review"`
2. Provide: `{{FILE_PATH}}`, `{{INCLUDE_LIST}}`, and `{{DIFF_OR_EMPTY}}`
3. Collect all reports before approving the PR
4. Any VIOLATIONS FOUND verdict = block the PR

---

## Red Flags — STOP

If you catch yourself thinking any of the following, STOP before writing your verdict:

- "It only calls one GL function directly, that's fine" → Stop. One raw GL call outside `IOpenGLContext` is a violation. Flag it.
- "ViewerApp is the orchestrator so it's fine to put logic there" → Stop. Orchestration means delegation. Logic belongs in domain classes.
- "The test utility is small, it won't hurt in src/" → Stop. Test code in production is a maintenance trap. Flag it.
- "It's in the same layer, so the dependency is acceptable" → Stop. Same layer, different files — check for circular dependencies. Design smell if found.
- "The violation is minor, I'll note it but give CLEAN" → Stop. There is no CLEAN with open violations. Verdict is VIOLATIONS FOUND.

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "Just a small dependency, doesn't affect architecture" | Architecture violations compound. Small ones become load-bearing. Fix them now. |
| "ViewerApp needed to call GL directly for performance" | Use `IOpenGLContext`. That abstraction exists precisely for this case. |
| "The test utility is tiny, no harm putting it in src/" | Test-only code in production is a maintenance trap and blurs the test boundary. |
| "Circular dependency is fine since they're in the same layer" | Same layer does not mean circular is acceptable. It is still a design smell requiring resolution. |
| "I'll refactor it properly later" | Later never comes. Fix the boundary violation before this code ships. |
| "The PR is urgent, we can clean up the architecture next sprint" | Urgency is the most common rationalization for shipping violations. The law applies under urgency too. |

---

## Related Skills

- `code-quality` — naming conventions and C++ patterns; architecture-review checks structure, code-quality checks form
- `testing` — governs what lives in `tests/` vs `src/testing/`; architecture-review enforces the boundary
- `infrastructure-review` — CMake build structure; architecture-review checks source structure
