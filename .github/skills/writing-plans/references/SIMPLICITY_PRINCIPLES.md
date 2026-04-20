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

During implementation, maintain exactly one failing test at a time. When a test reveals that more tests are needed, write their names in a list — do not activate them all simultaneously. Work through the list one entry at a time.

Planning implication: one active implementation todo at a time. Completing todos serially, with verification at each step, is faster than parallel activation of many unfinished tasks.

---

## Working Code Is Not Optional to Refactor

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

## Related Skills

- `writing-plans` — YAGNI, PPP, Skeptic Agent gate
- `brainstorming` — Structured Ideation reference for design exploration
- `execution` — Make It Work → Make It Right → Make It Fast gate
