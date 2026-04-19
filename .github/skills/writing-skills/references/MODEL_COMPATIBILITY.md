# Model Compatibility Reference

All skills in this system are written for **GPT-4.1 as the baseline**. This document identifies patterns that are most likely to be skipped or misapplied by lower-capability models, and the mitigation already built into each skill.

## Manual Testing Required

This document is a guide for manual evaluation. To verify a skill works with a specific model:
1. Load the skill in isolation (no other context)
2. Give the model a task that requires the skill's iron law
3. Observe: does the model follow the gate, or rationalize past it?
4. Specifically test each row in the Rationalization Prevention table

## Patterns Most Likely to Fail on Lower-End Models

### 1. Multi-step gates with conditional branches
**Skill:** `brainstorming` (Phase 1 Ambiguity Block), `systematic-debugging` (4-phase protocol)
**Risk:** Model skips to "obvious" answer without completing all phases
**Mitigation:** Announce + numbered steps + STOP language. Verify model actually waits for clarification responses.

### 2. "After every todo" rules
**Skill:** `subagent-driven-development` (2-stage review), `execution` (work loop step 6)
**Risk:** Model completes todos and skips reviewer dispatch — moves to next item silently
**Mitigation:** Iron Law 10 in copilot-instructions.md + explicit STOP in work loop. Test by observing whether model dispatches reviewers without being reminded.

### 3. Announcement declarations
**Skill:** All skills require "I am using the [skill] to [purpose]"
**Risk:** Model skips announcement silently, treating it as optional
**Mitigation:** Frame as commitment mechanism in writing-skills. Test by checking whether model announces before acting.

### 4. Honesty / vocabulary bans
**Skill:** `honesty`, `verification-before-completion`
**Risk:** Model uses "should work", "probably", "I'm confident" without catching itself
**Mitigation:** Banned vocabulary in copilot-instructions Iron Law 6. Test by watching for banned phrases in responses.

### 5. Skill refresh rule
**Skill:** `session-bootstrap`
**Risk:** Model never reloads a skill after 3 prompts or on new todo — relies on stale context
**Mitigation:** Session-bootstrap explicitly states reload triggers. Test across a 5-prompt session.

## Writing Skills for GPT-4.1 Compliance

When authoring or revising skills:

- **Front-load the iron law** — first 10 lines of content after frontmatter
- **Use imperative verbs** — "Stop. Load the skill now." not "You might want to consider loading the skill"
- **Flatten conditional logic** — "If X: do Y. Stop." on separate lines, not nested `if/then/else`
- **Make STOP conditions explicit** — "If this condition is not met: STOP. Do not proceed."
- **No implication** — never rely on the model inferring what "complete" means; define it as a numbered checklist

## Evaluation Signals (What to Watch For)

| Behavior | What it signals |
|----------|----------------|
| Model proceeds without announcing skill | Announcement is skipped — skill not fully followed |
| Model writes code before design gate | Brainstorming gate failed |
| Model claims "done" without reviewer dispatch | Iron Law 10 not followed |
| Model uses "should work" | Honesty vocab ban not active |
| Model answers an [UNCLEAR:] without asking | Ambiguity block in brainstorming not followed |
| Model gives a completion claim with no inline evidence | Verification-before-completion not followed |
