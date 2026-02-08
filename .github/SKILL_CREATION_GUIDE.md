# Creating New Agent Skills

Guidelines for creating new skills that follow the agentskills.io specification and work seamlessly with GitHub Copilot.

---

## Quick Checklist

Before creating a skill, ensure you can answer:
- [ ] **What does this skill do?** (one sentence)
- [ ] **When should it activate?** (specific triggers/keywords)
- [ ] **What problem does it solve?** (user need)
- [ ] **Is this better as a skill vs inline instructions?** (reusable pattern?)

---

## Skill Structure

```
.github/skills/
└── your-skill-name/
    ├── SKILL.md                    # Required: Main instructions for agent
    └── references/                 # Optional: Additional context loaded on-demand
        ├── REFERENCE.md
        └── EXAMPLES.md
```

**Do NOT create:**
- `scripts/` directory (keep it simple, avoid overcomplicating)
- `assets/` directory (unless absolutely necessary)
- Multiple skill files (one `SKILL.md` per skill)

---

## SKILL.md Format

### Frontmatter (Required)

```yaml
---
name: your-skill-name
description: Brief description of what the skill does and when to use it. Include specific keywords that trigger this skill. Max 1024 characters.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: YourName
  version: "1.0"
  category: [e.g., testing, refactoring, documentation]
  project: [Optional: ProjectName]
---
```

**Name Rules:**
- Lowercase only
- 1-64 characters
- Use hyphens (`-`) not underscores or spaces
- No leading/trailing hyphens
- No consecutive hyphens (`--`)
- Must match parent directory name

**Description Rules:**
- 1-1024 characters
- Include "what" and "when"
- Use keywords agents will search for
- Example: "Generate user stories with INVEST principles. Use when planning sprints, creating epics, or breaking down features."

### Body Content (Instructions for Agent)

**Key Principles:**
1. **Write for the AI agent, not humans**
   - Use imperative: "Do this", "Generate that"
   - Not: "This skill helps you generate..."

2. **Be conversational first**
   - Instruct the agent to ask questions before acting
   - Progressive disclosure: gather info → confirm → generate
   - Allow user overrides at any point

3. **Provide clear workflows**
   - Step 1: [What to do]
   - Step 2: [What to do next]
   - Use numbered steps or clear sections

4. **Include examples**
   - Show conversation flow
   - Demonstrate expected output
   - Provide edge case handling

**Recommended Sections:**

```markdown
# Instructions for Agent

## How This Skill is Invoked
[Explain how users will activate this skill in VS Code Copilot Chat]

## Core Approach
[High-level philosophy - conversation first, generation second, etc.]

## Step 1: [First Action]
[Detailed instructions]

## Step 2: [Next Action]
[More instructions]

## Key Principles
[Guidelines the agent should follow]

## Common Edge Cases
[How to handle vague requests, errors, etc.]

## Before Presenting Output
[Final checklist/validation]
```

---

## Progressive Disclosure Strategy

Skills should load context efficiently:

### Metadata (~100 tokens)
Loaded at startup for all skills. Keep description concise but keyword-rich.

### SKILL.md Body (<5000 tokens)
Loaded when skill activates. Keep under 500 lines.

**What to include:**
- Core workflow instructions
- Example conversations
- Key principles and guidelines
- Common edge cases

**What to move to references:**
- Detailed framework explanations
- Lengthy examples
- Project-specific conventions
- Template libraries

### References (loaded on-demand)
Only loaded when agent needs specific context.

**Good reference files:**
- `FRAMEWORK.md` - Detailed methodology (e.g., INVEST principles)
- `TEMPLATES.md` - Reusable templates or examples
- `PROJECT_CONTEXT.md` - Project-specific conventions
- `EXAMPLES.md` - Extended use cases

**Keep references focused:** Each file should cover ONE topic.

---

## Writing Agent Instructions

### ✅ Good: Imperative, Agent-Focused
```markdown
## Step 1: Gather Requirements

Ask the user clarifying questions conversationally:

> "I'd love to help with that! Just a few quick questions:
> 1. What problem are you trying to solve?
> 2. Is this urgent or exploratory work?"

**Allow them to skip questions** - use defaults if they say "just go".
```

### ❌ Bad: User-Focused Documentation
```markdown
## How to Use This Skill

To use this skill, you'll provide the following information to Copilot:
1. Your project requirements
2. The scope of work

The skill will then generate output for you.
```

---

## Skill Anti-Patterns

### ❌ Don't: Form-Based Skills
```markdown
Collect the following inputs:
1. Project name: ______
2. Framework: ______
3. Database: ______
```

