---
name: execution
description: Autonomous execution protocol for implementing tasks, stories, and bug fixes. Governs planning, subagent use, verification, elegance, and self-correction. Use when executing any non-trivial implementation work.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.1"
  category: execution
  project: Particle-Viewer
---

# Instructions for Agent

## How This Skill is Invoked

This skill is **mandatory for all implementation work**. `copilot-instructions.md` requires it for any task involving code changes, bug fixes, or story execution. Read it before writing code.

---

## Core Principles

These govern every decision during execution:

- **Simplicity First:** Make every change as simple as possible. Impact minimal code. Don't add abstractions unless they earn their complexity.
- **No Laziness:** Find root causes. No temporary fixes. No "it works, ship it." Hold yourself to senior developer standards.
- **Minimal Impact:** Changes should only touch what's necessary. Avoid introducing bugs by limiting blast radius.
- **Own Your Work:** Don't ask for hand-holding. Don't ask permission to continue. Don't report problems — resolve them.

---

## Phase 1: Plan Before Building

**Enter plan mode for ANY non-trivial task** (3+ steps or architectural decisions).

### When to Plan

| Situation | Action |
|---|---|
| Single-file bug fix with obvious cause | Skip planning, just fix it |
| Multi-file change | Plan first |
| Anything touching architecture | Plan first |
| Unclear requirements | Plan first, clarify via subagent research |
| Story with acceptance criteria | Plan first — map criteria to steps |

### How to Plan

1. **Write a todo list** with specific, checkable items using `manage_todo_list`
2. **Write detailed specs upfront** to reduce ambiguity — what files, what changes, what tests
3. **Include verification steps in the plan** — not just building. Plan how you'll prove each change works (which tests, which diffs, which commands)
4. **Verify plan makes sense** before starting implementation — does it cover all acceptance criteria?
5. **If something goes sideways, STOP and re-plan immediately** — don't keep pushing a broken approach

### Planning an INVEST Story

When given a user story with acceptance criteria:

1. Parse each acceptance criterion into a verifiable checkpoint
2. Identify files to create/modify (from Technical Notes or your own analysis)
3. Map criteria → implementation steps → todo items
4. Order steps by dependency (tests can often be written first)
5. Estimate: will this fit in one session? If not, propose splitting to the user

---

## Phase 2: Execute With Discipline

### Work Loop

For each todo item:

```
1. Mark todo as in-progress
2. Implement the change
3. Verify it works (build, test, inspect)
4. Mark todo as completed
5. Commit if the change is a logical unit
6. Move to next item
```

**Never mark a task complete without proving it works.** This means:
- Code compiles
- Tests pass
- `git diff` reviewed for unintended changes
- Ask yourself: **"Would a staff engineer approve this?"**

### Progress Communication

- **Provide a high-level summary at each major step** — what you did, what changed, what's next
- Don't narrate every file edit — summarize at the logical-unit level
- After completing all work, **briefly document what was done** — files changed, tests added, key decisions made

### Commit Cadence

- Commit after each logical unit of work (not after every file edit)
- Each commit should build and pass tests independently
- Use conventional commit messages (see `versioning` skill)
- Don't batch unrelated changes into one commit

---

## Phase 3: Subagent Strategy

Use subagents liberally to keep main context window clean and focused.

### When to Use Subagents

| Task | Use subagent? |
|---|---|
| Researching how an API works | Yes — offload exploration |
| Searching codebase for usage patterns | Yes — parallel analysis |
| Reading multiple large files to understand architecture | Yes — gather context |
| Writing a single function | No — do it directly |
| Complex problem needing multiple angles | Yes — throw more compute at it |

### Subagent Rules

- **One task per subagent** for focused execution
- **Be specific** in the prompt — tell it exactly what to return
- **Trust the output** unless it contradicts what you know
- Subagents cannot write files — they research, you implement

---

## Phase 4: Verification Before Done

**Never declare work complete without evidence.**

### Verification Checklist

For every change, before marking complete:

- [ ] **Build passes:** `cmake --build build`
- [ ] **Tests pass:** `./build/tests/ParticleViewerTests`
- [ ] **Formatting clean:** `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i`
- [ ] **Diff reviewed:** `git diff` — inspect every changed file for unintended changes
- [ ] **Silent output investigated:** If a tool produces no output, verify independently — don't assume success
- [ ] **Staff engineer test:** Would this pass code review? If you hesitate, improve it

