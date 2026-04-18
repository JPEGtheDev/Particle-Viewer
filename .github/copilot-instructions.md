# Copilot Instructions for Particle-Viewer

This is the agent onboarding document. It identifies the project, lists the skills that contain detailed rules, and states the few critical rules that apply to **every** task. All detailed guidelines live in skills.

## Iron Laws — Always Active

These apply in every session, every task, every model. No exceptions.

| # | Law |
|---|-----|
| 1 | **NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST.** Write test → watch it fail → write code. See `testing` skill. |
| 2 | **NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION.** Run `cmake --build build && ./build/tests/ParticleViewerTests` in THIS session. See `verification-before-completion` skill. |
| 3 | **NO FIXES WITHOUT ROOT CAUSE INVESTIGATION.** Follow the 4-phase protocol. See `systematic-debugging` skill. |
| 4 | **EVERY COMMIT USES CONVENTIONAL FORMAT.** `<type>[scope]: <description>` — wrong format breaks release automation. See `versioning` skill. |
| 5 | **FORMAT BEFORE EVERY COMMIT.** `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i`. CI rejects violations. |

**If you are tempted to rationalize past any of these: that thought is the rationalization. Stop. Follow the rule.**

## Project Overview

Particle-Viewer is a C++ OpenGL-based viewer for N-Body simulations — viewing 3D particle data, taking screenshots, and rendering videos.

**Tech stack:** C++20, OpenGL, CMake, Google Test, SDL3, GLM, Dear ImGui, stb, Tiny File Dialogs, GLAD.

## Skills Directory

Each skill owns one domain. Read the skill before working in that domain. **Never duplicate skill content in this file.**

| Skill | Path | Domain |
|-------|------|--------|
| `versioning` | `.github/skills/versioning/` | Conventional commits, PR titles, semantic versioning, releases |
| `build` | `.github/skills/build/` | CMake build, dependencies, Flatpak packaging, troubleshooting |
| `code-quality` | `.github/skills/code-quality/` | clang-format, clang-tidy, naming, C++ patterns, pre-commit |
| `testing` | `.github/skills/testing/` | AAA pattern, naming, mocks, visual regression, coverage |
| `workflow` | `.github/skills/workflow/` | CI/CD pipelines, artifacts, permissions, Flatpak GL gotchas |
| `documentation` | `.github/skills/documentation/` | Docs conventions, linking, formatting, skill authoring |
| `execution` | `.github/skills/execution/` | Autonomous execution protocol, planning, verification, bug fixing |
| `systematic-debugging` | `.github/skills/systematic-debugging/` | Root cause investigation protocol for bugs, failures, and errors |
| `verification-before-completion` | `.github/skills/verification-before-completion/` | Evidence-first verification before every completion claim or commit |
| `user-story-generator` | `.github/skills/user-story-generator/` | INVEST-aligned story creation |
| `self-evaluation` | `.github/skills/self-evaluation/` | End-of-session review, lessons learned |

## Critical Rules (Apply to Every Task)

These are the only rules stated here because they cut across all skills:

