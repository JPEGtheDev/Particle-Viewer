---
name: brainstorming
description: Use when a task needs design exploration before any implementation begins. Required for any task with unclear approach, significant architecture impact, or multiple valid solutions.
---

## Iron Law

```
NO CODE UNTIL THE DESIGN GATE IS PASSED.
```

Violating the letter of this rule is violating the spirit of this rule.

If you are about to write code and have not answered the design questions below, STOP. Complete this skill first.

**Announce at start:** "I am using the brainstorming skill to explore design options for [brief description]. No code will be written until the gate is passed."

---

## When to Invoke

This skill is required before writing code when ANY of the following are true:

| Signal | Why it matters |
|--------|---------------|
| The approach is unclear or contested | Code written before approach is settled creates rework |
| The task touches 2+ architectural layers | Cross-layer changes have hidden coupling costs |
| Multiple valid solutions exist | Picking without analysis risks the wrong trade-off |
| The user said "figure out the best way" | That is a design task, not an implementation task |
| You feel uncertain about WHERE to put the code | Uncertainty about structure = design question unanswered |
| The task involves a new dependency or library | Dependency choice is a design decision |

---

## HARD-GATE: Design Questions

Answer all applicable questions before writing a single line of production code. If you cannot answer a question, dispatch a research subagent — do not guess.

### Required Questions (all tasks)

1. **What is the simplest thing that could possibly work?**
   State it in one sentence. If you cannot, the problem is not well-understood yet.

2. **What are the top 3 alternative approaches?**
   List them. If only one approach exists, state why the others are ruled out.

3. **What are the trade-offs?**
   For each approach: what does it give, what does it cost? Use the template:
   ```
   Approach A: [benefit] / [cost]
   Approach B: [benefit] / [cost]
   Chosen: [A] because [specific reason the benefit outweighs the cost here]
   ```

4. **Where does this code live?**
   Name the file(s). If you are unsure, that is a gap — resolve it before coding.

5. **What does this NOT do?**
   State the explicit boundary: what is out of scope, deferred, or assumed away.

### Additional Questions (architecture-impacting tasks)

6. **Does this cross a layer boundary?**
   If yes: is the dependency direction correct (outer → inner, never inner → outer)?
   If no: confirm the change is confined to one layer.

7. **What existing code does this interact with?**
   Name the interfaces and concrete classes. If you do not know, dispatch an explore agent.

8. **Can this be tested without a real OpenGL context?**
   If no: why not, and what is the test strategy? (Visual regression? Mock context?)

---

## Design Gate Checklist

Before exiting brainstorming and entering implementation:

- [ ] Simplest approach identified and either chosen or explicitly rejected with reason
- [ ] All trade-offs named — none left implicit
- [ ] Files named — no "somewhere in src/"
- [ ] Out-of-scope items explicitly listed
- [ ] Architecture impact assessed (layer boundaries, dependency direction)
- [ ] Test strategy stated

✓ All checked → proceed to implementation
✗ Any unchecked → return to the relevant design question; do not proceed

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "I know what to do, I'll just start" | If you knew, the design questions would take 2 minutes. Answer them. |
| "We can refactor later" | Later never comes. Design debt compounds. 2 minutes now saves 2 hours later. |
| "It's a small change" | Small changes in the wrong layer cause large regressions. |
| "The approach is obvious" | Obvious approaches still need trade-offs named. That is what makes them defensible. |
| "I'll figure it out as I code" | Exploratory coding without a gate creates uncommittable work. |

---

## Output Format

When brainstorming is complete, output a **Design Decision Record** before switching to implementation:

```
## Design Decision Record

**Problem:** [one sentence]

**Chosen approach:** [one sentence]

**Alternatives considered:**
- [Approach A]: rejected because [reason]
- [Approach B]: rejected because [reason]

**Trade-off accepted:** [what we give up by choosing this approach]

**Files affected:** [list]

**Out of scope:** [list]

**Test strategy:** [how correctness will be verified]

**Gate passed:** YES — proceeding to implementation
```

This record becomes part of the PR description or commit message body.

---

## Red Flags → STOP

If you catch yourself:
- Writing code before completing the gate checklist
- Skipping trade-off analysis because "the answer is obvious"
- Naming "TBD" as a file location
- Treating "we'll know more when we start" as a valid answer

**All of these mean: you have not completed brainstorming. Return to the Design Gate.**
