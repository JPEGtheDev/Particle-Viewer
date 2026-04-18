---
name: session-postmortem
description: Retrospective analysis of a copilot chat session. Used after a session completes to analyze agent behavior patterns, identify where iron laws were bypassed, and extract durable lessons. Different from self-evaluation — this analyzes BEHAVIOR, not just outcomes.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.0"
  category: review
  project: Particle-Viewer
---

## Iron Law

```
ANALYZE BEHAVIOR, NOT JUST OUTCOMES — FIND WHERE THE MODEL RATIONALIZED
```

YOU MUST examine the session for rationalization patterns. A clean outcome does not prove a clean process. No exceptions.

**Announce at start:** "I am running the session-postmortem skill to analyze [session description]."

---

## Difference From Self-Evaluation

These two skills are distinct and NOT interchangeable:

| Skill | Who runs it | When | What it analyzes |
|-------|-------------|------|-----------------|
| `self-evaluation` | The agent itself | At the end of its own session | Outcomes, lessons, skill updates |
| `session-postmortem` | A different agent or human reviewer | After a session closes | Agent *behavior* patterns during the session |

Self-evaluation captures what was learned. Session postmortem finds where the agent rationalized, bypassed laws, or got lucky.

---

## What to Analyze

Work through each question in sequence. Do not skip a question because the session "seemed fine."

1. **Iron Law compliance** — was TDD followed? Were verification commands (`cmake --build build`, `./build/tests/ParticleViewerTests`) actually run before claiming completion? Were root causes traced before fixes were applied?
2. **Rationalization patterns** — did the agent express confidence without running verification? Make factual claims before executing commands? Bypass skill dispatch by acting in a domain without reading the relevant skill first?
3. **Subagent usage** — were tasks delegated when the `execution` skill required delegation? Did the agent fill its own context with exploration work that should have gone to a subagent?
4. **Context hygiene** — was the main context window used efficiently, or was it consumed by research, file reading, and exploration that blocked forward progress?
5. **Skill loading** — were required skills loaded *before* acting in their domain, per the auto-load table in `copilot-instructions.md`? Or loaded after the fact, retroactively?
6. **Correction moments** — when the user corrected the agent, what was the specific failure mode? Is that failure mode already present in the relevant skill's rationalization table? If not, it should be added.

---

## Postmortem Report Format

```markdown
## Session Postmortem: [session-title]

### Summary
[1-2 sentence description of what the session accomplished]

### Iron Law Compliance
| Law | Followed | Evidence | Notes |
|-----|----------|----------|-------|
| TDD (no prod code without test) | ✅/❌/N/A | ... | ... |
| Verification before completion | ✅/❌/N/A | ... | ... |
| Root cause before fixes | ✅/❌/N/A | ... | ... |
| Conventional commits | ✅/❌/N/A | ... | ... |

### Behavioral Patterns Observed
[List patterns observed: rationalization, premature completion claims, guessing without verification, etc.]

### Failure Modes
| Moment | What happened | What should have happened |
|--------|---------------|--------------------------|
| ... | ... | ... |

### Skill Gaps Found
[Excuses or behaviors NOT yet present in any rationalization table — these need to be added to a skill]

### Recommended Skill Updates
[Specific text to add to specific skills, with skill name and section]

### Verdict: HEALTHY / NEEDS IMPROVEMENT / SYSTEMIC ISSUE
```

Verdict definitions:
- **HEALTHY** — iron laws followed, no repeated failure modes, skills loaded proactively
- **NEEDS IMPROVEMENT** — one or two isolated lapses, not repeated, skill updates would prevent recurrence
- **SYSTEMIC ISSUE** — same failure mode appeared multiple times, or the agent bypassed an iron law and delivered a result anyway

---

## Red Flags — Signs the Session Went Wrong

If any of the following apply, the verdict is at minimum NEEDS IMPROVEMENT:

- Agent expressed confidence before running verification commands
- Agent proposed or applied fixes before establishing root cause
- Agent claimed a task complete without running the pre-commit gate (`clang-format`, build, tests)
- Agent loaded skills after acting in their domain (retroactive skill loading is compliance theater)
- Agent kept research, file reading, and exploration in the main context instead of dispatching subagents
- User had to correct the same behavior more than once in the same session

If three or more of the above apply, the verdict is SYSTEMIC ISSUE and the relevant skills need immediate rationalization table updates.

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "It was mostly fine, no need for a postmortem" | Systematic improvement requires systematic review. "Mostly fine" hides the one failure that will recur. |
| "It was a one-off mistake, not a pattern" | Check the full session before concluding that. One-off mistakes look different from patterns only after you check. |
| "The outcome was good, so the process must have been right" | Good outcome from bad process = lucky, not reliable. Process failures compound. |
| "The user didn't complain" | Some failures are silent. The agent may have produced a plausible wrong answer that the user accepted. Postmortem finds those. |
| "Self-evaluation already covered this" | Self-eval is by the agent. Postmortem is external. Different perspective, different blind spots. Both are necessary. |
| "The session was short, not worth analyzing" | Short sessions have failure modes too. The analysis is proportionate — a short session gets a short postmortem. |

---

## Related Skills

- `self-evaluation` — agent-run end-of-session review; complements but does not replace this skill
- `execution` — iron laws this skill checks for compliance
- `verification-before-completion` — the gate this skill checks was applied correctly
- `skill-reviewer` — if skill gaps are found, dispatch skill-reviewer to verify the updated skill is complete