### ✅ Do: Conversational Skills
```markdown
Start a natural conversation:

> "What are you building? I'll ask follow-ups to understand the requirements."

Based on their answer, ask ONE relevant follow-up question.
```

---

### ❌ Don't: Rigid Templates
```markdown
Generate this exact output:

[Copy this template verbatim]
```

### ✅ Do: Flexible Generation
```markdown
Generate output based on user's needs. Structure should include:
- [Section 1]
- [Section 2]

Adjust formatting based on their preferences (they may ask for different style).
```

---

### ❌ Don't: Over-Specify Implementation
```markdown
Use React hooks.
Write TypeScript interfaces first.
Create these exact files: [list]
```

### ✅ Do: Outcome-Focused Instructions
```markdown
Create a component that manages user authentication state.

**If the user has preferences (React, Vue, etc.)**, honor them.
**Otherwise**, infer from project context.
```

---

## Effort Estimation (If Applicable)

For skills that generate work items (user stories, tasks), include estimation guidance:

```markdown
## Estimating Effort

Provide estimates in [metric: story points, hours, premium requests].

**Small:** [criteria]
**Medium:** [criteria]  
**Large:** [criteria]

Consider:
- Implementation complexity
- Testing requirements
- Integration effort
- Documentation needs
```

---

## Model Selection (If Applicable)

If certain tasks require specific AI models:

```markdown
## Model Recommendations

**Small (Lightweight):** Simple, well-defined tasks suitable for quick turnaround.
**Standard:** Typical implementation and integration work with moderate complexity.
**Advanced:** Complex architecture or deep reasoning tasks that require design and trade-offs.

When recommending, explain why:
> "This task requires an advanced model because [architectural decisions/complex refactoring]."
```

---

## Reference File Guidelines

### When to Create References

Create reference files when:
- Content is >200 lines
- Information is optional/contextual
- Multiple skills might use the same information
- Content is project-specific

### Reference File Structure

```markdown
# [Topic Name]

Brief introduction to what this reference covers.

---

## Section 1

Content...

## Section 2

Content...

---

## Quick Reference

[Summary or checklist at the end]
```

### Linking to References

In SKILL.md:
```markdown
For detailed framework principles, see [references/FRAMEWORK.md](references/FRAMEWORK.md).

**Only load this file when:** [specific condition]
```

In instructions:
```markdown
"If the user asks about INVEST principles, load [references/INVEST_FRAMEWORK.md](references/INVEST_FRAMEWORK.md) and explain."
```

---

## Testing Your Skill

### Manual Testing Checklist

- [ ] Activate skill in VS Code Copilot Chat
- [ ] Test with vague request (does it ask clarifying questions?)
- [ ] Test with specific request (does it generate correctly?)
- [ ] Test override behavior (can user change direction mid-conversation?)
- [ ] Test iteration (can user refine output?)
- [ ] Verify references load only when needed
- [ ] Check output quality and format

### Validation

Run validation if available:
```bash
skills-ref validate .github/skills/your-skill-name
```

Check:
- [ ] Frontmatter is valid YAML
- [ ] Name matches directory name
- [ ] Description is <1024 characters
- [ ] No prohibited characters in name
- [ ] SKILL.md is <500 lines
- [ ] References are one level deep

---

## Common Mistakes

### 1. Writing User Documentation Instead of Agent Instructions
**Wrong:** "This skill helps you create user stories by asking questions."  
**Right:** "Ask the user questions to understand their requirements before generating."

### 2. Not Allowing Overrides
**Wrong:** "Generate story with default template."  
**Right:** "Generate story with default template. If user requests changes, update immediately."

### 3. Front-Loading Too Many Questions
**Wrong:** Ask 10 questions upfront  
**Right:** Ask 2-3 questions, generate, then offer refinements

### 4. Overcomplicating with Scripts
**Wrong:** Create Python validation script, shell script helper, etc.  
**Right:** Keep instructions in SKILL.md, let agent handle logic

### 5. Forgetting Progressive Disclosure
**Wrong:** Include all examples, templates, and references in SKILL.md  
**Right:** Link to references, load only when needed

### 6. Vague Activation Triggers
**Wrong:** "Use for development tasks"  
**Right:** "Use when creating user stories, planning sprints, breaking down epics, or refactoring work into manageable tasks"

---

## Breaking Up Work for Parallel Agents

When generating user stories for work that will be distributed to multiple agents (especially using parallel AI models), **structure stories to match agent capability tiers:**

