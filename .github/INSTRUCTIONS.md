# 📋 Creating User Stories with Copilot

This project has a custom **User Story Generator** skill that helps you create INVEST-aligned stories through conversation.

---

## How to Use It

In VS Code Copilot Chat, just ask:

```
Create a user story for [your feature/task]
```

Examples:
- "Create a story for framebuffer capture utility"
- "I need a story for refactoring global state"
- "Help me write a story for visual regression testing"

The skill will **start a conversation** with you to understand what you need, rather than just generating something generic.

---

## What Makes This Different

### It's Conversational

The skill doesn't dump a questionnaire on you. Instead, it asks **natural follow-up questions** based on what you've said, like a helpful colleague would:

**You:** "Create a story for framebuffer capture"

**Copilot:** 
> "I'd love to help create that story! Let me ask a few questions to make sure I get it right:
> 
> 1. What problem does this solve? (What's the user/business value?)
> 2. Is this a feature, refactor, spike, or bug fix?
> 3. Rough scope estimate—is this small (few hours), medium (1-2 days), or larger?
> 
> Feel free to skip any you're unsure about!"

### You're Always in Control

- **Skip questions:** Say "just use defaults" to move faster
- **Override anything:** Say "actually, make it simpler" or "use BDD format" and it adjusts immediately
- **Change direction mid-conversation:** "That's not quite what I meant..." → it pivots
- **Refine after generation:** The story is never "final" until you say so

### Progressive Disclosure

The skill:
1. Starts with minimal questions to understand your goal
2. Asks 1-2 follow-ups based on your answers
3. Summarizes its understanding and confirms
4. Generates the story
5. Invites you to refine it

No 10-question forms. Just a natural conversation.

---

## What You'll Get

An INVEST-aligned story with:

- **Title** – Clear, action-oriented
- **User story** – "As a [role], I want to [action], so that [value]"
- **Acceptance criteria** – Specific, measurable outcomes
- **Technical notes** – Dependencies, constraints, affected files
- **Definition of Done** – What "complete" means for this story

All tailored to your needs and Particle-Viewer conventions.

---

## Example Flow

### Starting Point
**You:** "I want to create a story for adding visual regression tests"

### Conversation
**Copilot:** 
> "Got it! Testing what specifically—framebuffer capture, image comparison, CI integration, or all of the above? Also, is this for existing rendering code or new functionality?"

**You:** "Framebuffer capture first, for existing rendering"

**Copilot:**
> "Perfect! So we're creating a feature story to capture OpenGL framebuffer output, roughly medium scope (1-2 days). 
> 
> I'm thinking:
> - **Title:** Create Framebuffer Capture Utility for Visual Regression Testing
> - **Value:** Enables automated screenshot comparison in CI pipeline
> - **Key outcomes:** Capture RGBA frames to PPM, handle vertical flip, work in headless mode
> 
> Does that sound right, or should I adjust anything?"

**You:** "Looks good!"

**Copilot:** [generates full story]
> "I've written up the story above. What do you think?
> 
> I can:
> - Break this into smaller subtasks if it's too big
> - Add more specific acceptance criteria
> - Adjust the scope or format
> 
> Or if it looks good, we can call it done!"

---

## Common Requests

### "Make it simpler"
Copilot will reduce scope, simplify Definition of Done, or break it into smaller pieces.

### "Use BDD format"
Switches acceptance criteria to Given/When/Then format.

### "Break this down"
Splits large stories into 3-4 independent smaller stories.

### "Add file paths"
Includes specific files that will be created or modified.

### "Show me an example first"
Loads reference examples from the skill's knowledge base.

---

## Project-Specific Knowledge

The skill knows Particle-Viewer conventions:

- **Naming:** PascalCase for classes (`FramebufferCapture`), snake_case for files (`framebuffer_capture.hpp`)
- **Testing:** GoogleTest in `tests/` directory, headless OpenGL with Xvfb
- **CI:** GitHub Actions with Mesa drivers, no GPU required
- **Architecture:** Elimination of global state, proper layering

You don't need to explain these—the skill will apply them automatically.

---

## Tips for Best Results

### Be Conversational
You don't need to be formal. Just say what you're thinking:
- ✅ "I want to refactor the globals in clutter.hpp"
- ✅ "We need tests but I'm not sure what kind yet"
- ❌ "Generate user story with parameters: type=feature, size=M..."

### Answer What You Know, Skip What You Don't
Don't have a size estimate? That's fine—say so or skip it. The skill will infer sensibly.

### Iterate Freely
The first draft is just a starting point. Ask for changes:
- "Make the Definition of Done shorter"
- "Add a constraint about performance"
- "Actually, this should be two separate stories"

### Use It for Any Story Type
- **Features:** New capabilities or components
- **Refactors:** Improving existing code structure
- **Spikes:** Time-boxed research or investigation
- **Bugs:** Fixing defects or incorrect behavior

---

## References

The skill uses these reference files (loaded on-demand):

- **[INVEST Framework](skills/user-story-generator/references/INVEST_FRAMEWORK.md)** – What makes a good story
- **[Story Templates](skills/user-story-generator/references/STORY_TEMPLATES.md)** – Examples for different story types
- **[Particle-Viewer Context](skills/user-story-generator/references/PARTICLE_VIEWER_CONTEXT.md)** – Project conventions and standards

You don't need to read these—the skill uses them automatically when relevant.

---

## Getting Started

Try it now:

```
Create a story for: Create Framebuffer Capture Utility
```

Copilot will start asking questions, and you'll build the story together!

---

## Creating New Skills

Want to build another Copilot skill for this project? See [SKILL_CREATION_GUIDE.md](SKILL_CREATION_GUIDE.md) for comprehensive guidelines on creating skills that follow the agentskills.io specification, including best practices, anti-patterns to avoid, and testing checklist.