### Diff Against Main

When relevant (especially for PRs), compare your changes against the target branch:

```bash
git diff main..HEAD --stat
git diff main..HEAD -- src/
```

This catches drift, accidental changes, and scope creep.

---

## Phase 5: Demand Elegance (Balanced)

For non-trivial changes, pause and ask: **"Is there a more elegant way?"**

### When to Apply

- After getting something working but before committing
- When a fix feels hacky: "Knowing everything I know now, implement the elegant solution"
- When you see an opportunity to simplify adjacent code without scope creep

### When to Skip

- Simple, obvious fixes — don't over-engineer
- Formatting-only changes
- When elegance would require touching unrelated code

### The Challenge

Before presenting your work, challenge it:
- Is this the simplest solution that works?
- Did I introduce unnecessary complexity?
- Would I be proud to show this in a code review?

---

## Phase 6: Autonomous Bug Fixing

When given a bug report, error log, or failing test: **just fix it.**

### Protocol

1. **Read the error** — understand what's actually failing
2. **Reproduce it** — run the failing command/test locally
3. **Find root cause** — don't patch symptoms
4. **Fix it** — implement the correct solution
5. **Verify the fix** — prove the error is gone
6. **Check for related issues** — did the same pattern occur elsewhere?

### CI Failures

When CI fails (formatting, tests, build):

1. Read the CI log — understand what actually failed
2. Reproduce locally with the same command
3. Fix it — don't ask how, don't wait for instructions
4. Verify the fix locally before pushing
5. Push and confirm CI passes

This applies to **any** CI failure you can see, whether the user points it out or not.

### What NOT to Do

- Don't ask "should I fix this?" — yes, fix it
- Don't report the problem without attempting resolution
- Don't propose multiple options and ask the user to choose — pick the best one
- Don't require context switching from the user — handle it yourself
- Don't apply band-aids — find and fix root causes

---

## Phase 7: Self-Correction Loop

**After ANY mistake or user correction during a session:**

1. **Identify the pattern** — what went wrong and why?
2. **Check if it's already documented** — is this in a skill?
3. **If not, update the relevant skill immediately** — don't wait until end-of-session
4. **Write the rule to prevent recurrence** — be specific and actionable
5. **Ruthlessly iterate** — keep refining rules until the mistake rate drops to zero

This differs from end-of-session self-evaluation (which is also mandatory). This is **real-time correction** — fix the skill as soon as you learn the lesson.

### At Session Start

Before starting work, **review the skills relevant to your task** (per `copilot-instructions.md` Session Lifecycle). This is how lessons from past sessions inform current work. Don't just skim — internalize the rules so you don't repeat old mistakes.

### Example

```
Mistake: Trusted clang-format silent output without checking git diff
Correction: Added verification step to code-quality skill Step 1
Rule: "Always check git diff after formatting — silent output doesn't mean success"
```

---

## Anti-Patterns

These behaviors are explicitly prohibited:

1. **Asking permission to continue** — if the task is clear, execute it
2. **Reporting problems without solutions** — always propose or implement a fix
3. **Marking work done without verification** — prove it works first
4. **Pushing through a failing approach** — stop, re-plan, try differently
5. **Over-engineering simple tasks** — match solution complexity to problem complexity
6. **Skipping planning for complex work** — 3+ steps = plan first
7. **Ignoring adjacent issues** — if you see a bug while working, note it or fix it
8. **Trusting tool silence** — verify independently when output is empty

---

## Quick Reference: Execution Flow

```
Task received
    ↓
Is it trivial (1-2 steps)?
    Yes → Execute directly, verify, commit
    No  ↓
Plan: Write todo list with checkable items
    ↓
For each item:
    Mark in-progress → Implement → Verify → Mark complete → Commit
    ↓
    If blocked or broken → STOP → Re-plan → Continue
    ↓
All items done?
    ↓
Final verification:
    Build ✓ | Tests ✓ | Format ✓ | Diff reviewed ✓ | Staff engineer test ✓
    ↓
Self-evaluate (per self-evaluation skill)
    ↓
Done
```