### Model-to-Skill Mapping

**Small (Junior-level work):**
- Simple, well-scoped tasks with clear inputs/outputs
- Single-concern implementations (no cross-cutting complexity)
- Routine, established patterns (no architectural decisions)
- Examples: Write unit tests for data structures, create simple utilities, apply coding standards

**Standard (Mid-level work):**
- Moderate complexity (multi-step, multiple considerations)
- Integration work (connecting established components)
- Problem-solving with known patterns (error handling, setup, configuration)
- Examples: Standard feature implementation, CI integration, graphics/shader tests with mocks

**Advanced (Senior-level work):**
- Architectural decisions and design
- Complex integration (subtle interactions, race conditions, global state)
- Novel solutions (not following established patterns)
- Deep reasoning about trade-offs
- Examples: Refactoring global state, designing integration tests, system-wide quality gates

### Example: Breaking Up Test Coverage

**Original story (too big):** "Increase Test Coverage for Core Components" (50-70 requests)

**Broken into three investable stories:**

1. **Unit Tests for Data Structures** (Small, 10-15 requests)
   - Particle, Settings tests
   - No mocking, no GL dependencies
   - Clear test structure

2. **Unit Tests for Graphics & CI** (Standard, 25-35 requests)
   - Camera, Shader tests + mocks
   - CI workflow setup
   - Standard patterns

3. **Integration Tests & Quality Gates** (Advanced, 15-25 requests)
   - Viewer pipeline integration tests
   - Coverage enforcement logic
   - Flakiness elimination strategy

**Benefits:**
- Each story matches an agent's capability level
- Work can proceed in parallel (with dependencies)
- Effort estimates are more accurate per agent
- Juniors focus on straightforward tasks; seniors on complex decisions

### When to Break Up Stories

Break up if:
- Story spans multiple architectural concerns
- Different sections require different skill levels
- Can't describe in one clear focus area
- Effort estimate mixes simple + complex tasks

**Don't break up if:**
- Story is already small and focused
- Breaking creates artificial dependencies
- All work requires same skill level

---

## Examples of Good Skills

### Conversational Story Generator
- Asks questions naturally
- Confirms understanding before generating
- Allows iteration
- Includes effort estimates
- References framework docs on-demand

### Code Reviewer
- Asks what type of review (security, performance, style)
- Generates feedback with specific line references
- Offers to explain recommendations
- Loads coding standards from references when needed

### Architecture Documenter
- Asks about system scope
- Generates diagrams (mermaid)
- Confirms with user before finalizing
- References architecture patterns from separate file

---

## Skill Naming Conventions

Good names are:
- Action-oriented: `user-story-generator`, `code-reviewer`
- Specific: `visual-regression-tester` not `tester`
- Consistent: Use same pattern across skills

Examples:
- ✅ `user-story-generator`
- ✅ `api-documentation-writer`
- ✅ `test-data-generator`
- ❌ `helper` (too vague)
- ❌ `Utils` (not descriptive)
- ❌ `my_skill` (use hyphens, not underscores)

---

## Documentation Requirements

After creating a skill, document:

### In README.md (or .github/INSTRUCTIONS.md)
```markdown
## Available Skills

### user-story-generator
Generate INVEST-aligned user stories through conversation.

**Usage:** `Create a user story for [feature]`

**What it does:** Asks clarifying questions, generates formatted stories, includes effort estimates
```

### In Skill's SKILL.md
Include "How This Skill is Invoked" section at the top.

---

## Version Management

When updating skills:

1. **Increment version** in metadata:
```yaml
metadata:
  version: "1.1"  # Was "1.0"
```

2. **Document changes** in comments or CHANGELOG

3. **Test compatibility** - ensure existing usage still works

4. **Keep backward compatible** when possible

---

## Summary: The Perfect Skill

✅ **Clear activation triggers** in description  
✅ **Conversational approach** - asks before generating  
✅ **Allows overrides** - user control at every step  
✅ **Progressive disclosure** - uses references efficiently  
✅ **Under 500 lines** in SKILL.md  
✅ **Focused and specific** - does ONE thing well  
✅ **Agent-focused instructions** - not user docs  
✅ **Tested** - works in real Copilot Chat scenarios  
✅ **Documented** - users know when/how to use it

---

## Getting Help

- **Specification:** [agentskills.io/specification](https://agentskills.io/specification)
- **Examples:** Review existing skills in `.github/skills/`
- **Validation:** Use `skills-ref validate` if available
- **Community:** Share skills and get feedback from team
