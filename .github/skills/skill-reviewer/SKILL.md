---
name: skill-reviewer
description: Use when authoring or updating skills to review files for completeness, iron laws, and mechanical gate patterns. Dispatch 1 instance per skill file. Returns a structured review report.
---

## Iron Law

```
A SKILL WITHOUT AN IRON LAW IS DOCUMENTATION, NOT ENFORCEMENT
```

Every skill YOU MUST have all five gate elements. Missing any one = incomplete skill. No exceptions.

**Announce at start:** "I am using the skill-reviewer skill to review [skill-name]."

---

## The Five Gate Elements

Every skill MUST contain ALL of the following. Missing any one element is an automatic FAIL.

1. **Iron Law** — `## Iron Law` section with backtick-wrapped law statement, "No exceptions" language, and "YOU MUST" phrasing
2. **Announcement requirement** — explicit line: "Announce at start: I am using [skill] to [purpose]"
3. **Rationalization Prevention table** — minimum 5 rows, each naming a specific excuse with a specific counter
4. **Red Flags→STOP section** — minimum 5 trigger thoughts, each with a concrete action to take
5. **Version** — YAML frontmatter `version:` field, present and current

---

## Review Checklist

Run every item for each skill file under review:

- [ ] Iron Law block present and uses hard language ("YOU MUST", "No exceptions", bright-line rule)?
- [ ] Announcement requirement is explicit (exact wording required, not implied)?
- [ ] Rationalization Prevention table present with ≥5 rows?
- [ ] Red Flags→STOP section present with ≥5 trigger thoughts?
- [ ] Version field in YAML frontmatter?
- [ ] Skill-specific commands present (not generic placeholder examples)?
- [ ] Cross-references to at least 1 related skill?
- [ ] Domain language is correct (C++/CMake/Google Test specific — not Python/JS/generic)?

---

## Dispatch Pattern

For every skill file being reviewed:

1. Dispatch ONE skill-reviewer agent per file (parallel — do not wait for one before starting the next)
2. Provide the agent: skill file path, list of recent changes (if any), and the five-element checklist above
3. Collect ALL reports before acting on any of them
4. For each FAIL item: update the skill and re-dispatch a review of that specific element

---

## Review Report Format

A skill-reviewer agent MUST return its findings in this exact structure:

```markdown
## Skill Review: [skill-name] v[version]

### Gate Elements
| Element | Present | Quality | Notes |
|---------|---------|---------|-------|
| Iron Law | ✅/❌ | Strong/Weak | ... |
| Announcement | ✅/❌ | Clear/Vague | ... |
| Rationalization Table | ✅/❌ | N rows | ... |
| Red Flags→STOP | ✅/❌ | N items | ... |
| Version | ✅/❌ | vX.Y | ... |

### Issues Found
[List each issue with file:line reference]

### Verdict: PASS / NEEDS WORK
```

A verdict of NEEDS WORK means the skill MUST be updated before it can be dispatched in production use.

---

## Red Flags — STOP

If you catch yourself thinking any of the following, STOP and re-examine before filing your verdict:

- "It has most of the required elements" → Stop. Count every element. Partial is FAIL.
- "The iron law is present but mild" → Stop. Re-read the law. Does it use "YOU MUST" or "No exceptions"? If not, mark Weak.
- "There are only 4 rows in the rationalization table, close enough" → Stop. Count them. ≥5 is the rule. 4 is a FAIL.
- "The announcement is implied by the skill description" → Stop. It must be an explicit line. Implied is a FAIL.
- "I'll note the issues but still give PASS" → Stop. There is no PASS with open issues. Verdict is NEEDS WORK.

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "It has some enforcement language, close enough" | All five elements are required. Partial is FAIL. No exceptions. |
| "The rationalization table is implied by the red flags" | They serve different cognitive functions. Both are required independently. |
| "This skill is simple, it doesn't need all elements" | All skills need all elements. Consistency is the point. Simplicity is not an exemption. |
| "I'll add the missing table later" | Later never comes. A skill without all five elements is incomplete. Add it now. |
| "The iron law is already stated in the announcement" | They are separate sections with separate functions. Both are required. |
| "The domain language seems fine, I won't check closely" | Generic examples (Python, npm, pip) in a C++/CMake skill are a FAIL. Check every command. |

---

## Related Skills

- `self-evaluation` — uses this skill's checklist when reviewing skills updated during a session
- `documentation` — governs how skills are structured, linked, and formatted
