# Copilot Instructions for Particle-Viewer

This is the agent onboarding document. It identifies the project, lists the skills that contain detailed rules, and states the few critical rules that apply to **every** task. All detailed guidelines live in skills.

## Before Every Response — Run This Checklist

Before generating any output, ask yourself:

1. **About to write C++ code?** → Have I written a failing test first? If not: stop. Write the test.
2. **About to say "done", "should work", or express satisfaction?** → Have I run `cmake --build build && ./build/tests/ParticleViewerTests` in this session? If not: run it now.
3. **Encountered a bug or failure?** → Have I traced the root cause (not just guessed)? If not: load `systematic-debugging`.
4. **Non-trivial task (3+ steps)?** → Have I loaded the required skills from the table below? If not: load them now.
5. **Forming a theory or assumption?** → "I think" is not acceptable. Do I have empirical evidence (code, test output, documentation)? If not: dispatch a subagent to confirm before proceeding.
6. **About to finalize a plan?** → Have I answered "What is this NOT addressing?" If no answer, or if the answer reveals a gap: stop. Revise the plan. Dispatch a Skeptic Agent for any 3+ todo plan. See `writing-plans` skill.
7. **About to start multi-step work?** → Have I stated requirements back in my own words and labeled ambiguities `[UNCLEAR: ...]`? If not: state them before planning. See `writing-plans` skill.
8. **Unclear approach, multiple valid solutions, or architecture impact?** → HARD-GATE: load `brainstorming` and answer all design questions before writing any code.
9. **About to dispatch a subagent?** → Load `subagent-driven-development`. One clear objective per agent. State the return format. Verify results before propagating claims.

**This checklist applies on EVERY turn. Not just session start.**

## Meta-Level Priority — Overrides Everything

When instructions conflict, this order governs **at the meta level**:

| Priority | Source | What it means |
|----------|--------|---------------|
| **1 — User** | Explicit user instructions (direct requests, corrections, project config overrides) | Always wins. If the user says "skip tests this time," do it. |
| **2 — Skills** | Loaded skill files | Override default model behavior where they conflict |
| **3 — Default** | Default model behavior | Only applies when no skill or user instruction covers the situation |

**If the user overrides an Iron Law:** follow the user. State the override explicitly: "Proceeding without [X] as instructed — noting this deviates from Iron Law N."

---

## Iron Laws — Always Active

These apply in every session, every task, every model. No exceptions unless the user explicitly overrides (Meta-Level Priority 1 above).

| # | Law |
|---|-----|
| 1 | **NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST.** Write test → watch it fail → write code. See `testing` skill. |
| 2 | **NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION.** Run `cmake --build build && ./build/tests/ParticleViewerTests` in THIS session. Evidence must be inline. See `verification-before-completion` skill. |
| 3 | **NO FIXES WITHOUT ROOT CAUSE INVESTIGATION.** Follow the 4-phase protocol. See `systematic-debugging` skill. |
| 4 | **EVERY COMMIT USES CONVENTIONAL FORMAT.** `<type>[scope]: <description>` — wrong format breaks release automation. See `versioning` skill. |
| 5 | **FORMAT BEFORE EVERY COMMIT.** `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i`. CI rejects violations. |
| 6 | **FAILURE IS RECOVERABLE. FALSE CONFIDENCE IS NOT.** "Should work" is banned. No completion claim without inline evidence. See `honesty` skill. |
| 7 | **CLARIFY FIRST. PLAN BEFORE CODE. NO PLACEHOLDERS.** Restate requirements, label `[UNCLEAR:]`, build todos before touching code. See `writing-plans` skill. |
| 8 | **NO CODE UNTIL THE DESIGN GATE IS PASSED.** Unclear approach, architecture impact, or multiple valid solutions = load `brainstorming` first. |
| 9 | **DISPATCH BEFORE GUESSING.** No theory, assumption, or memory is a basis for action. If you cannot point to a file, line, or test output — dispatch a subagent. See `subagent-driven-development` skill. |
| 10 | **DISPATCH REVIEWERS AFTER EVERY TODO.** Stage 1: spec compliance. Stage 2: code quality. Never advance to the next todo with an unreviewed todo behind you. See `subagent-driven-development` skill. |
| 11 | **THE BROWN M&M LAW.** Named after Van Halen's 1982 World Tour rider (Article 126: M&Ms backstage, no brown ones — buried in technical requirements as a canary. Brown M&Ms meant the contract wasn't read; safety requirements were also at risk). Every skill that defines a `## Canary` section requires the agent to produce that canary output when applying the skill. A missing canary is a trust violation. See `subagent-driven-development` skill. |

