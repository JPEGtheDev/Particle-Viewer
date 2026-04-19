# Skill Anatomy — Element Reference

Full schemas and examples for each of the 5 required skill elements.

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
