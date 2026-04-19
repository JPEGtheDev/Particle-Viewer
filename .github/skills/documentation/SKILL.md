---
name: documentation
description: Use when creating docs, updating guides, writing API references, or reviewing documentation for Particle-Viewer.
---

## Iron Law

```
DOCUMENT WHAT, NOT WHY — WHY LIVES IN SKILLS AND DOCS
```

Violating the letter of this rule is violating the spirit of this rule.

Code comments explain *what* code does. Explanations >5 lines belong in `skills/` or `docs/`, not inline. Never duplicate content across doc files.

**Announce at start:** "I am using the documentation skill to [write/update/review] [description]."

---

# Instructions for Agent

## How This Skill is Invoked

In VS Code, users will activate this skill by:
- Typing `@workspace /documentation [description]` in Copilot Chat
- Or asking: "Write docs for [feature]", "Update the testing guide", "Add API documentation"

When activated, write documentation that follows the project's established conventions.

---

## Core Principle: Documentation Lives Close to Code

Documentation MUST be discoverable, accurate, and maintained alongside the code it describes.

---

## Step 1: Determine Documentation Type

Ask the user what they need:

> "What documentation would you like? I support:
> 1. **API/class documentation** — Inline comments and header docs for classes
> 2. **Guide/tutorial** — How-to guides in `docs/`
> 3. **Standards document** — Coding, testing, or process standards
> 4. **Skill documentation** — Copilot skill SKILL.md and references
> 5. **Documentation review** — Check existing docs for accuracy and broken links"

---

## Step 2: Follow Documentation Conventions

### File Organization

| Type | Location | Example |
|------|----------|---------|
| Code standards | `docs/CODING_STANDARDS.md` | Formatting, naming, static analysis |
| Testing standards | `docs/TESTING_STANDARDS.md` | AAA pattern, naming, coverage |
| Feature guides | `docs/testing/*.md` | Visual regression guide |
| Process docs | `docs/RELEASE_PROCESS.md` | Release automation |
| Quick references | `docs/CONVENTIONAL_COMMITS.md` | Commit format |
| Copilot skills | `.github/skills/<name>/SKILL.md` | Testing, workflow skills |
| Skill references | `.github/skills/<name>/references/*.md` | Examples, patterns |
| Instructions | `.github/copilot-instructions.md` | Agent onboarding |

### Formatting Standards

1. **Use markdown** for all documentation
2. **Include a Table of Contents** for documents with 3+ sections
3. **Use tables** for structured reference data (types, locations, options)
4. **Use code blocks** with language tags for all code examples
5. **Use relative links** between documents — verify they resolve correctly from the file's location
6. **Keep line length reasonable** — no hard limit but aim for readability

### Linking Rules

- **Always use relative paths** between docs (e.g., `../TESTING_STANDARDS.md` from `docs/testing/`)
- **Verify link targets exist** before adding links
- **Always account for the source file's directory** when computing relative paths
- **Link to specific sections** with anchors when referencing a subsection

### Content Standards

1. **Be concrete, not vague** — use specific file paths, class names, and commands
2. **Include examples** — show both correct and incorrect patterns where applicable
3. **Keep docs in sync with code** — when changing code, update related docs in the same PR
4. **Use tables for type references** — struct members, enum values, API surfaces

---

## Step 3: Apply Skill-Specific Rules

### For Copilot Skills

Skills follow a specific structure. See [references/DOCUMENTATION_EXAMPLES.md](references/DOCUMENTATION_EXAMPLES.md) for templates.

Key rules for skills:
1. **Minimize duplication** — skills should reference other skills, not repeat their content
2. **SKILL.md** contains the workflow and rules
3. **references/** contains examples, patterns, and templates
4. **Keep skills focused** — one skill per domain (testing, workflow, documentation)

### For copilot-instructions.md

The instructions file is the agent onboarding document:
1. **Keep it concise** — summarize rules, reference skills for details
2. **No examples in instructions** — examples belong in skill references
3. **No detailed guidelines in instructions** — guidelines belong in skills
4. **Reference skills by path** — e.g., "use the `testing` skill (`.github/skills/testing/`)"

**When adding a new skill, update all 4 locations in copilot-instructions.md:**
1. **Skills Directory table** — add the row with name, path, domain
2. **Before Every Response checklist** — add an item if the skill has a HARD-GATE trigger condition
3. **Minimum skill loads table** — add the row(s) for when this skill must be read
4. **Instruction Priority Hierarchy** — no change needed unless the skill introduces a new priority tier

Skipping any of these means the skill exists but is never loaded — it is effectively invisible.

---

## Step 4: Review Checklist

Before presenting documentation, verify:

- [ ] File is in the correct location per the organization table
- [ ] All relative links resolve correctly from the file's directory
- [ ] Code examples have language tags and are syntactically valid
- [ ] Tables are properly formatted
- [ ] Content is concrete (specific paths, commands, class names)
- [ ] No duplication with other docs — reference instead of repeat
- [ ] Table of contents matches section headings
- [ ] Documentation matches current code state

✓ All pass → documentation is ready to present
✗ Any unmet → resolve all failing items before presenting documentation

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "I'll document it after it's working" | Documentation written separately from code drifts immediately. Write it with the code. |
| "The code is self-documenting" | Code explains what. Docs and skills explain why, when, and the design rationale. |
| "A long comment will do for now" | Comments >5 lines of explanation belong in `skills/` or `docs/`. Move it. |
| "I'll update the docs in a follow-up" | Follow-up docs never match the implementation. Update alongside the change. |
| "This is internal code, no docs needed" | Internal code is the hardest to understand 6 months later. Document it. |
| "The skill already covers this" | Check for duplication — if both a skill and a doc cover it, one is wrong. Consolidate. |

---

## Red Flags — STOP

If you catch yourself thinking any of these, stop and follow the rule:
- Writing a code comment longer than 5 lines
- "I'll add documentation in the next PR"
- Two files saying the same thing in different words
- Copying content from a skill into a doc file (or vice versa)
- "This is obvious, no docs needed"
- Updating code without checking if related docs are now stale

**All of these mean: Document what the code does, and put the why in `skills/` or `docs/`. Update docs alongside code changes.**

---

## Reference

For documentation templates and examples, see [references/DOCUMENTATION_EXAMPLES.md](references/DOCUMENTATION_EXAMPLES.md).
