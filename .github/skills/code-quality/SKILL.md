---
name: code-quality
description: Use when writing or reviewing C++ code, running pre-commit checks, or addressing formatting, naming, or static analysis violations.
---

## Iron Law

```
NO UNFORMATTED OR UNTIDY CODE SHIPS
```

Run clang-format AND clang-tidy BEFORE every commit. CI will reject violations. No exceptions.

Violating the letter of this rule is violating the spirit of this rule.

**Announce at start:** "I am using the code-quality skill to [format/lint/review] [description]."

---

# Instructions for Agent

## How This Skill is Invoked

In VS Code, users will activate this skill by:
- Typing `@workspace /code-quality [description]` in Copilot Chat
- Or asking: "Format the code", "Fix clang-tidy warnings", "What naming convention?", "Pre-commit checklist"

When activated, apply the formatting, naming, and pattern rules below.

---

## Core Principle: Tools Enforce Style, Humans Write Logic

Formatting and naming are automated via `.clang-format` and `.clang-tidy`. Never manually format code — run the tools.

---

## Step 1: Pre-Commit Checklist (MANDATORY)

Run these before **every** commit:

```bash
# 1. Format ALL changed C++ files
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# 2. Check what changed
git diff --name-only | head -20

# 3. Spot-check any files you touched (don't trust silent tool output)
# Look for: trailing semicolons after function }, exception handling, 
# multi-declaration statements, bracing consistency
git diff src/[touched_file].cpp | head -100

# 4. Verify formatting passes (same check CI runs)
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# 5. Build and test
cmake --build build
./build/tests/ParticleViewerTests

# 6. (Optional) Static analysis
clang-tidy src/main.cpp -- -Isrc/glad/include
```

**⚠️ Critical:** Do NOT trust that `clang-format --dry-run -Werror` with no output means success. Always visually inspect `git diff` of modified files. Silent tool output can mask formatting issues.

---

## Step 2: Formatting Rules (clang-format)

Auto-enforced via `.clang-format`. Key settings:

| Rule | Value |
|------|-------|
| Standard | C++20 |
| Indentation | 4 spaces (no tabs) |
| Line length | 120 characters |
| Functions/classes braces | Allman (new line) |
| Control structure braces | K&R (same line) |
| Include order | C++ standard → system → external libs → project headers |
| Pointer alignment | Left (`int* ptr`) |

Full configuration: `.clang-format`

---

## Step 3: Human-Reviewable Formatting Patterns

These require **manual inspection** after running clang-format — tools don't always catch them:

| Issue | Fix | Example |
|-------|-----|---------|
| Semicolon after function closing brace | Remove trailing `;` | `void foo() { }` not `void foo() { };` |
| Empty constructor syntax | Use `= default` | `Shader() = default;` not `Shader() {};` |
| Exception catching by value | Catch by const reference | `catch (const std::ifstream::failure& e)` |
| Multiple declarations in one line | Split to separate lines | `uint32_t w; uint32_t h;` not `uint32_t w, h;` |

**Always check `git diff` for these patterns after formatting.**

---

## Step 4: Naming Conventions (clang-tidy enforced)

| Element | Convention | Example |
|---------|-----------|---------|
| Classes/Structs/Enums | `PascalCase` | `ParticleSystem`, `RenderSettings` |
| Functions/Methods | `camelCase` (preferred, not enforced) | `initializeBuffers`, `getParticleCount` |
| Variables/Parameters | `snake_case` | `particle_count`, `delta_time` |
| Member variables | `snake_case` + trailing `_` for private | `window_width`, `camera_` |
| Constants/Macros | `UPPER_CASE` | `MAX_PARTICLES`, `PI` |
| File names | `snake_case` | `particle_system.hpp` |
| Include guards | `<PROJECT>_<PATH>_<FILE>_H` | `PARTICLE_VIEWER_PARTICLE_SYSTEM_H` |
| Namespaces | `snake_case` | `particle_viewer` |

---

## Step 5: Static Analysis (clang-tidy)

```bash
# Analyze a source file
clang-tidy src/main.cpp -- -Isrc/glad/include

# Analyze using compilation database
clang-tidy src/particle.hpp -p build

# Auto-fix (always review auto-fix output before committing — it can change behavior)
clang-tidy -fix src/main.cpp -- -Isrc/glad/include
```