**If you are tempted to rationalize past any of these: that thought is the rationalization. Stop. Follow the rule.**

**Violating the letter of any Iron Law is violating the spirit of it.**

**`honesty` is always active and applies on every turn. Full mechanics in `.github/skills/honesty/SKILL.md`.**

Particle-Viewer is a C++ OpenGL-based viewer for N-Body simulations — viewing 3D particle data, taking screenshots, and rendering videos.

**Tech stack:** C++20, OpenGL, CMake, Google Test, SDL3, GLM, Dear ImGui, stb, Tiny File Dialogs, GLAD.

## Skills Directory

Each skill owns one domain. Read the skill before working in that domain. **Never duplicate skill content in this file.**

Skills are organized into **DDD bounded contexts**. Sub-domain skills (e.g., `visual-regression-testing` under QUALITY) have their own iron law and are invoked independently — but the parent skill routes to them. The full DDD map lives in `writing-skills`. New skills require ≥1% session invocation frequency to justify creation; use reference docs otherwise.

### EXECUTION context

| Skill | Path | Domain |
|-------|------|--------|
| `execution` | `.github/skills/execution/` | Work loop, commit rhythm, mode declaration, behavior preservation |
| `writing-plans` | `.github/skills/writing-plans/` | Plan building, scope gates, Skeptic Agent, YAGNI/PPP/STTCPW |
| `brainstorming` | `.github/skills/brainstorming/` | HARD-GATE design exploration before any implementation begins |
| `subagent-driven-development` | `.github/skills/subagent-driven-development/` | Subagent dispatch, 2-stage review, empirical evidence, worktrees |
| `dispatching-parallel-agents` | `.github/skills/dispatching-parallel-agents/` | Parallel agent dispatch, isolation, result aggregation |
| `using-git-worktrees` | `.github/skills/using-git-worktrees/` | Parallel agent isolation, A/B testing, branch safety for subagents |

### QUALITY context

| Skill | Path | Domain |
|-------|------|--------|
| `testing` | `.github/skills/testing/` | AAA pattern, naming, mocks, test taxonomy — routes to sub-domains |
| `visual-regression-testing` | `.github/skills/visual-regression-testing/` | ↳ OpenGL visual testing boundary, baseline approval, tolerance, camera framing |
| `code-quality` | `.github/skills/code-quality/` | clang-format, clang-tidy, naming conventions, pre-commit — routes to cpp-patterns |
| `cpp-patterns` | `.github/skills/cpp-patterns/` | ↳ GL resource management, SDL3 gotchas, DRY, Broken Window, Deprecation, Docs-Same-Commit |
| `contract-testing` | `.github/skills/contract-testing/` | ↳ Contract verification for every abstract type and interface |
| `cpp-safety` | `.github/skills/cpp-safety/` | ↳ Scope-bound resource ownership, exception safety, destructor rules |
| `oop-principles` | `.github/skills/oop-principles/` | ↳ Is-A/Has-A gate, SOLID check before any class hierarchy |
| `verification-before-completion` | `.github/skills/verification-before-completion/` | Evidence-first verification before every completion claim or commit |
| `systematic-debugging` | `.github/skills/systematic-debugging/` | Root cause investigation protocol for bugs, failures, and errors |

### DELIVERY context

| Skill | Path | Domain |
|-------|------|--------|
| `versioning` | `.github/skills/versioning/` | Conventional commits, PR titles, semantic versioning, releases |
| `build` | `.github/skills/build/` | CMake build, dependencies, Flatpak packaging, troubleshooting |
| `workflow` | `.github/skills/workflow/` | CI/CD pipelines, artifacts, permissions |
| `flatpak` | `.github/skills/flatpak/` | Flatpak packaging, OpenGL/SDL3 runtime, NVIDIA GL workarounds |
| `finishing-a-development-branch` | `.github/skills/finishing-a-development-branch/` | Branch ceremony, squash strategy, PR creation, post-merge cleanup |

### REVIEW context

