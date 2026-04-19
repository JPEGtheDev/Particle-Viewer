---
name: session-postmortem
description: Use after a session completes.
---

## Iron Law

```
ANALYZE BEHAVIOR, NOT JUST OUTCOMES — FIND WHERE THE MODEL RATIONALIZED
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST examine the session for rationalization patterns. A clean outcome does not prove a clean process. No exceptions.

**Announce at start:** "I am running the session-postmortem skill to analyze [session description]."

---

## The Blameless Principle

This postmortem is about **decisions and systems**, not character. Every failure is a decision that could be better-gated or better-informed. The language in the report must reflect this.

| Blameful (forbidden) | Blameless (required) |
|----------------------|---------------------|
| "The agent was careless" | "The planning gate was not applied before code was written" |
| "The agent didn't follow the rules" | "The rationalization table did not cover this excuse pattern" |
| "The agent got lucky" | "The process produced a correct outcome by path rather than by gate" |
| "The agent lied" | "The confidence vocabulary gate was not active for this claim type" |

The goal is not to evaluate the agent — it is to find the gaps in the system that allowed the failure to occur undetected. A gap found is a skill improvement waiting to be written.

---

## Difference From Self-Evaluation

| Skill | Who runs it | When | What it analyzes |
|-------|-------------|------|-----------------|
| `self-evaluation` | The agent itself | At the end of its own session | Outcomes, lessons, skill updates |
| `session-postmortem` | A different agent or human reviewer | After a session closes | Agent *behavior* patterns during the session |

Self-evaluation captures what was learned. Session postmortem finds where the agent rationalized, bypassed laws, or got lucky.

---

## The 5-Part Blameless Postmortem Structure

Work through every part. Do not skip a section because the session "seemed fine."

### Part 1: Timeline

Reconstruct the session in chronological order. Focus on **decision points** — moments where the agent chose a path. Each entry: what happened, what was the agent's stated reason, what was the actual situation.

```
HH:MM - [Action taken] — [Stated reason if given] — [Observation]
```

Do not editorialize in the timeline. Record events.

### Part 2: Root Cause

For each failure or near-miss in the timeline, identify the **underlying cause**. Not the proximate cause ("the agent wrote code without a test") but the root cause ("no gate in the session-start checklist enforced TDD before the first `edit` call in this task type").

Use "5 Whys" for each failure:
```
Why did X happen?
  Because Y
Why did Y happen?
  Because Z
...until you reach a system/gate gap, not a character judgment
```

### Part 3: Contributing Factors

What conditions made the root cause more likely? These are not root causes — they are the environment that allowed the root cause to surface.

Examples:
- Ambiguous requirements that weren't clarified at session start
- Missing rationalization table entry for a specific excuse pattern
- Skill that was required but not in the auto-load table
- Context pressure (long session, many files in flight)

### Part 4: What Went Well

Name the things that worked — gates that fired correctly, skills that were loaded proactively, subagents that were dispatched when required. This is not a consolation section — it identifies which gates are reliable and should be preserved.

### Part 4b: Where We Got Lucky

This section is separate from "What Went Well" and is the most commonly omitted part of a postmortem.

"Lucky" means: the correct outcome was produced, but not because of a reliable process. The difference:

| What Went Well | Where We Got Lucky |
|---|---|
| Gate fired → outcome correct | Gate did not fire → outcome happened to be correct anyway |
| Plan was followed → no rework | Plan wasn't followed → user didn't notice |
| Test caught the regression | Test didn't exist → no regression this time |

"Got lucky" items are highest-priority action items. A system that produces correct outcomes by luck is not reliable — the next session the luck may not hold. Name each lucky path and identify the gate that would have caught it if the luck had run out.

### Part 5: Action Items

For every root cause and contributing factor, one concrete action item. Each must be:
- **Specific** — names the exact skill file and section to update
- **Triggerable** — describes exactly what should happen differently
- **Owned** — states which skill or gate owns the change

**Dan Milstein Classification (apply to every action item):**

| Strength | Type | Why |
|----------|------|-----|
| ✅ STRONG | Gate/system change — a structural change that makes the default behavior correct | Changes the system; no willpower required |
| ✅ STRONG | Rationalization table addition — explicitly names and counters the excuse used | Counters the specific thought pattern before it fires |
| ✅ STRONG | Red Flag → STOP addition — a new trigger phrase added to the checklist | Mechanical: fires on recognition, not intent |
| ❌ WEAK | Behavior-change item — "be more careful about X", "remember to check Y" | Requires willpower on every occurrence; degrades under pressure |
| ❌ WEAK | Vague skill update — "improve the planning section" | Cannot be applied mechanically; produces inconsistent results |

**Required:** Every action item must be STRONG classification. If you can only produce a WEAK item, ask: "What structural change would make this mistake impossible to make, rather than just less likely?"

| # | Root Cause / Factor | Action Item | Strength | Target File |
|---|---------------------|-------------|----------|-------------|
| 1 | [Root cause] | Add "[specific text]" to [section] | ✅/❌ | [skill file] |

No vague items. "Be more careful" is not an action item. "Add the phrase 'X' to the rationalization table in execution Phase 2 under excuse 'Y'" is.

---

## Iron Law Compliance Check

Run every item before generating the report:

| Law | Followed | Evidence | Notes |
|-----|----------|----------|-------|
| TDD (no prod code without test) | ✅/❌/N/A | [what proves this] | |
| Verification before completion | ✅/❌/N/A | [what proves this] | |
| Root cause before fixes | ✅/❌/N/A | [what proves this] | |
| Conventional commits | ✅/❌/N/A | [what proves this] | |
| Skills loaded before acting | ✅/❌/N/A | [what proves this] | |
| Commitments kept or acknowledged | ✅/❌/N/A | [what proves this] | |

✓ All followed → note compliance in report
✗ Any violated → verdict is SYSTEMIC ISSUE for the violated law — document in action items

---

## Postmortem Report Format

```markdown
## Session Postmortem: [session-title]