1. **Format before committing.** Run `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i` before every commit. CI will reject unformatted code.
2. **Conventional commits always.** Every commit message and every PR title must use `<type>[scope]: <description>`. See `versioning` skill.
3. **PR title = squash commit message.** Do NOT change an existing PR title unless the user explicitly asks.
4. **Build and test before pushing.** `cmake --build build && ./build/tests/ParticleViewerTests`
5. **Never commit from CI.** Pipelines are read-only. See `workflow` skill.
6. **Comment hygiene.** If you write or encounter a code comment longer than 5 lines of explanation, migrate the detail to the appropriate skill or `docs/` file and replace with a 1-line reference. Code comments explain *what*; skills and docs explain *why*.
7. **Self-evaluate before finishing.** See [Session Lifecycle](#session-lifecycle) below — this is mandatory, not optional.
8. **Todo list for every task.** Use `manage_todo_list` at session start to plan work. Mark items in-progress before starting, completed immediately after finishing. No exceptions — even small tasks get a todo list so progress is visible.

## Session Lifecycle (MANDATORY)

Every session follows this lifecycle. All models (Opus, Sonnet, Haiku) MUST execute both phases.

### On Start — Skill Check

Before writing code, read the skill(s) relevant to your task from the Skills Directory above. If the task touches multiple domains, read multiple skills.

**Minimum skill loads by task type:**

| If the task involves… | MUST read these skills |
|---|---|
| Any implementation work | `execution` |
| Writing or editing C++ code | `execution`, `code-quality` |
| Writing or editing tests | `execution`, `code-quality`, `testing` |
| Creating a PR or commit | `versioning`, `verification-before-completion` |
| CI/CD or workflow changes | `workflow` |
| Build system or dependency changes | `build` |
| Writing or editing documentation | `documentation` |
| Bug fixes or error resolution | `execution`, `systematic-debugging` |
| Any failure or unexpected behavior | `systematic-debugging`, `verification-before-completion` |

If unsure, read `code-quality` — it applies to nearly every code task.

### On Finish — Self-Evaluate and Compact

**Before your final message to the user**, you MUST do all of the following:

1. **Read** `.github/skills/self-evaluation/SKILL.md` and follow its steps.
2. **Identify lessons learned** — mistakes made, user corrections, patterns discovered.
3. **Check existing skills** — is the lesson already documented? If yes, skip.
4. **Apply updates** — for High/Medium priority lessons, update the relevant skill file and bump its version in the YAML frontmatter. Commit the skill update with the session's work.
5. **Compact** — scan any files you touched for bloated comments or duplicated docs. Migrate detail to skills/docs and leave 1-line references.
6. **Report** — include a brief `### Session Self-Evaluation` block in your final message:
   ```
   ### Session Self-Evaluation
   Lessons: [count] | Skills updated: [list or "None"] | Compacted: [files or "None"]
   ```

If you have nothing to report, still include the block with zeroes. This ensures the behavior is habitual.

## Source Code Layout

```
src/
├── *.cpp, *.hpp          # Core source (viewer_app, camera, shader, particle, etc.)
├── ui/                   # ImGui menu system
├── graphics/             # IOpenGLContext, SDL3Context
├── testing/              # PixelComparator (test utility, shipped in src/)
├── shaders/              # GLSL shaders (auto-copied to Viewer-Assets/shaders/ at build)
├── glad/                 # GLAD OpenGL loader (embedded)
└── tinyFileDialogs/      # File dialog library (embedded)

tests/
├── core/                 # Unit tests for src/ classes
├── integration/          # Multi-component interaction tests
├── testing/              # Tests for test utilities
├── visual-regression/    # Headless visual regression tests
└── mocks/                # MockOpenGL, MockOpenGLContext

docs/                     # Human-readable guides and standards
.github/skills/           # Copilot agent skills (see table above)
```

### Key Source Files

| File | Purpose |
|------|---------|
| `main.cpp` | Entry point — creates and runs `ViewerApp` |
| `viewer_app.hpp/cpp` | Main app class; owns window, render pipeline, all state |
| `camera.hpp` | Header-only 3D camera system |
| `shader.hpp` | Header-only shader program management |
| `particle.hpp` | Header-only particle data (`std::vector<glm::vec4>`) |
| `settingsIO.hpp` | Header-only config file I/O |
| `Image.hpp/cpp` | RGBA image class with PPM/PNG save/load |
| `ui/imgui_menu.hpp/cpp` | ImGui main menu bar |
| `graphics/SDL3Context.hpp/cpp` | SDL3 window + OpenGL context with retry logic |
| `testing/PixelComparator.hpp/cpp` | Pixel comparison for visual regression |

## Additional Resources

| Resource | Path |
|----------|------|
| Coding standards | `docs/CODING_STANDARDS.md` |
| Testing standards | `docs/TESTING_STANDARDS.md` |
| Conventional commits | `docs/CONVENTIONAL_COMMITS.md` |
| Release process | `docs/RELEASE_PROCESS.md` |
| Visual regression guide | `docs/testing/visual-regression.md` |
| Integration test guide | `docs/testing/integration-tests.md` |
| ImGui integration | `docs/IMGUI_INTEGRATION.md` |
| Window management | `docs/WINDOW_MANAGEMENT.md` |
| Camera positioning lessons | `docs/visual-regression/camera-positioning-lessons-learned.md` |
| Flatpak GL/SDL3 gotchas | `.github/skills/workflow/references/FLATPAK_GL_GOTCHAS.md` |
| Microsoft C++ Core Guidelines | https://isocpp.github.io/CppCoreGuidelines/ |
| Google Test docs | https://google.github.io/googletest/ |

## When in Doubt

1. Read the relevant skill before making assumptions.
2. Check existing code for patterns.
3. Run clang-format and clang-tidy.
4. Build and test locally before pushing.
