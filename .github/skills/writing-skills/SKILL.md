---
name: writing-skills
license: MIT
description: Use when creating, editing, or reviewing a skill file.
---


## Iron Law

```
YOU MUST: VERIFY ALL FIVE GATE ELEMENTS BEFORE SHIPPING ANY SKILL. No exceptions.
```

Violating the letter of this rule is violating the spirit of this rule.

**Announce at start:** "I am using the writing-skills skill to [create/edit/review] the [skill-name] skill."

---

## BEFORE PROCEEDING

Before creating, editing, or shipping any skill or agent template:

1. **Creating a new skill?** Invoked in ≥1% of sessions? Place in the correct Domain-Driven Design (DDD) context (see AGENTS.md).
   - YES → proceed
   - NO → write a reference doc in the relevant skill's `references/` instead; do not create a skill

2. **5-element anatomy check** -- all 5 MUST be present (full schemas in `references/SKILL_ANATOMY_ELEMENTS.md`):
   - FRONTMATTER: `name` + `description` starting with "Use when..."
   - IRON LAW: ALL CAPS rule + "Violating the letter..." line + "YOU MUST" + "No exceptions."
   - ANNOUNCEMENT: `**Announce at start:** "I am using the [skill] to [purpose]."`
   - GATE FUNCTION: `## BEFORE PROCEEDING` with numbered conditions + ✓/✗ branches
   - RATIONALIZATION TABLE: `## Rationalization Prevention` with ≥5 rows

3. **Alexandrian form check** -- every non-trivial rule MUST answer Context + Forces. See `references/SKILL_ANATOMY_ELEMENTS.md`.

4. **Modifying anatomy elements?** Read `references/SKILL_ANATOMY_ELEMENTS.md` before any edits. Frontmatter-only changes: reference files optional.

5. **Auditing existing skills?** Load `skill-reviewer` agent template; inject 4 reference files; one agent per file. See Dispatch Pattern below.

✓ All met → proceed
✗ Any unmet → resolve the unmet item before touching the skill file

---

## Dispatch Pattern -- Skill Review

1. Read these four files from `writing-skills/references/`:
   - `SKILL_ANATOMY_ELEMENTS.md` → `{{SKILL_ANATOMY_ELEMENTS}}`
   - `VOICE_AUTHORITY_RULES.md` → `{{VOICE_AUTHORITY_RULES}}`
   - `SIZE_AND_COMPRESSION.md` → `{{SIZE_AND_COMPRESSION}}`
   - `REVIEW_INSTRUCTIONS.md` → `{{REVIEW_INSTRUCTIONS}}`
2. For each skill file: substitute all four placeholders in `agents/skill-reviewer.md`, set `{{SKILL_PATH}}`, `{{RECENT_CHANGES}}`, and `{{WORKTREE_PATH}}` (output of `git rev-parse --show-toplevel`), dispatch.
3. Collect all reports before acting on any result.
4. For each NEEDS WORK verdict: update the skill and re-dispatch a review of that file.

---

## Red Flags -- STOP

- Creating a skill for a pattern in fewer than 1% of sessions -- **STOP. Write a reference doc instead.**
- Description field contains "covers", "enforces", "provides", "includes" -- **STOP. Rewrite to "Use when [conditions]."**
- Iron Law lacks "letter/spirit" line -- **STOP. Add "Violating the letter of this rule is violating the spirit of this rule." now.**
- Iron Law lacks "YOU MUST" and "No exceptions." -- **STOP. Both phrases are required. Add them.**
- No BEFORE PROCEEDING gate -- **STOP. Add a gate with explicit YES/NO branches before shipping.**
- No rationalization table -- **STOP. Add ≥5 rows now. A skill without one is a suggestion.**
- Skill contains "should", "prefer", "consider" -- **STOP. Replace every instance with MUST, DO NOT, or an explicit imperative.**

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "The description is good enough as-is" | If it mentions what the skill does (not when to use it), models will follow the description instead of reading the skill |
| "Letter/spirit line is redundant" | It explicitly closes the rationalization that "technically I followed the rule" |
| "Rationalization tables are overkill" | They are the highest return-on-investment element -- they name and counter excuses before they occur |
| "This pattern only comes up occasionally" | Occasional patterns belong in a reference doc, not a skill. Skills loaded in every session cost context budget and dilute signal value. |
| "I'll add the gate function later" | A skill without a gate is a suggestion, not a constraint |
| "Context and Forces are extra writing overhead" | Rules without context misfire in edge cases. The overhead forces clarity about when the rule applies. |

---

## Related Skills

- `skill-reviewer` agent template -- dispatches review agents using this skill's reference files as injected criteria
- `documentation` -- governs how skill reference docs are structured, formatted, and linked
- `self-evaluation` -- reviews skills updated during a session using this checklist

---

## References

- `references/SKILL_ANATOMY_ELEMENTS.md` -- full element schemas, bad/good examples, rationale, Alexandrian Pattern Form guide
- `references/SIZE_AND_COMPRESSION.md` -- token count targets, compression rules, line limits
- `references/VOICE_AUTHORITY_RULES.md` -- authority table, Absolute Path Rule, Acronym Rule
- `references/MODEL_COMPATIBILITY.md` -- patterns most likely to be skipped by lower-end models
- `references/REVIEW_INSTRUCTIONS.md` -- review process, checklist, qualitative questions, return format; injected into `skill-reviewer` agent at dispatch time