### Summary
[1-2 sentence description of what the session accomplished]

### Timeline
[Chronological decision points]

### Root Cause Analysis
[5-Whys for each failure/near-miss]

### Contributing Factors
[Conditions that made failures more likely]

### What Went Well
[Gates that fired correctly, proactive behaviors]

### Action Items
| # | Root Cause / Factor | Action Item | Target File |
|---|---------------------|-------------|-------------|

### Iron Law Compliance
[Table from above]

### Verdict: HEALTHY / NEEDS IMPROVEMENT / SYSTEMIC ISSUE
```

Verdict definitions:
- **HEALTHY** — iron laws followed, no repeated failure modes, skills loaded proactively
- **NEEDS IMPROVEMENT** — one or two isolated lapses; skill updates would prevent recurrence
- **SYSTEMIC ISSUE** — same failure mode appeared multiple times, or the agent bypassed an iron law and delivered a result anyway

---

## Red Flags — Signs the Session Went Wrong

If any of the following apply, the verdict is at minimum NEEDS IMPROVEMENT:

- Agent expressed confidence before running verification commands
- Agent proposed or applied fixes before establishing root cause
- Agent claimed a task complete without running the pre-commit gate
- Agent loaded skills after acting in their domain (retroactive skill loading)
- Agent kept research and exploration in the main context instead of dispatching subagents
- User had to correct the same behavior more than once in the same session
- Agent dropped an announced commitment without acknowledging it

Three or more of the above = SYSTEMIC ISSUE. Relevant skills need immediate rationalization table updates.

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "It was mostly fine, no need for a postmortem" | "Mostly fine" hides the one failure that will recur. Systematic review is required. |
| "It was a one-off mistake, not a pattern" | Check the full session before concluding that. One-off mistakes look different from patterns only after you check. |
| "The outcome was good, so the process must have been right" | Good outcome from bad process = lucky, not reliable. The blameless principle exists to find the lucky path. |
| "The user didn't complain" | Some failures are silent. The agent may have produced a plausible wrong answer the user accepted. Postmortem finds those. |
| "Self-evaluation already covered this" | Self-eval is by the agent. Postmortem is external. Different perspective, different blind spots. Both are necessary. |
| "The session was short, not worth analyzing" | Short sessions have failure modes too. A short session gets a proportionate postmortem — 10 minutes, not an hour. |

---

## Related Skills

- `self-evaluation` — agent-run end-of-session review; complements but does not replace this skill
- `execution` — iron laws this skill checks for compliance
- `verification-before-completion` — the gate this skill checks was applied correctly
- `skill-reviewer` — if skill gaps are found, dispatch skill-reviewer to verify the updated skill is complete
