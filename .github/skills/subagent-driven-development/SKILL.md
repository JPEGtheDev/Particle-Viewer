---
name: subagent-driven-development
description: Use when delegating implementation tasks, confirming theories, running parallel research, or reviewing completed work. Governs subagent dispatch, 2-stage review, status codes, and the empirical evidence mandate.
---

## Iron Law

```
DISPATCH BEFORE GUESSING — SUBAGENTS ARE CHEAP, WRONG ASSUMPTIONS ARE EXPENSIVE
```

**Announce at start:** "I am using the subagent-driven-development skill to [dispatch/review/confirm] [brief description]."

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

### Red Flags → STOP and Dispatch

These thoughts mean stop immediately:

| Thought | Required action |
|---------|----------------|
| "I think the issue is..." | Dispatch explore agent → read the actual code |
| "This should work because..." | Run it. Read the output. |
| "I'm confident that..." | State the evidence, or dispatch to get it |
| "It probably passes..." | Run the test suite |
| "I remember that..." | Memory is always unverified — dispatch |
| "Based on how it usually works..." | Dispatch to confirm the actual behavior |

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

Every subagent doing implementation work must report one of these four codes:

| Code | Meaning | Your response |
|------|---------|---------------|
| `DONE` | Task complete, all verification passed | Accept; run requesting-code-review |
| `DONE_WITH_CONCERNS` | Complete but flagged issues for review | Accept; address concerns before merging |
| `NEEDS_CONTEXT` | Blocked by missing information | Provide context; re-dispatch |
| `BLOCKED` | Cannot proceed due to dependency or environment | Resolve blocker; re-dispatch or reassign |

Require this code in every implementer prompt. Do not accept a response that doesn't include one.

---

## 2-Stage Review Protocol

Every completed implementation task requires two reviews in this order:

```
Stage 1: Spec Compliance Review     ← ALWAYS FIRST
Stage 2: Code Quality Review        ← ONLY after Stage 1 passes
```

**Never skip Stage 1.** Code that doesn't meet the spec doesn't benefit from quality review.

### Stage 1: Spec Compliance Review

**Question:** Does the implementation do what the spec/requirements asked?

Dispatch a general-purpose agent with this prompt:

```
You are a spec compliance reviewer. Your only job is to verify that the
implementation matches the requirements — not to evaluate code quality.

Requirements/acceptance criteria:
[PASTE FULL REQUIREMENTS]

Implementation to review (file paths + diffs):
[PASTE DIFF OR FILE CONTENTS]

Answer only these questions:
1. Which requirements are fully addressed?
2. Which requirements are partially addressed? (describe the gap)
3. Which requirements are missing entirely?
4. Are there any behaviors present that contradict the requirements?

Return a structured report. Do not comment on code style, naming, or quality.
```

If Stage 1 returns any gaps: fix them before proceeding to Stage 2.

### Stage 2: Code Quality Review

**Question:** Is the implementation clean, maintainable, and correct?

Dispatch a code-review agent (1 per file) — see requesting-code-review skill.

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

Match model to task complexity:

| Task type | Model choice |
|-----------|-------------|
| Mechanical (grep, format, rename) | Cheapest capable model |
| Research (reading, summarizing) | Cheap model |
| Complex reasoning, architecture | Standard model |
| Critical review, security, design | Premium model |

Never dispatch a premium model for mechanical work. Reserve capacity for judgment-intensive tasks.

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

## Quick Reference

```
Task to delegate
    ↓
Read-only? → explore agent (parallel up to 4)
    ↓
Needs file changes? → general-purpose + worktree
    ↓
Implementation completes → status code required (DONE/DONE_WITH_CONCERNS/NEEDS_CONTEXT/BLOCKED)
    ↓
DONE or DONE_WITH_CONCERNS?
    ↓
Stage 1: Spec Compliance Review → gaps found? → fix → re-review
    ↓
Stage 2: Code Quality Review (1 agent per file)
    ↓
All reviews clean → invoke requesting-code-review skill
```
