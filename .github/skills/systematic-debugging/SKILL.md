---
name: systematic-debugging
description: Use when encountering any bug, test failure, build error, or unexpected behavior.
---

# Instructions for Agent

## How This Skill is Invoked

This skill is **mandatory before proposing any fix**. It applies whenever you encounter:
- A test failure, build error, or CI failure
- Unexpected rendering or application behavior
- Anything the user describes as "broken"

When activated, announce: **"I am using the systematic-debugging skill to investigate [brief description of issue]."**

---

## Iron Law

```
YOU MUST INVESTIGATE ROOT CAUSE BEFORE PROPOSING ANY FIX.
No exceptions.
```

Violating the letter of this rule is violating the spirit of this rule.

**Announce at start:** "I am using the systematic-debugging skill to [brief description of issue]."

---

## BEFORE PROCEEDING

- [ ] The full error output has been read completely — not just the last line
- [ ] The failure reproduces consistently with identified reproduction steps
- [ ] I can state: "The root cause is X because Y" with evidence

✓ All met → proceed
✗ Any unmet → continue Phase 1 investigation; do not proceed

---

## When to Use

Use this skill for every one of these situations:

- Test failures (unit, integration, visual regression)
- Build failures (cmake, clang, linker errors)
- Unexpected rendering or application behavior
- Visual regression test failures
- CI pipeline failures
- Performance problems
- Anything the user describes as "broken"

**Use this skill especially** when under time pressure, when "one quick fix" seems obvious, when you've already tried a fix that didn't work, or when you don't fully understand the issue. These are exactly when the iron law is most needed — and most likely to be skipped.

---

## The Four Phases

**Important scope boundary:** A request to "debug" or "investigate" is a request for root cause analysis and a findings report — NOT a request to fix. Present your findings and wait for explicit instruction before applying any fix. This applies even when the fix seems obvious.

Each phase MUST be completed in order.

### Phase 1: Root Cause Investigation

- Read the error message **completely** — do not skim the last line and assume you understand it
- Reproduce consistently — what exact steps trigger the failure?
- Check recent changes — `git diff`, recent commits — what changed?
- For **build errors**: read the FULL compiler output, not just the first error
- For **test failures**: read the FULL test output; run the specific failing test in isolation
- For **visual regression failures**: examine the diff image, compare pixels
- For **CI failures**: reproduce locally with the equivalent command before touching code

BEFORE proceeding to Phase 2, verify:
1. The error message has been read completely.
2. The failure reproduces consistently with identified reproduction steps.
3. You can state: "The root cause is X because Y."

✓ All 3 met → proceed to Phase 2
✗ Any unmet → continue Phase 1 investigation; do not proceed

### Phase 2: Pattern Analysis

