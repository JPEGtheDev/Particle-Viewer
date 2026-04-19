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

## External Reviewer Protocol (Required)

A self-assessment cannot find what the agent rationalized away. **An external reviewer reads the raw event log cold — no session memory, no emotional investment in the outcome.** This is not optional. A postmortem with only self-assessment is incomplete.

### Setup

Derive the paths for the session being reviewed:

```
SESSION_ID:       [session ID — shown in workspace.yaml or .copilot/session-state/ directory name]
EVENTS_LOG:       ~/.copilot/session-state/[SESSION_ID]/events.jsonl
WORKSPACE:        ~/.copilot/session-state/[SESSION_ID]/
SELF_ASSESSMENT:  ~/.copilot/session-state/[SESSION_ID]/postmortem.md  (if self-eval already ran)
OUTPUT:           ~/.copilot/session-state/[SESSION_ID]/postmortem-external.md
```

### Protocol

1. **Dispatch the external reviewer subagent** using `.github/agents/postmortem-reviewer.md` with the above paths filled in. The reviewer reads `events.jsonl` directly — no pre-export needed.

2. **Run self-assessment in parallel** (the main agent does Parts 1–5 from memory while the reviewer works).

3. **Wait for the reviewer to complete.** Read its output from `postmortem-external.md`.

4. **Reconcile.** Where the external reviewer and self-assessment agree: note the convergence. Where they conflict: the external reviewer's log-cited findings are authoritative. The self-assessment's uncited claims are not.

5. **Merge into the final report.** The final postmortem includes both perspectives. Discrepancies between self-assessment and external review are explicitly noted — they are themselves a finding.

**Precedence rule:** External reviewer findings backed by log evidence override self-assessment claims backed only by memory. A gate "remembered" as having fired but absent from the log did not fire.

---

## Batch Analysis Protocol

To run postmortems on older sessions that were never reviewed:

```bash
# List all sessions, find those without an external postmortem
for dir in ~/.copilot/session-state/*/; do
  session_id=$(basename "$dir")
  if [ ! -f "$dir/postmortem-external.md" ] && [ -f "$dir/events.jsonl" ]; then
    echo "$session_id"
  fi
done
```

Dispatch up to 4 reviewer subagents in parallel, each pointed at a different `events.jsonl`. Use `postmortem-reviewer.md` for each. Collect results as they complete.

**Priority order for batch review:**
1. Sessions where a self-assessment `postmortem.md` exists but no external review — highest value, direct comparison available
2. Sessions with a `plan.md` but no postmortem at all — likely had structured work with no retrospective
3. Sessions with only checkpoint files — lower priority, less context available

**What to do with batch findings:** If a pattern appears in 2 or more sessions (same gate skipped, same rationalization, same lucky path), that pattern becomes a STRONG action item in the next skill update. Single-session findings are noted but not acted on until confirmed.

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

### Part 4c: User Prompt Quality Review

This section is not optional and is not softened to avoid conflict. Poor prompts create conditions that make agent failure more likely — finding those patterns serves both parties.

**The blameless principle applies here too.** Prompt quality review is about prompt patterns, not the user's judgment or intelligence. The same person who writes excellent prompts on Tuesday writes ambiguous ones on Friday under deadline pressure. The goal is to name the pattern so future sessions don't repeat it.

**What to evaluate:**

| Pattern | What it looks like | Why it matters |
|---|---|---|
| Multi-part prompt with no priority order | A single message with 8–12 separate instructions | Agent must guess what matters most; important items get buried or dropped |
| Approval mid-execution | "yes, proceed" or "I was adding commentary" while the agent was already acting | Creates ambiguous ownership of the direction — what was approved, what wasn't? |
| Scope addition after implementation starts | "also add..." after a plan has been confirmed | Forces replanning mid-execution; earlier work may be invalidated |
| Implicit requirements treated as obvious | Requirements not stated because "it's obvious" | Agent optimizes for the stated goal, not the unstated one |
| Contradictory instructions in the same message | Two rules that cannot both be followed | Agent picks one silently; user is surprised by the choice |
| "Just do it" for a task needing design review | "just implement X directly" for a multi-subsystem change | Bypasses brainstorming gate by framing the task as simple |
| Moving goalposts without acknowledgment | Changing requirements mid-session without noting what changed | Agent may be building toward an invalidated target without knowing it |

**What the review should produce:**

For each pattern found, cite the specific message or exchange where it occurred. Name what the effect was (lost work, wrong direction, wasted subagent runs). Give one concrete recommendation for how that type of prompt could be written differently next time.

Do not soften findings. A prompt that cost three subagent runs and a revert deserves to be called that, with the example quoted.

**Format:**

```
#### Prompt Feedback

**Pattern found:** [name from table above]
**Example (quoted or paraphrased):** "[the prompt or relevant portion]"
**Effect:** [what it caused — misdirection, wasted agents, ambiguity not caught until late]
**Recommendation:** [one specific, actionable change for future prompts of this type]
```

If no patterns are found after genuine review, state that explicitly with evidence. Do not omit this section — an empty section with "no issues found" and supporting evidence is the correct output when prompts were clean.

---

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

### Where We Got Lucky
[Correct outcomes produced without reliable process — highest-priority action items]

### External Reviewer Findings
[Summary of external reviewer's findings — log-cited evidence only]
[Discrepancies with self-assessment explicitly noted]
[If external review not yet complete: "PENDING — external reviewer dispatched"]

### Prompt Feedback
[User prompt quality review — patterns found, quoted examples, effects, recommendations]
[If no issues: state explicitly with supporting evidence]

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
- Prompt Feedback section was omitted or left as a placeholder
- **External reviewer was not dispatched — postmortem is self-assessment only**

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
| "I'll skip the external reviewer, it's just overhead" | The external reviewer reads events the agent rationalized away. Skipping it means the postmortem finds only what the agent was willing to find. |

---

## Related Skills

- `self-evaluation` — agent-run end-of-session review; complements but does not replace this skill
- `execution` — iron laws this skill checks for compliance
- `verification-before-completion` — the gate this skill checks was applied correctly
- `skill-reviewer` — if skill gaps are found, dispatch skill-reviewer to verify the updated skill is complete
