---
name: cpp-patterns
description: Use when implementing C++ code for Particle-Viewer, handling GL resources, working with SDL3, or applying DRY/deprecation/docs-commit patterns.
---

## Iron Law

```
NO C++ THAT VIOLATES RESOURCE MANAGEMENT OR BREAKS EXISTING ABSTRACTIONS
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST clean up all GL resources in destructors and update documentation in the same commit as any public interface change. No exceptions.

**Announce at start:** "I am using the cpp-patterns skill to [implement/review] [description]."

---

## BEFORE PROCEEDING

- [ ] Have I loaded the code-quality skill?
- [ ] Am I about to introduce a GL resource without RAII?
- [ ] Am I about to duplicate logic that already exists in the codebase?
- [ ] Has the class or function I am changing been read — not recalled from memory?

✓ All met → proceed
✗ Any unmet → load the code-quality skill, apply RAII, search for existing implementations, or read the target code before writing any production code

---

# Instructions for Agent

When writing C++ code for this project, apply these patterns consistently.

---

## Data Organization

- Group related member variables into POCOs/structs (e.g., `WindowConfig`, `SphereParams`)
- Structs provide their own defaults to reduce constructor initializer lists
- Use structs for vertex data (`QuadVertex` with x, y, u, v) instead of raw float arrays
- When a test or fixture has many flat member variables, group them into domain-specific structs

---

## Error Handling

- Check return values from OpenGL and file I/O operations
- Log errors to console with descriptive messages
- Use assertions for preconditions and invariants
- Open binary data files with `"rb"` mode for cross-platform correctness

---

## Memory Management

- Use stack allocation; heap only when lifetime or size requires it
- Use RAII for resource management
- Smart pointers for dynamic memory
- Clean up ALL GL resources in destructors (VAOs, VBOs, FBOs, RBOs, textures)
- Prevent copy of classes that own GL resources (delete copy ctor/assignment)

---

## OpenGL Usage

- Check OpenGL errors after major operations
- GLAD loader call: `gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)`
- Shaders loaded from `Viewer-Assets/shaders/`
- Modern VBO/VAO patterns for vertex data
- Bounds-check SDL3 scancode values before indexing key state arrays
- Use `glGetIntegerv(GL_VIEWPORT, ...)` over cached viewport values where viewport may change
- `gl_PointSize` clamped by `GL_POINT_SIZE_RANGE` (max 256px on Mesa/llvmpipe)
- For Flatpak/SDL3/GL gotchas: see `.github/skills/workflow/references/FLATPAK_GL_GOTCHAS.md`

### SDL3 Gotchas

- **Subsystem flags:** `SDL_Init` must include the flag for every SDL3 subsystem used. Missing a flag causes silent failure — no error, no events, no devices.
- **`*ForID` query functions:** Use `SDL_Get*ForID()` variants (e.g., `SDL_GetJoystickGUIDForID`) over opening a joystick just to read a value then close it. Unnecessary open consumes a file handle.
- **Joystick type is distro-dependent:** `SDL_GetJoystickTypeForID()` returns `SDL_JOYSTICK_TYPE_GAMEPAD` on SteamOS but `SDL_JOYSTICK_TYPE_UNKNOWN` on generic Linux (OpenSuse, Ubuntu) because stock udev rules don't set the GAMEPAD property. Always pair type-based detection with a capability-based fallback: open the joystick, check `SDL_GetNumJoystickAxes() >= 4 && SDL_GetNumJoystickButtons() >= 6`, then close it.
- **Gamepad hold-button state:** Don't route through `Camera::KeyReader()`. `KeyReader` has a single-press dispatch block — calling it every frame fires single-press handlers repeatedly. For hold-buttons that mirror keyboard held keys, add a dedicated method (e.g., `Camera::setSpeedBoost(bool)`).

---

## Headers

- Every header must include all headers it directly uses — no transitive include reliance
- Mark functions defined in headers as `inline` to avoid multiple-definition linker errors

---

## ImGui Integration

See [`docs/IMGUI_INTEGRATION.md`](../../../docs/IMGUI_INTEGRATION.md) for architecture, FetchContent setup, menu system, and overlay positioning.

---

## Removing Features / User-Requested Changes

When removing a call site from `viewer_app.cpp`, **do not also delete the supporting `Camera` public method** unless it is architecturally wrong. The Camera API is stable; call sites in `viewer_app` change frequently with user preferences. Only remove a Camera method if it is architecturally wrong, not merely unused at the current moment.

---

## DRY — Knowledge, Not Text

DRY means every piece of **knowledge** has a single authoritative representation. Not text.

**Acid test:** If you change one copy and not the other, does the system break? If yes: DRY violation. If no: independent representations of different things — not a violation.

- **DRY violation signal:** "If this constant changes, I need to update it in N places."
- **False DRY signal:** "These two functions look the same." Looks are not knowledge. Check whether they represent the same concept before extracting.

---

## Broken Window Protocol

When you encounter a code smell or quality violation while working on something else:
1. Do NOT ignore it
2. Do NOT fix it mid-task (scope creep, contaminates diffs)
3. Apply the boarding protocol:

```
[BROKEN WINDOW NOTED: src/viewer_app.cpp:142 — raw glDrawArrays outside IOpenGLContext]
```

Place this in a `// TODO` at the point of observation. After your current task completes, create a follow-up todo to address it. One window = one todo.

---

## Deprecation Completeness Gate

Deprecating a symbol is NOT complete until every call site is removed or explicitly annotated.

Before marking anything deprecated:
1. Search all call sites: `grep -r "FunctionName" src tests`
2. Either remove them all in this commit, OR annotate each with `// TODO: migrate to [replacement]`
3. If in a public API: add a compiler warning with migration instructions

A `[[deprecated]]` attribute with active, unannotated call sites is annotated debt, not deprecation.

---

## Docs-Same-Commit Rule

When you change a **public interface** (function signature, class API, behavior visible to callers), update documentation in **the same commit**.

Applies to:
- `docs/` markdown files describing the changed interface
- `.github/skills/` files referencing the changed behavior
- Inline doc comments in headers describing the contract

Documentation out of sync with the interface actively misleads. Not a follow-up commit. Same commit.

---

## Red Flags — STOP

- Class owns GL resources but has no cleanup in destructor
- Copy constructor not deleted for a class that owns GL objects
- Deprecated symbol with active call sites and no migration annotation
- Public interface changed but documentation not updated in this commit
- `SDL_Init` called without the subsystem flag for a subsystem being used
- Transitive include reliance without explicit `#include`

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "I'll clean up the GL resources in a follow-up" | GL leaks cause test failures and memory issues; clean up now |
| "The deprecation is signaled with [[deprecated]], that's enough" | Call sites still run deprecated code. Remove them or annotate them. |
| "Docs can be updated separately" | Stale docs mislead the next developer. Same commit is the rule. |
| "It's a transitive include, it works" | Transitive includes break when source headers change. Be explicit. |
| "Removing the call site is enough to remove the feature" | If the Camera method is architecturally valid, keep it. Only the call site was user preference. |
