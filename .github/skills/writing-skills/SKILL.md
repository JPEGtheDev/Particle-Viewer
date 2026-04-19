---
name: writing-skills
description: Use when creating a new skill, editing an existing skill, or reviewing a skill for quality compliance.
---

## Iron Law

```
NO SKILL SHIPS WITHOUT MEETING THE 5-ELEMENT ANATOMY CHECK
```

Violating the letter of this rule is violating the spirit of this rule.

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

A skill that covers a rarely-used edge case becomes cognitive overhead, not leverage. Use reference docs for low-frequency patterns.

---

## DDD Bounded Context Map

Every skill belongs to exactly one bounded context. Place new skills in the correct context.

| Context | Domain | Skills |
|---------|--------|--------|
| **EXECUTION** | How work is planned and done | execution, writing-plans, brainstorming, subagent-driven-development, using-git-worktrees |
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

If any element is absent: the skill is incomplete. Fix it before shipping.

---

## Element 1: Frontmatter

```yaml
---
name: skill-name-with-hyphens
description: Use when [triggering conditions and symptoms only]
---
```

**Rules:**
- `description` = triggering conditions ONLY. Never summarize what the skill does.
- If the description summarizes workflow, the model will follow the description instead of reading the skill body.
- Start with "Use when..." in third person
- Keep under 200 characters

**Bad (workflow summary):**
```yaml
description: Use when writing tests. Follows AAA pattern, naming conventions, covers Google Test patterns.
```

**Good (triggering conditions):**
```yaml
description: Use when writing or reviewing any test for Particle-Viewer.
```

---

## Element 2: Iron Law

```markdown
## Iron Law

​```
[ALL CAPS RULE — no exceptions language]
​```

Violating the letter of this rule is violating the spirit of this rule.
```

The "letter/spirit" line is mandatory. It closes the rationalization gap where agents argue "technically I followed the rule" while violating its intent.

---

## Element 3: Announcement

```markdown
**Announce at start:** "I am using the [skill-name] skill to [purpose]."
```

This is the commitment mechanism. Public commitment to a behavior increases follow-through.

---

## Element 4: Gate Function

Every skill must have at least one BEFORE PROCEEDING gate:

```markdown
BEFORE [action], verify:
1. [Condition one]
2. [Condition two]
3. [Condition three]

✓ All met → proceed
✗ Any unmet → [specific required action before proceeding]
```

Gates must specify what happens on FAIL — not just list conditions.

---

## Element 5: Rationalization Table

```markdown
## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "[specific rationalization]" | [direct counter] |
```

Rationalization tables are the most powerful element — they preemptively name the excuses and counter them before the agent reaches for them.

---

## Voice Authority Rules

Apply to every line of a skill:

| Soft (forbidden) | Authority (required) |
|-----------------|---------------------|
| "should" | MUST |
| "prefer X" | "Use X" or give an explicit decision rule |
| "consider" | "Check" / "Verify" / specific action |
| "try to" | absolute rule or explicit condition |
| "often / usually / typically" | always / never / explicit condition |
| "it might be worth" | state the rule directly |
| "you could potentially" | state the action directly |

**The test:** Can the agent rationalize past this sentence? If yes, make it a rule.

---

## Size Limits

| Skill type | Target | Hard limit |
|------------|--------|------------|
| Frequently loaded (EXECUTION, QUALITY) | ≤300 lines | 500 lines |
| Reference skills (DELIVERY, REVIEW) | ≤250 lines | 400 lines |
| Sub-domain skills | ≤150 lines | 200 lines |

When a skill exceeds its hard limit, split by domain. Each split must have a distinct iron law.

---

## Red Flags — STOP

- Creating a skill for a pattern that occurs in fewer than 1% of sessions
- Description field contains "covers", "enforces", "provides", "includes" → rewrites workflow
- Iron Law does not include the "letter/spirit" line
- No BEFORE PROCEEDING gate
- No rationalization table
- Skill contains "should", "prefer", "consider"
- Skill size exceeds hard limit

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "The description is good enough as-is" | If it mentions what the skill does (not when to use it), models will follow the description instead of reading the skill |
| "Letter/spirit line is redundant" | It explicitly closes the rationalization that "technically I followed the rule" |
| "Rationalization tables are overkill" | They are the highest-ROI element — they name and counter excuses before they occur |
| "This pattern only comes up occasionally" | If it's occasional, it belongs in a reference doc, not a skill |
| "I'll add the gate function later" | A skill without a gate is a suggestion, not a constraint |
