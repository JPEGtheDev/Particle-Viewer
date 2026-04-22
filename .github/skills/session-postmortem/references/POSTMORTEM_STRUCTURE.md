# Postmortem Report Format

Canonical template and verdict definitions for session postmortem reports.

---

## Report Template

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
[Insert the Iron Law Compliance Check table from session-postmortem SKILL.md]

### Verdict: HEALTHY / NEEDS IMPROVEMENT / SYSTEMIC ISSUE
```

---

## Verdict Definitions

- **HEALTHY** — iron laws followed, no repeated failure modes, skills loaded proactively
- **NEEDS IMPROVEMENT** — one or two isolated lapses; skill updates would prevent recurrence
- **SYSTEMIC ISSUE** — same failure mode appeared multiple times, or the agent bypassed an iron law and delivered a result anyway
