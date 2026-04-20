# Execution Patterns Reference

Source: Ward Cunningham's C2 wiki audit — patterns for effective implementation execution, optimization discipline, and performance management.

---

## Profile Before Optimizing

Write code for clarity, flexibility, and correctness first. Optimize only after measurement proves it necessary. "I think this is slow" is not a profiling baseline.

The three rules of optimization:
1. Don't do it.
2. (For experts only) Don't do it yet.
3. Profile first, then optimize the measured bottleneck — not the guessed one.

This principle extends to any resource: memory allocation, network round-trips, disk access. The pattern of premature optimization is: implement something slower for clarity, observe hypothetical slowness, optimize without measuring, introduce complexity, discover the bottleneck was elsewhere. Source: C2 Wiki "ProfileBeforeOptimizing", "RulesOfOptimization".

---

## Assign Problems, Not Tasks

When delegating to a subagent or a collaborator, provide the problem and its constraints — not the implementation steps. An executor given steps will follow them even when the steps lead to the wrong outcome. An executor given the problem can recognize when circumstances require a different approach.

The distinction:
- Task assignment: "Add a pointer line to SKILL.md, then append the new section to the reference file, then commit."
- Problem assignment: "The SKILL.md needs a pointer to the new reference file for Six Thinking Hats. The file is over its word limit — use the minimum change that achieves the goal."

The first fails when the file has already been modified. The second adapts. Source: C2 Wiki "AssignProblemsNotTasks".

---

## Technical Debt Visibility

Technical debt is the cost of rework that accumulates when expedient solutions are chosen over correct ones. The debt itself is not always wrong — sometimes a quick solution is correct for the moment. The danger is invisible debt: shortcuts taken without recording that a shortcut was taken.

Rule: every deliberate shortcut must be marked at the decision point. Include: what the shortcut is, what the correct solution would be, and why the shortcut was chosen now. An unmarked shortcut becomes permanent. A marked one can be scheduled.

Do not use technical debt as a rationalization for low-quality work. Use it only to describe explicitly accepted trade-offs with a documented intent to revisit. Source: C2 Wiki "TechnicalDebt", "WardExplainsDebtMetaphor".

---

## Related Skills

- `execution` — work loop, commit rhythm, mode declaration
- `writing-plans` — SIMPLICITY_PRINCIPLES.md for Assign Problems Not Tasks; YAGNI gate
- `systematic-debugging` — Profile Before Optimizing applies equally to debugging: measure before concluding