- Find working examples of similar code in the codebase
- Compare working vs broken — list every difference, however small
- Read reference implementations COMPLETELY (don't skim)
- Understand ALL dependencies before proposing changes

### Phase 3: Hypothesis and Testing

- Form ONE hypothesis: "I think X is the root cause because Y"
- Make the SMALLEST possible change to test that hypothesis
- One variable at a time — do not bundle multiple changes
- If it didn't work: form a NEW hypothesis; do not pile more fixes on top of the failed attempt

### Phase 4: Implementation

- Fix the root cause, not the symptom
- Write a failing test that reproduces the issue first (see `testing` skill)
- ONE change at a time
- Verify the fix resolves the issue
- Check if the same pattern exists elsewhere in the codebase

---

## The 3-Strikes Architecture Rule

If 3 or more fix attempts have failed: **STOP immediately. Do not attempt a 4th fix.**

Examine whether the approach is architecturally wrong before continuing. Common signals:
- Each fix reveals new coupling in a different place
- Fixes require "massive refactoring" that keeps expanding
- Each fix creates new symptoms elsewhere

**Discuss with the user before attempting more fixes.** The problem is likely not what you think it is.

---

## Red Flags — STOP

If you find yourself thinking any of the following, **STOP and return to Phase 1**:

- "Quick fix for now, investigate later"
- "Just try changing X and see if it works"
- "It's probably X, let me fix that"
- "Add multiple changes, run tests, see what sticks"
- "I don't fully understand but this might work"
- "I've already tried 2 things, one more fix won't hurt"
- "Let me just rerun the test" (for a failing test — rerunning without understanding = ignoring root cause)
- "It works on my machine" (still needs root cause)
- "CI must have a glitch" (still needs root cause)
- "The Flatpak environment must be doing something weird" (still needs systematic investigation)

**All of these mean: STOP. Return to Phase 1.**

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "Issue seems simple, don't need the process" | Simple issues have root causes too. The process is fast for simple bugs. |
| "Emergency, no time for process" | Systematic debugging is FASTER than guess-and-check thrashing. |
| "Just try this first, then investigate" | First fix sets the pattern. Do it right from the start. |
| "I'll write the test after confirming the fix works" | Untested fixes don't stick. A test first proves the fix actually resolves the issue. |
| "Multiple fixes at once saves time" | Can't isolate what worked. Creates new bugs. |
| "I've already looked at it, I know the problem" | Looking at symptoms ≠ understanding root cause. |
| "The test is flaky, just rerun it" | Flaky = non-determinism = root cause needed. |
| "One more fix attempt" (after 2+ failures) | 3+ failures = architectural problem. Question the approach. |
| "I found it — patching it now" | "Debug" means investigate and report. It does not mean fix. Present findings, wait for instruction. |

---

## Project-Specific Commands

```bash
# Build failure — read the first 50 lines (root error is usually near the top)
cmake --build build 2>&1 | head -50

# Test failure — run the specific failing test in isolation
./build/tests/ParticleViewerTests --gtest_filter=TestSuite.TestName

# Visual regression — examine the diff image
# Diffs are written to: tests/visual-regression/diffs/

# CI failure — reproduce locally with the equivalent command before touching code
cmake --build build && ./build/tests/ParticleViewerTests
```

---

## Integration

- `testing` skill — for writing the failing test that reproduces the issue (Phase 4)
- `verification-before-completion` skill — for verifying the fix actually resolved the issue before claiming done

---

## Multi-Component Debugging

When a failure could involve multiple layers (e.g., OpenGL, SDL3, shader, parser, UI), standard per-file investigation is insufficient. Use this instrumentation template to isolate the layer boundary where the failure originates.

### Layer Boundary Isolation Protocol

Before proposing any fix in a multi-component failure:

1. **Name the layers involved.** List every component the failing code path touches in execution order.
   ```
   Example: File picker → ViewerApp → SDL3Context → OpenGL → Shader → Render
   ```

2. **Identify the boundary between "working" and "broken."**
   Ask: at which layer does the correct input produce incorrect output?
   ```
   Layer check: does [Layer A] receive correct input? → YES/NO
   Layer check: does [Layer B] produce correct output given correct input? → YES/NO
   ```
   The first "NO" is the boundary where the failure lives.

3. **Add instrumentation at that boundary.**
   ```cpp
   // Minimal boundary probe — remove after diagnosis
   std::cerr << "[DEBUG boundary] input: " << input << " output: " << output << "\n";
   ```

4. **Run with instrumentation.** Read the output. State: "The failure is between [Layer A] and [Layer B] because [evidence]."

5. **Remove all instrumentation before the fix commit.**

### Common Multi-Component Failure Patterns

| Symptom | Most likely boundary | Investigation action |
|---------|---------------------|---------------------|
| Renders blank / nothing visible | Shader or OpenGL state | Check `glGetError()` after each GL call; verify shader compile/link log |
| Tests pass locally, fail in CI | Environment difference | Check: headless display? OpenGL driver? Font path? File path separator? |
| Visual regression diff shows offset | Camera or viewport transform | Log camera matrix and viewport before render; compare against baseline |
| SDL3 window creates but hangs | SDL3/OpenGL init sequence | Add `SDL_GetError()` and `glGetError()` probes at each init step |
| Flatpak crash on startup | Library version mismatch | Run `ldd` on the binary; check manifest pinned versions |

### Layer Taxonomy for This Project

```
User Input (SDL3 events)
    ↓
ViewerApp (main app logic, state machine)
    ↓
UI layer (ImGui menu — imgui_menu.hpp/cpp)
    ↓
Graphics layer (IOpenGLContext, SDL3Context)
    ↓
OpenGL driver (via GLAD)
    ↓
Shader (GLSL — Viewer-Assets/shaders/)
    ↓
GPU output
```

When debugging, identify which layer in this stack first produces incorrect behavior. Fix at that layer — not at the symptom layer above it.

---

## HeisenBug Patterns in C++

A HeisenBug is a bug that disappears or changes behavior when you attempt to observe or isolate it. In C++, several mechanisms cause this. Work through the table when a bug vanishes under investigation:

| Cause | Symptom | Investigation action |
|-------|---------|---------------------|
| Uninitialized variable | Zero in debug builds (compilers zero-initialize stack in debug mode), garbage in release | Explicitly initialize all variables. Run with address sanitizer (`-fsanitize=address,undefined`). |
| `assert(x = 5)` single-equals trap | Assignment inside `assert` executes in debug, but the entire expression is removed in release (`NDEBUG`) | Audit every `assert` statement for `=` vs `==`. |
| Stack padding | Buffer overflow corrupts neighbor data in release, but is absorbed by debug padding | Run with address sanitizer. Never rely on stack layout. |
| Debugger timing | Race condition disappears because step-through execution serializes threads | Test without the debugger attached. Use thread sanitizer (`-fsanitize=thread`). |
| Compiler optimization | Undefined behavior that is visible at `-O0` is legally optimized away at `-O2` | Compare debug vs release. Run with undefined behavior sanitizer (`-fsanitize=undefined`). |
| Logging/tracing changes timing | A log statement adds a memory barrier or slows execution enough to change a race | Remove logs before final diagnosis. |
| Exception path masks crash | A `catch` block swallows the real fault location | Check exception handler coverage. Add specific re-throw or logging at catch boundaries. |

### HeisenBug Investigation Checklist

Before declaring "the bug disappeared":

- [ ] Compare debug vs release build behavior: `cmake -DCMAKE_BUILD_TYPE=Debug` vs `Release`
- [ ] Audit every `assert` for `=` (assignment) vs `==` (comparison)
- [ ] Compile with sanitizers: `-fsanitize=address,undefined` — does the bug surface?
- [ ] If a threading issue: run with thread sanitizer (`-fsanitize=thread`)
- [ ] Test without the debugger attached — run the binary directly
- [ ] Compile with `-O2` and `-O0` — does behavior differ between the two?
- [ ] Check for uninitialized variables: `-Wuninitialized -Wall -Wextra`

A bug that only reproduces in one build mode is a real bug with a real root cause. The difference in behavior is evidence — use it.
