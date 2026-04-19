---
name: subagent-driven-development
description: Use when delegating implementation tasks, confirming theories, running parallel research, or reviewing completed work.
---

## Iron Laws

```
YOU MUST DISPATCH BEFORE GUESSING — SUBAGENTS ARE CHEAP, WRONG ASSUMPTIONS ARE EXPENSIVE.
YOU MUST DISPATCH REVIEWERS AFTER EVERY TODO — SPEC COMPLIANCE FIRST, THEN CODE QUALITY.
No exceptions.
```

Violating the letter of these rules is violating the spirit of these rules.

**Announce at start:** "I am using the subagent-driven-development skill to [dispatch/review/confirm] [brief description]."

---

## The SDD Loop

```
Pick up todo
    |
    v
Dispatch implementer subagent (implementer.md)
    |
    v
Implementer returns status code
    |
    +-- NEEDS_CONTEXT --> Provide the missing information. Re-dispatch.
    |
    +-- BLOCKED --> Assess. Provide context if possible. Otherwise escalate to user.
    |
    +-- PARTIAL --> Read completed/remaining split.
    |                Verify what was completed (build + tests).
    |                Create new todo(s) for remaining work.
    |                Proceed to review for the completed portion only.
    |
    +-- DONE_WITH_CONCERNS --> Read concerns before proceeding to review.
    |                          Address concerns if correctness risk. Otherwise proceed.
    |
    +-- DONE
         |
         v
Stage 1: Dispatch spec-compliance-reviewer (spec-compliance-reviewer.md)
    |
    +-- GAPS --> Implementer fixes gaps. Re-dispatch Stage 1.
    |
    +-- PASS
         |
         v
Stage 2: Dispatch code-quality-reviewer (code-quality-reviewer.md)
    |
    +-- REQUEST CHANGES --> Implementer fixes. Re-dispatch Stage 2.
    |
    +-- APPROVE or APPROVE WITH NITS
         |
         v
Mark todo done. Reload relevant skills (session-bootstrap refresh rule).
Pick up next todo.
    |
    v
(After all todos) → Dispatch final code reviewer → finishing-a-development-branch
```

**Do not advance past any todo until both Stage 1 and Stage 2 are PASS/APPROVE.**

---

## BEFORE PROCEEDING — SDD Dispatch Gate

Before dispatching any subagent:

1. The todo has a single, clear objective — no compound tasks bundled together.
2. The agent prompt includes all necessary context: file paths, constraints, and return format.
3. A dedicated worktree exists for any agent that modifies files; `using-git-worktrees` skill is loaded.
4. If a pre-built template exists in `.github/agents/` for this task type: use it instead of injecting rules inline. Available templates: `implementer.md`, `skeptic.md`, `spec-compliance-reviewer.md`, `code-quality-reviewer.md`, `researcher.md`, `postmortem-reviewer.md`, `explorer.md`, `architecture-reviewer.md`, `infrastructure-reviewer.md`.
5. Agent type is correct for the task: explore for read-only research, code-review for analysis, general-purpose+worktree for file modifications, task for build/test/lint.

✓ All 5 met → dispatch the agent
✗ Any unmet → refine the todo, complete the prompt, create the worktree, or select the correct agent type before dispatching

---

## Why Subagents Are Mandatory

Using subagents is discipline, not convenience:

- **Main context fills fast.** Once full, early instructions — including iron laws — are evicted. Keep context lean.
- **Subagents search without your bias.** They confirm or deny theories precisely because they lack your assumptions.
- **Parallel dispatch is faster.** Five explore agents simultaneously beat five sequential searches.
- **Subagents keep your diff clean.** Exploratory dead-ends and interim summaries never pollute main context.

---

## Empirical Evidence Mandate

**No theory, assumption, or belief is acceptable as a basis for action. Every claim must be backed by empirical evidence.**

**Acceptable evidence:**
- Code you read yourself — opened the file, saw the line
- Test output you ran yourself — executed the command, read the result
- Documentation cross-checked against actual source code
- A/B test results — implemented both, compared outcomes
- A targeted test written to prove or disprove the theory

**Not acceptable:**
- Memory ("I remember this worked")
- Confidence ("I'm sure the parser handles this")
- Assumption ("It should work because Y")
- Prior success ("It worked last time")

If you cannot point to a specific file, line, or test run — dispatch a subagent.

## Red Flags — STOP

These thoughts mean stop immediately:

