---
name: writing-skills
description: Use when creating a new skill, editing an existing skill, or reviewing a skill for quality compliance.
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

A skill that covers a rarely-used edge case becomes cognitive overhead, not leverage. Use reference docs for low-frequency patterns.

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

See `references/VOICE_AUTHORITY_RULES.md` for the full voice quality rules (authority table, Absolute Path Rule, Acronym Rule). Apply all three to every line of every skill.

---

## Size and Token Efficiency

**Why this matters:** Skills are written for GPT-4.1 as the baseline. Every instruction must be mechanically precise and literally executable — no nested conditionals, no implied reasoning chains. Verbose skills evict iron law content from the context window.

### Word Count Targets

| Skill type | Target words | Hard limit |
|------------|-------------|------------|
| EXECUTION / QUALITY (frequently loaded) | ≤ 400 words | 600 words |
| DELIVERY / REVIEW / KNOWLEDGE | ≤ 500 words | 800 words |
| Sub-domain skills | ≤ 200 words | 350 words |
| Reference files (not SKILL.md) | No limit — loaded on demand |

**Check before shipping:**
```bash
wc -w .github/skills/<skill-name>/SKILL.md
```

### Writing for Mechanical Execution (GPT-4.1 Baseline)

Every line in every skill MUST satisfy this test: **Can a GPT-4.1 class model execute this instruction literally without inferring intent?**

Rules:
- Use imperative sentences. Not "you should check" — "Check. Stop if not met."
- Flatten all conditional logic into sequential steps. No "if X then Y else Z" — write two separate rules.
- No implicit dependencies. If a gate requires loading another skill, say: "Load `[skill]` now. Do not proceed until loaded."
- Iron law comes FIRST — before explanation, before context.
- Announcement comes SECOND — it is a commitment mechanism, not a courtesy.
- Gate function comes THIRD — step-by-step with explicit STOP conditions.

### Compression Rules

1. **Move heavy reference to a `references/` file — never delete it.**

   GOOD: Skill file has one pointer: `See references/NAMING_RULES.md for full naming tables.`
   BAD: Naming tables deleted to save lines — the same naming mistakes now recur with no documented reason why the rule exists.

2. **Never delete content when compressing — move it.** Move excess to `references/`. Decision rationale, examples, and "why we did this" explanations belong in references even if not loaded every session. A deleted "why" gets rediscovered the hard way.

   GOOD: References file states: "We use `m_` prefix because clang-tidy rule `readability-identifier-naming` enforces it — removing the prefix silently breaks CI."
   BAD: Rationale removed to save lines. Next engineer removes `m_` prefixes not knowing why. CI breaks.

3. **Never repeat content another skill owns.** Cross-reference with a one-line pointer.

   GOOD: `execution` skill says: "For commit format, see the `versioning` skill."
   BAD: `execution` skill copies the full conventional commit table from `versioning` — both diverge over time.

4. **One example per pattern.** Delete redundant examples.

   GOOD: One test name example showing `ClassName_Action_ExpectedResult`.
   BAD: Six variations of the same pattern — models pick the last example seen, not the rule.

5. **Description field never summarizes workflow.** See Element 1.

   GOOD: `description: Use when writing or reviewing any test for Particle-Viewer.`
   BAD: `description: Use when writing tests. Follows AAA pattern, enforces naming, handles Google Test patterns.` — models followed the description instead of reading the skill body.

---

## Size Limits (Lines — secondary; word count above is primary)

| Skill type | Target | Hard limit |
|------------|--------|------------|
| Frequently loaded (EXECUTION, QUALITY) | ≤300 lines | 500 lines |
| Reference skills (DELIVERY, REVIEW) | ≤250 lines | 400 lines |
| Sub-domain skills | ≤150 lines | 200 lines |

When a skill exceeds its hard limit, split by domain. Each split must have a distinct iron law.

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

## References

- `writing-skills/references/MODEL_COMPATIBILITY.md` — patterns most likely to be skipped by lower-end models and how to write skills that survive them
- `writing-skills/references/VOICE_AUTHORITY_RULES.md` — full voice quality rules: authority table, Absolute Path Rule, Acronym Rule; embed directly in reviewer agent prompts