Configuration in `.clang-tidy` enforces:
- `modernize-*` — Modern C++ practices (smart pointers, nullptr, auto, range-based loops)
- `readability-*` — Const correctness, function complexity (≤25 cognitive, ≤50 statements, <5 params)
- `cppcoreguidelines-*` — Microsoft C++ Core Guidelines
- `performance-*`, `bugprone-*`, `portability-*`, `clang-analyzer-*`

Header filter excludes embedded libs: `glad`, `tinyFileDialogs`, `stb_*`.

**CI status:** clang-tidy runs in CI as advisory (non-blocking). clang-format is blocking.

---

## Step 6: Project-Specific C++ Patterns

For runtime patterns (data organization, error handling, memory management, OpenGL/SDL3 usage, DRY, Broken Window Protocol, Deprecation, Docs-Same-Commit), load the `cpp-patterns` skill (`.github/skills/cpp-patterns/`).

---

## Step 7: Adding a Feature / Fixing a Bug

### New Feature Workflow
1. **Scan the class interface** — before writing integration code that calls methods on an existing class, verify which members are public/private. Classes like `Camera` have a mix; don't assume public.
2. Make code changes following naming conventions
3. Add unit tests in `tests/core/` (see `testing` skill)
4. Run `clang-format -i` on ALL changed files
5. Build and verify tests pass
6. Commit: `feat: description`

### Removing Features / User-Requested Changes
When removing a gamepad feature or call site from `viewer_app.cpp` at user request, **do not also delete the supporting `Camera` public method**. The Camera API is stable across sessions; call sites in `viewer_app` change frequently with user preferences. Removing `isRenderingSphere()` when L3/R3 was dropped meant restoring it when L3/R3 came back one session later. Only remove a Camera method if it is architecturally wrong, not merely unused at the current moment.

### Bug Fix Workflow
1. Write a failing test that reproduces the bug
2. Fix the code
3. Verify test passes, run full suite
4. Run `clang-format -i` on changed files
5. Commit: `fix: description`

---

## Review Checklist

- [ ] `clang-format -i` run on all changed files
- [ ] `clang-format --dry-run -Werror` passes
- [ ] Build succeeds
- [ ] All tests pass
- [ ] Conventional commit message format used
- [ ] No raw `new`/`delete` — use RAII or smart pointers
- [ ] GL resources cleaned up in destructors
- [ ] Headers are self-contained
- [ ] If a public interface changed: documentation updated in same commit (see cpp-patterns skill)
- [ ] If a symbol is deprecated: all call sites removed or annotated (see cpp-patterns skill)

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "I'll run clang-format once at the end before the PR" | Format after every meaningful change. Catch issues early, not in bulk. |
| "clang-tidy has too many false positives, I'll skip it" | Fix or suppress with justification in code. Suppression without reason is tech debt. |
| "The naming is close enough to the convention" | Exact naming prevents confusion across sessions and contributors. |
| "Formatting is cosmetic, doesn't affect behavior" | Unformatted code gets rejected by CI. It's a hard gate, not a preference. |
| "I'll clean up the style in a follow-up PR" | Style debt compounds. Clean it now while context is fresh. |
| "The auto-formatter will handle it" | Run the auto-formatter explicitly — it doesn't run itself. |

---

## Red Flags — STOP

If you catch yourself thinking any of these, stop and follow the rule:
- About to commit without running `clang-format -i`
- "clang-tidy warning but it seems like a false positive"
- "The naming is slightly different but close enough"
- "I'll clean up formatting in the next commit"
- "The CI will format it automatically"
- Writing code and planning to format "later in this session"

**All of these mean: Run `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i` and `clang-tidy` before every commit. No exceptions.**

---

## Reference

- Full coding standards: [`docs/CODING_STANDARDS.md`](../../../docs/CODING_STANDARDS.md)
- clang-format config: `.clang-format`
- clang-tidy config: `.clang-tidy`
- Commit format: `versioning` skill (`.github/skills/versioning/`)
- Testing patterns: `testing` skill (`.github/skills/testing/`)
