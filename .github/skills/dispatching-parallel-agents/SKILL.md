---
name: dispatching-parallel-agents
description: Use when multiple independent read-only research tasks can run simultaneously, or when you need to fan out investigation across many files or hypotheses.
---

## Iron Law

```
PARALLEL AGENTS ARE FOR READ-ONLY WORK ONLY.
FILE MODIFICATIONS REQUIRE WORKTREES — ONE WORKTREE PER AGENT.
```

Violating the letter of this rule is violating the spirit of this rule.

**Announce at start:** "I am using the dispatching-parallel-agents skill to [describe what is being parallelized]."

---

## When to Parallelize

Dispatch agents in parallel when ALL of the following are true:

1. Tasks are **independent** — no agent needs the output of another to start
2. Tasks are **read-only** OR each write agent has its own isolated worktree
3. Each task has a **single clear objective** — multi-part briefs produce noise

**Good candidates for parallelization:**
- Scanning multiple files for a pattern
- Confirming/denying multiple independent hypotheses simultaneously
- Researching 3+ unrelated topics at once
- Running multiple spec-compliance or code-quality reviews simultaneously

**Do NOT parallelize:**
- Tasks where Agent B needs Agent A's output
- Multiple write agents on the same files (race condition)
- Tasks requiring shared state or coordinated decisions

---

## Concurrency Rules

**Copilot Enterprise:** No practical concurrency limit. Dispatch as many as the task warrants.

**Standard accounts:** Verify your account's agent concurrency limit before dispatching. Default assumption: up to 4 concurrent unless you have confirmed a higher limit.

State your concurrency assumption before dispatching: "Dispatching N agents in parallel — [Enterprise / confirmed N-agent limit / assuming default]."

---

## Isolation Rules

Each parallel agent MUST receive:

1. **Isolated context** — never share your session history. Provide only what that agent needs.
2. **A single objective** — one question, one file set, one hypothesis.
3. **An explicit return format** — tell it exactly what to give back.
4. **No knowledge of other agents** — agents do not know about each other.

**Why isolation matters:** Agents that share context inherit your biases and assumptions. Isolated agents search without your anchoring.

---

## Aggregating Results

When parallel agents return:

1. Read all results before acting on any
2. Treat every finding as a **hypothesis** — not a fact
3. Cross-check conflicting findings against the source files yourself
4. State what each agent found and what you verified:
   ```
   Agent 1 found: [X] — verified by reading [file:line]
   Agent 2 found: [Y] — conflicts with Agent 1 — investigating
   ```
5. Do NOT propagate a finding to the user until you have verified it

---

## Write Agents — Worktree Requirement

When parallelizing write work (e.g., multiple implementers working on independent subsystems):

1. Create one worktree per agent BEFORE dispatching
2. Verify each worktree is gitignored: `git check-ignore -q .worktrees || echo "NOT IGNORED"`
3. Pass worktree path as the agent's working directory
4. After agents complete: review each diff independently before merging

```bash
# Create isolated worktrees
git worktree add .worktrees/agent-feature-a -b feat/agent-feature-a
git worktree add .worktrees/agent-feature-b -b feat/agent-feature-b

# Review diffs after completion
git -C .worktrees/agent-feature-a diff main
git -C .worktrees/agent-feature-b diff main
```

See `using-git-worktrees` skill for full worktree lifecycle.

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "These tasks are mostly independent" | Mostly = risk. Confirm independence before dispatching. |
| "I'll just have them both edit the file" | Race condition. One agent's changes will be overwritten. Use worktrees. |
| "I'll share my session context — it's helpful" | It contaminates their search. They'll confirm your assumptions instead of testing them. |
| "One agent can do all this" | Sequential agents fill your context. Parallel agents preserve it. |
| "I don't need to verify — the agent found it" | Findings are hypotheses. You verify before you propagate. |