| Thought | Required action |
|---------|----------------|
| "I think the issue is..." | Dispatch explore agent → read the actual code |
| "I dispatched a subagent — I'll also work on this while waiting" | STOP. The only permissible next call is read_agent. "I'll wait" is a binding constraint, not a statement of intent. |
| "This should work because..." | Run it. Read the output. |
| "I'm confident that..." | State the evidence, or dispatch to get it |
| "It probably passes..." | Run the test suite |
| "I remember that..." | Memory is always unverified — dispatch |
| "Based on how it usually works..." | Dispatch to confirm the actual behavior |
| "Dispatching a file-modifying agent without creating a worktree first" | STOP. Create the worktree and load `using-git-worktrees` before dispatch. |
| "A template exists but I'll build the prompt manually" | STOP. Use the pre-built template from `.github/agents/`. Do not reinvent it. |

---

## Dispatch Decision Table

| Task | Dispatch? | Type |
|------|-----------|------|
| Exploring unfamiliar APIs or libraries | Yes | explore agent |
| Scanning 5+ files for patterns | Yes | explore agent |
| Confirming a theory or assumption | Yes | explore agent |
| Validating a plan before implementation | Yes | Skeptic Agent (see writing-plans skill) |
| Code review (per-file) | Yes | code-review agent, 1 per file |
| Skill review | Yes | general-purpose + skill-reviewer skill |
| Multi-file implementation with file isolation | Yes | general-purpose + git worktree |
| Quick grep/glob in 1–2 files | No | do inline |
| Reading one known file | No | do inline |
| Single-step trivial command | No | do inline |

**Max 4 concurrent agents.** Beyond that, results compete for context and quality drops.

---

## Implementer Status Codes

Every subagent doing implementation work must report one of these five codes. Require it in every implementer prompt. Do not accept a response that does not include one.

| Code | Meaning | Your response |
|------|---------|---------------|
| `DONE` | Task complete, all verification passed, no concerns | Proceed to Stage 1 review |
| `DONE_WITH_CONCERNS` | Complete but flagged issues for dispatcher review | Read concerns. Address if correctness risk. Otherwise proceed to Stage 1. |
| `PARTIAL` | Partially complete — some items done and verified, rest not done | Verify completed portion. Create new todo(s) for remaining work. Proceed to Stage 1 for completed portion only. |
| `NEEDS_CONTEXT` | Cannot proceed — specific missing information listed | Provide the missing information. Re-dispatch. |
| `BLOCKED` | Cannot proceed — external dependency or environment issue described | Assess blocker. Provide context if possible. Otherwise escalate to user. |

---

## 2-Stage Review Protocol

Every completed implementation task requires two reviews in this order. This is mandatory — not optional — after every single todo.

```
Stage 1: Spec Compliance Review     ← ALWAYS FIRST (spec-compliance-reviewer.md)
Stage 2: Code Quality Review        ← ONLY after Stage 1 passes (code-quality-reviewer.md)
```

**Never skip Stage 1.** Code that doesn't meet the spec doesn't benefit from quality review.

**Worktree hygiene:** All implementer subagents MUST work in a worktree. Never dispatch an implementer to the main working tree. See Git Worktrees section below.

### Stage 1: Spec Compliance Review

**Question:** Does the implementation do what the spec/requirements asked?

Use `.github/agents/spec-compliance-reviewer.md`. Provide:
- Full requirements / acceptance criteria for the todo
- Full diff or file contents of the implementation

If Stage 1 returns GAPS: implementer fixes gaps. Re-run Stage 1 before proceeding.

### Stage 2: Code Quality Review

**Question:** Is the implementation clean, maintainable, and correct?

Use `.github/agents/code-quality-reviewer.md` — 1 agent per file changed.

If Stage 2 returns REQUEST CHANGES: implementer fixes. Re-run Stage 2 before proceeding.

---

## Git Worktrees for Parallel Work

When a subagent needs to **modify files** (not just read), give it a worktree:

```bash
# Create worktree
git worktree add .worktrees/agent-task-name -b feat/agent-task-name

# Verify .worktrees is gitignored (BEFORE creating)
git check-ignore -q .worktrees || echo "ADD .worktrees TO .gitignore FIRST"

# Pass this path as working directory in the subagent prompt

# After subagent completes — review its diff
git -C .worktrees/agent-task-name diff main

# Merge and clean up
git worktree remove .worktrees/agent-task-name
```

