---
name: writing-skills
description: Use when creating, editing, or reviewing a skill file.
---

## Iron Law

```
NO SKILL SHIPS WITHOUT MEETING THE 5-ELEMENT ANATOMY CHECK
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST verify all 5 elements are present before shipping any skill. No exceptions.
Missing any single element means the skill is incomplete — fix it before using it.

**Announce at start:** "I am using the writing-skills skill to [create/edit/review] the [skill-name] skill."

---

# Instructions for Agent

## The 1% Threshold Gate

**BEFORE creating a new skill, verify it meets the threshold:**

```
GATE: Will this skill be invoked in at least 1% of development sessions?

YES → proceed to create the skill
NO  → document the pattern in the relevant skill's references/ directory instead
```

---

## DDD Bounded Context Map

Every skill belongs to exactly one bounded context. Place new skills in the correct context.

| Context | Domain | Skills |
|---------|--------|--------|
| **EXECUTION** | How work is planned and done | execution, writing-plans, brainstorming, subagent-driven-development, dispatching-parallel-agents, using-git-worktrees |
| **QUALITY** | How code meets correctness standards | testing, visual-regression-testing, code-quality, cpp-patterns, verification-before-completion, systematic-debugging |
| **DELIVERY** | How code ships | versioning, build, workflow, finishing-a-development-branch |
| **REVIEW** | How work is validated | architecture-review, infrastructure-review, skill-reviewer, requesting-code-review, receiving-code-review |
| **REFLECTION** | How the agent improves | self-evaluation, session-postmortem |
| **KNOWLEDGE** | How knowledge is captured | documentation, writing-skills |
| **PRODUCT** | How features are defined | user-story-generator, user-story-estimation |
| **BEHAVIOR** | How the agent behaves honestly and bootstraps | honesty, session-bootstrap |

**Sub-domain skills** (e.g., `visual-regression-testing` under QUALITY) are valid when they have a distinct iron law and are invoked independently of the parent skill.

---

## Skill Anatomy — 5 Required Elements

Every skill MUST contain all five elements:

```
1. FRONTMATTER    — name + description (triggering conditions only)
2. IRON LAW       — ALL CAPS rule + "letter/spirit" line
3. ANNOUNCEMENT   — "I am using [skill] to [purpose]"
4. GATE FUNCTION  — BEFORE PROCEEDING: numbered steps with YES/NO branches
5. RATIONALIZATION TABLE — specific excuses with counters
```

Any absent element = skill is incomplete. Fix before shipping.

**Frontmatter also applies to agent templates:** `.github/agents/*.md` prompt template files require the same `name` + `description` YAML frontmatter block as skill files. Missing frontmatter causes parse errors when the CLI loads the agent.

---

See `references/SKILL_ANATOMY_ELEMENTS.md` for full element schemas, bad/good examples, and rationale for each element.

---

## Voice Authority Rules

See `references/VOICE_AUTHORITY_RULES.md` for the full voice quality rules (authority table, Absolute Path Rule, Acronym Rule). Apply all three to every line of every skill.

---

See `references/SIZE_AND_COMPRESSION.md` for word count targets, the GPT-4.1 mechanical execution rules, compression rules, and line limits.

---

## Red Flags — STOP

- Creating a skill for a pattern in fewer than 1% of sessions — **STOP. Write a reference doc instead.**
- Description field contains "covers", "enforces", "provides", "includes" — **STOP. It summarizes workflow. Rewrite to "Use when [conditions]."**
- Iron Law lacks "letter/spirit" line — **STOP. Add "Violating the letter of this rule is violating the spirit of this rule." now.**
- Iron Law lacks "YOU MUST" and "No exceptions." — **STOP. Both phrases are required. Add them.**
- No BEFORE PROCEEDING gate — **STOP. Add a gate with explicit YES/NO branches before shipping.**
- No rationalization table — **STOP. Add ≥5 rows now. A skill without one is a suggestion.**
- Skill contains "should", "prefer", "consider" — **STOP. Replace every instance with MUST, DO NOT, or an explicit imperative.**

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "The description is good enough as-is" | If it mentions what the skill does (not when to use it), models will follow the description instead of reading the skill |
| "Letter/spirit line is redundant" | It explicitly closes the rationalization that "technically I followed the rule" |
| "Rationalization tables are overkill" | They are the highest-ROI element — they name and counter excuses before they occur |
| "This pattern only comes up occasionally" | If it's occasional, it belongs in a reference doc, not a skill |
| "I'll add the gate function later" | A skill without a gate is a suggestion, not a constraint |

---

## Related Skills

- `skill-reviewer` — runs the 5-element gate check on completed skills; dispatch 1 per file
- `documentation` — governs how skill reference docs are structured, formatted, and linked
- `self-evaluation` — reviews skills updated during a session using this checklist

## Reference Reading Gate

```
Editing frontmatter only (no anatomy changes)?
  YES → reference files optional
  NO  → SKILL_ANATOMY_ELEMENTS.md MUST be read before making changes
Creating a new skill or modifying anatomy elements (iron law, gate, rationalization table)?
  → View references/SKILL_ANATOMY_ELEMENTS.md now — before any edits
```

## References

- `writing-skills/references/SKILL_ANATOMY_ELEMENTS.md` — full element schemas, bad/good examples, rationale for each of the 5 elements
- `writing-skills/references/SIZE_AND_COMPRESSION.md` — word count targets, GPT-4.1 mechanical execution rules, compression rules, line limits
- `writing-skills/references/VOICE_AUTHORITY_RULES.md` — authority table, Absolute Path Rule, Acronym Rule; embed directly in reviewer agent prompts
- `writing-skills/references/MODEL_COMPATIBILITY.md` — patterns most likely to be skipped by lower-end models and how to write skills that survive them
