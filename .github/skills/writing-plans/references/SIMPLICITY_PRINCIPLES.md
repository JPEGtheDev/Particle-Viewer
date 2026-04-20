# Simplicity Principles Reference

Source: Ward Cunningham's C2 wiki audit — principles for scoping plans, choosing simple designs, and avoiding speculative complexity.

---

## Beck's Four Rules as a Planning Filter

Before committing to a design or a plan, verify it satisfies these rules in order:

1. **Does it make tests pass?** If the design cannot be verified by a test, question its existence.
2. **Does it eliminate duplication?** Every concept expressed once. Duplicate todos = duplicate implementation.
3. **Does it express intent clearly?** A plan that requires explanation to execute is a plan that will be executed incorrectly.
4. **Does it minimize moving parts?** Fewest classes, files, and dependencies that satisfy the above three.

Cut any todo that cannot be traced to a passing test or a clear requirement.

---

## Work Backward from Pseudo-Code

Before implementation, write the ideal call site in free-form domain language — unconstrained by the production language:

```
// Ideal:
scene.render(particles, camera, lightingModel)
  .withPostProcessing(bloom, toneMapping)
  .saveAs("frame.png")
```

Then work backward to fit the production language (C++20). If the production language requires significant ceremony to express the ideal, the gap reveals a missing abstraction — add it.

Avoids letting language constraints limit design thinking.

---

## YAGNI Applied to Plans

YAGNI governs feature additions — **not** code clarity, refactoring, or structural decisions.

Forbidden in plan todos:
- "We'll probably need this later"
- "Add support for future X"
- "Make it extensible for Y"

Every todo must trace to a specific acceptance criterion. If it cannot, remove it.

---

## When to Use a Pattern (Three Forces Rule)

A pattern is justified only when multiple independent forces are genuinely in tension. If only one force is present, a simpler solution exists.

Ask before applying any design pattern:
1. What forces are in tension? (Name at least two.)
2. Does the pattern resolve those specific forces?
3. Have three real instances of this problem appeared? (Rule of Three)

If any answer is "no" or "not yet" → use the simpler solution.

---

## When to Stop Refactoring

Refactor when:
- Code is unclear and blocking understanding
- Code is duplicated and blocking a new feature
- Adding a new feature requires touching the same logic in multiple places

Stop refactoring when:
- The code no longer smells
- The new feature is unblocked

Do not refactor toward a theoretical canonical form. Do not refactor stable code speculatively. Refactor the path you are walking.

---

## One Active Failing Test

See `testing/references/CONTRACT_TESTING.md — One Active Failing Test` for the full rule.

Planning implication: one active implementation todo at a time. Completing todos serially, with verification at each step, is faster than parallel activation of many unfinished tasks. Stack future test names; do not activate them simultaneously.

---

## Do Not Refactor Stable Code Speculatively

Refactor only when fixing a bug or adding a feature. Working, stable code that does not block forward progress is not a refactoring target — maintenance cost is real, not theoretical.

"Working code trumps everything" is a corollary of YAGNI. The moment a feature need drives a refactor, the refactor is justified. Until then, it is not.

---

## Principle vs Advice vs Pattern (Classification)

Before documenting or applying a software idea, classify it:

| Type | Form | When to use |
|---|---|---|
| Principle | "Always do X" | Universal; applies without context |
| Advice | "If A, do X" | Conditional; context matters |
| Pattern | "To balance forces A, B, C... do X" | Multiple independent forces in genuine tension |

Not every useful idea is a pattern. Forcing a pattern form onto advice dilutes pattern vocabulary. Document each at its correct level.

---

## Simplest Thing That Could Possibly Work (STTCPW)

Before finalizing any design or implementation, ask: "Is there a simpler solution that satisfies all acceptance criteria?" The burden of proof is on the complex option, not the simple one. STTCPW is not "simplest thing that is easy to type" — it is "simplest thing that could pass all the tests." Reject solutions that satisfy criteria not in scope. Source: C2 Wiki "DoTheSimplestThingThatCouldPossiblyWork".

## Dimensions of Simplicity

Kent Beck's ordered checklist for assessing design quality: (1) Passes all tests. (2) Reveals intention — names, structure, and organization communicate what the code does without comment. (3) No duplication — each concept has exactly one home. (4) Fewest elements — no class, method, or variable that does not serve a purpose in criteria 1–3. Apply in order: criterion 1 gates criterion 2, which gates criterion 3, which gates criterion 4. Source: C2 Wiki "XpSimplicityRules".

## Assign Problems, Not Tasks

Delegate outcomes, not procedures. State what success looks like — the verifiable end state — and let the assignee determine the path. A task delegation says "do these five steps." A problem delegation says "the system must do X when Y; verify it." Task delegation creates followers who cannot adapt when the steps do not fit; problem delegation creates owners who can. This is the canonical definition. See `subagent-driven-development/SKILL.md` and `execution/SKILL.md` for project-level application. Source: C2 Wiki "AssignProblemsNotTasks".

## Clear Requirements

A requirement is not clear until it has a verifiable acceptance criterion. "The system should be fast" is not a requirement. "The system must respond within 200ms for 95% of requests under 100 concurrent users" is a requirement. Ambiguous requirements are the most expensive bug category: they are introduced at the design phase and surface at the acceptance phase. Source: C2 Wiki "RequirementsAsTests".

## Big Reduction Up Front

Before planning the work, identify the one simplification that removes the most future work. This is not premature optimization — it is asking "if we change the structure of the problem, how much of the solution disappears?" The question is: what is the highest-leverage simplification available before any code is written? Source: C2 Wiki "BigReductionUpFront".

## Related Skills

- `writing-plans` — YAGNI, PPP, Skeptic Agent gate
- `brainstorming` — Structured Ideation reference for design exploration
- `execution` — Make It Work → Make It Right → Make It Fast gate