| Skill | Path | Domain |
|-------|------|--------|
| `architecture-review` | `.github/skills/architecture-review/` | Layer boundary, dependency direction, and IOpenGLContext compliance |
| `infrastructure-review` | `.github/skills/infrastructure-review/` | CI/CD pipelines, CMake reproducibility, Flatpak manifest compliance |
| `skill-reviewer` | `.github/skills/skill-reviewer/` | Review skill files for completeness, iron laws, and gate elements |
| `requesting-code-review` | `.github/skills/requesting-code-review/` | Targeted review requests, SHA-based dispatch, agent pre-review |
| `receiving-code-review` | `.github/skills/receiving-code-review/` | Processing review feedback without performative agreement |

### REFLECTION context

| Skill | Path | Domain |
|-------|------|--------|
| `self-evaluation` | `.github/skills/self-evaluation/` | End-of-session review, lessons learned |
| `session-postmortem` | `.github/skills/session-postmortem/` | Retrospective behavioral analysis of a completed agent session |

### KNOWLEDGE context

| Skill | Path | Domain |
|-------|------|--------|
| `documentation` | `.github/skills/documentation/` | Docs conventions, linking, formatting, skill authoring |
| `writing-skills` | `.github/skills/writing-skills/` | Skill authoring standard, DDD map, anatomy gate |

### PRODUCT context

| Skill | Path | Domain |
|-------|------|--------|
| `user-story-generator` | `.github/skills/user-story-generator/` | INVEST-aligned story creation — routes to user-story-estimation |
| `user-story-estimation` | `.github/skills/user-story-estimation/` | ↳ Effort estimation, premium request counts, model tier selection |

### BEHAVIOR context

| Skill | Path | Domain |
|-------|------|--------|
| `honesty` | `.github/skills/honesty/` | **Always active.** Trust mechanics, confidence vocabulary, process language, talk-straight. Hardcoded into session-start hook. Load the full skill for postmortems or communication audits. |
| `session-bootstrap` | `.github/skills/session-bootstrap/` | Session lifecycle: On Start skill routing + On Finish self-evaluation |

### Agent Prompt Templates

Reusable agent prompts live in `.github/agents/`. Use these when dispatching subagents via the `task` tool:

| Template | Use when |
|----------|----------|
| `implementer.md` | Dispatching an agent to implement a feature in a worktree |
| `skeptic.md` | Reviewing a plan for gaps before implementation begins |
| `spec-compliance-reviewer.md` | Stage 1 post-todo review: does implementation match spec? (always first) |
| `code-quality-reviewer.md` | Stage 2 post-todo review: code quality, correctness, standards (only after Stage 1 passes) |
| `researcher.md` | Empirically confirming or denying a hypothesis |
| `postmortem-reviewer.md` | External review of a completed agent session retrospective |
| `explorer.md` | Read-only multi-file research across many independent targets |
| `architecture-reviewer.md` | Per-file layer boundary and IOpenGLContext compliance review |
| `infrastructure-reviewer.md` | Per-file CI/CMake/Flatpak compliance review |

### Instruction Priority Hierarchy

This is the **internal** rule precedence (within the skills system). For meta-level priority (user vs. skills vs. default), see the section at the top of this file.

| Priority | Source | Scope |
|----------|--------|-------|
| 1 (highest) | Iron Laws in this file | Every turn, every model, no exceptions |
| 2 | Loaded skill files | Active when the relevant domain is in play |
| 3 | Session context / plan.md | Current session scope only |
| 4 | Inferred convention | Only when no explicit rule exists |

**Never use Priority 4 to override Priority 1–3.** If a skill is not loaded, the default is the iron law, not your best guess.

## Critical Rules (Apply to Every Task)

1. **Format before committing.** `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i`
2. **Conventional commits always.** `<type>[scope]: <description>` — see `versioning` skill.
3. **Build and test before pushing.** `cmake --build build && ./build/tests/ParticleViewerTests`

**Session lifecycle (skill loading + self-evaluation) is in the `session-bootstrap` skill.**

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
scratch/                  # Session workspace for exploratory/intermediate files
                          # Use for: large text dumps, intermediate analysis, theory-testing artifacts
                          # DO NOT commit scratch/ contents — it is .gitignored
                          # Examples: DRY_full.txt, Mock_full.txt, research summaries
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
| Flatpak GL/SDL3 gotchas | `.github/skills/flatpak/SKILL.md` |
| Microsoft C++ Core Guidelines | https://isocpp.github.io/CppCoreGuidelines/ |
| Google Test docs | https://google.github.io/googletest/ |

## When in Doubt

1. Read the relevant skill before making assumptions.
2. Check existing code for patterns.
3. Run clang-format and clang-tidy.
4. Build and test locally before pushing.
