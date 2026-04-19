---
name: using-git-worktrees
description: Use when running parallel agent work, testing an approach in isolation, or keeping the main branch clean while a subagent operates on a separate branch.
---

## Iron Law

```
NEVER LET A SUBAGENT COMMIT DIRECTLY TO THE MAIN WORKING BRANCH
```

Violating the letter of this rule is violating the spirit of this rule.

Every subagent gets its own worktree. The main context reviews and merges.

**Announce at start:** "I am using the using-git-worktrees skill to create a worktree for [purpose]."

---

## Why Worktrees

Git worktrees let multiple agents work on different branches simultaneously **without branch switching**. Each agent gets:
- Its own working directory
- Its own branch
- Full access to the git history

Benefits:
- Main context stays clean — no dirty working tree from subagent experiments
- Parallel agent execution without conflicts
- Easy to discard failed experiments: delete the worktree
- Empirical A/B testing: two agents, two worktrees, compare results

---

## Worktree Lifecycle

### Create

```bash
# Create worktree on a new branch
git worktree add ../Particle-Viewer-agent-<name> -b agent/<name>

# Create worktree tracking an existing branch
git worktree add ../Particle-Viewer-agent-<name> <branch-name>
```

Use `../Particle-Viewer-agent-<name>` as the path — one level up from the repo root, named descriptively.

### List active worktrees

```bash
git worktree list
```

### Remove when done

```bash
git worktree remove ../Particle-Viewer-agent-<name>
git branch -d agent/<name>   # only after merging or discarding
```

---

## BEFORE Creating a Worktree Gate

```
BEFORE creating a worktree, verify:
1. The task is independent enough for a subagent (not tightly coupled to in-progress main work)
2. A descriptive name for the branch exists: agent/<purpose>
3. The subagent prompt includes the worktree path explicitly

✓ All met → create the worktree and dispatch
✗ Any unmet → work in the main context instead
```

---

## Subagent Dispatch Pattern

When dispatching a subagent to a worktree:

1. Create the worktree first (main context)
2. Pass the worktree path to the subagent as its working directory
3. Subagent commits to its branch inside the worktree
4. Main context reviews the diff: `git diff main..agent/<name>`
5. If approved: merge or cherry-pick into main
6. If rejected: `git worktree remove` — no cleanup needed in main

**The subagent MUST NOT push to `main`, `master`, or the active development branch.**

---

## A/B Testing with Worktrees

When "I think" is not enough and empirical evidence is needed:

```bash
# Approach A
git worktree add ../Particle-Viewer-approach-a -b agent/approach-a

# Approach B
git worktree add ../Particle-Viewer-approach-b -b agent/approach-b

# Dispatch two agents — one per worktree — with identical test harness
# Compare: test results, line count, coupling, readability
# Adopt the winner; discard the loser's worktree
```

This pattern replaces "I think approach A is better" with measurable output.

---

## Red Flags — STOP

- Subagent working directly in the main repo directory
- Subagent output committed to `main` or the active feature branch without review
- Worktree left alive after the work is merged or discarded (leaks branch clutter)
- Dispatch to a worktree without passing the worktree path in the agent prompt

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "The task is simple enough to do in main" | Simple tasks don't need a worktree — this rule applies when you WOULD use a subagent |
| "I'll review the subagent output before merging" | Review happens in the main context; the subagent STILL needs its own worktree to work safely |
| "Worktrees add overhead" | One git command. The cleanup time saved from a subagent polluting main more than compensates |
| "I think this approach is right, no need for A/B" | "I think" is not evidence. Dispatch two agents and let the output decide. |