**Directory selection priority:** `.worktrees/` → `worktrees/` → ask user
**Safety gate:** Run `git check-ignore -q .worktrees` before creating. If not ignored, add to `.gitignore` immediately — do not skip.

---

## Model Selection

Match model tier to task complexity. Instructions must be written for GPT-4.1 baseline regardless of selected tier.

| Task type | Default tier |
|-----------|-------------|
| Mechanical: grep, rename, format, one-function change | Standard |
| Research: read files, summarize patterns, compare approaches | Standard |
| Implementation: multi-file, design judgment | Standard |
| Review: spec compliance, code quality, architecture | Standard |
| Architecture design, security, final review | Premium |

**Using Premium for non-architecture tasks:** State the reasoning before dispatching. Example: "Dispatching Premium for this review because the change touches 3 layer boundaries." Do not dispatch Premium silently for mechanical work.

**Concurrency:** Copilot Enterprise accounts have no practical agent concurrency limit. Dispatch as many parallel agents as the task warrants. Standard accounts: verify your limit before parallelizing.

**For parallel read-only research:** Use `dispatching-parallel-agents` skill.

---

## Delegation Quality Rules

- **One clear objective per subagent** — no multi-part briefs
- **State the return format explicitly** — tell it exactly what to give back
- **Provide complete context** — subagents are stateless
- **Fresh context per task** — never share session history; it contaminates the subagent's search
- **Accept findings unless they conflict with evidence you verified yourself**
- **If a subagent finds something unexpected:** treat it as a hypothesis; verify before acting

---

## Anti-Patterns

| Anti-pattern | Why it fails |
|---|---|
| "I'll check this myself" (for 5+ files) | Fills context, biased by assumptions |
| Skipping Stage 1 review because "it looks right" | Spec gaps ship; quality review doesn't catch them |
| One agent reviewing multiple large files | Coverage is shallow; 1 per file is the rule |
| Acting on subagent findings without verifying | Subagents can be wrong — findings are hypotheses |
| Dispatching without a clear return format | Agent returns noise |
| Sharing full session history as context | Contaminates search; subagent inherits your assumptions |
| Reporting DONE before 2-stage review | Code exists; correctness unverified |

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "The subagent's description sounds right, I'll skip review" | Descriptions are summaries — they omit bugs. YOU MUST read the actual diff and dispatch the 2-stage review every time. |
| "This is just docs, no code review needed" | Documentation errors ship as silently as code bugs. Stage 1 spec compliance applies to every todo without exception. |
| "I verified one file, the rest are probably fine" | Each file requires its own code-quality reviewer. One agent per file is the rule — no extrapolation across files. |
| "The subagent said PASS, that's good enough" | A subagent's self-assessment is not a review. PASS from an implementer means dispatch Stage 1 — not skip it. |
| "I'll do a quick scan instead of dispatching a code-review agent" | A quick scan inherits your assumptions. A dispatched code-review agent does not. Dispatch the agent. |
| "I'll add the worktree after dispatching" | Worktrees MUST exist before dispatch. The agent needs the worktree path in its prompt — it cannot create its own isolation after the fact. |
| "I'll include the rules in the prompt instead of using a template" | Injected rules drift between sessions. Pre-built templates in `.github/agents/` are the single source of truth. Use them. |

---

## Quick Reference

```
Task to delegate
    |
    +-- Read-only research? → dispatching-parallel-agents skill
    |
    +-- Needs file changes?
         |
         v
    Create worktree (ALWAYS — never dispatch to main working tree)
         |
         v
    Dispatch implementer (implementer.md)
         |
         v
    Status code: DONE / DONE_WITH_CONCERNS / PARTIAL / NEEDS_CONTEXT / BLOCKED
         |
         +-- NEEDS_CONTEXT → provide info, re-dispatch
         +-- BLOCKED → assess, escalate
         +-- PARTIAL → verify completed, create todos for remaining, proceed for completed
         +-- DONE_WITH_CONCERNS → read concerns, proceed if no correctness risk
         +-- DONE
              |
              v
    Stage 1: spec-compliance-reviewer.md → GAPS? → implementer fixes → re-run Stage 1
              |
              v
    Stage 2: code-quality-reviewer.md (1 per file) → REQUEST CHANGES? → implementer fixes → re-run Stage 2
              |
              v
    Mark todo done. Reload skills (session-bootstrap refresh rule).
    Pick up next todo.
              |
              v
    (After all todos) → final code review → finishing-a-development-branch
```
