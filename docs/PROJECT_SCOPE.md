---
title: "Project Scope Gate"
description: "Scope gate for Particle Viewer work: what the project is (viewer) and is not (simulator), and the project-context loading guide."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, context, scope, planning]
related:
  - "PROJECT_CONTEXT.md"
---

# Particle-Viewer Project Scope Gate

## Step 1: Understand the Request -- Verify Project Scope First

**CRITICAL: Verify Project Scope First**

Before asking any other questions, **load and review** [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) to understand:
- **What this project actually does** (it's a VIEWER for pre-computed simulations, NOT a simulator)
- What components exist vs don't exist
- Current architecture and pain points

**Do NOT assume functionality based on project naming:**
- "Particle-Viewer" = visualization tool, not physics engine
- Reads particle data from files, doesn't compute physics
- Focus on rendering, file I/O, camera controls, visual testing

**If the user mentions functionality that doesn't exist in the project**, ask:
> "Just to clarify--are you planning to add [feature], or did I misunderstand the scope? I want to make sure we're working on something that fits the project's purpose as a [viewer/simulator/etc]."

## Using Project Context

**ALWAYS load project context FIRST before generating any story.**

Throughout the conversation, Load project-specific references **when needed**:

### For Particle-Viewer stories:
**Load [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) IMMEDIATELY when:**
- User requests ANY story for this project
- Before asking clarifying questions (so you ask the RIGHT questions)
- Need to understand what actually exists vs assumptions

**Use it to verify:**
- Is this feature request aligned with project purpose?
- Does this component/functionality actually exist?
- Are we focusing on real value (rendering, I/O, visualization) or imagined features (physics simulation)?

### For INVEST validation:
Apply the INVEST (Independent, Negotiable, Valuable, Estimable, Small, Testable) framework when:
- User asks "what makes a good story?"
- Story seems to violate INVEST principles
- Need to explain why you're suggesting certain changes

### For story examples:
Draw on story format examples when:
- User is new to story writing
- Need to show format variations (BDD (Behavior-Driven Development) vs bullets)
- Asked for examples of features, refactors, spikes, or bugs

**Don't load project context speculatively.** Use progressive disclosure--load `PROJECT_CONTEXT.md` only when you need it, not upfront for every conversation.

## Related

- [Project Context](PROJECT_CONTEXT.md) -- component inventory, architecture overview, and what this project is/is not
