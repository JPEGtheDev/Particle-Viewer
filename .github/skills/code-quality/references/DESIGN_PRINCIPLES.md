# Design Principles Reference

Source: Ward Cunningham's C2 wiki audit — principles for writing and evaluating clean code.

---

## Beck's Four Rules of Simple Design (in priority order)

1. **Tests pass** — all tests green; no broken behavior
2. **No duplication** — Once and Only Once; every concept expressed exactly once
3. **Expresses ideas clearly** — names and structure reveal intent; reader does not need comments to understand
4. **Minimizes classes and methods** — no unnecessary abstractions; fewest elements that satisfy the above three

The order is fixed. Do not minimize classes at the expense of clarity. Do not eliminate duplication at the expense of passing tests.

---

## Simply Understood Code

Code is simply understood when:
- It fits on one screen without scrolling
- Variables are assigned once (no mutation chains)
- Reading order is linear top-to-bottom
- Names are unambiguous in their immediate context

The goal of MeaningfulNames is that code reads as an executable description of what it does.

---

## Write Your Code Like Prose

Nouns name objects. Verbs name methods with side effects. Adjectives name decorators. Method names read as sentences in context:

```cpp
bool deviceIsPoweredOn();           // boolean accessor reads as predicate
void takeItemsFrom(int n, List& l); // verb + preposition signals ownership transfer
window.moveTo(targetPoint);         // subject.verb(object)
```

When a method name requires a preposition ("from", "to", "with") or a conjunction ("and", "or"), it may signal that the behavior belongs in a different class (feature envy) or that the method is doing too much.

---

## Using Good Naming to Detect Bad Code

Method names that require prepositions or conjunctions are signals, not style choices:

- `applyUpdatesTo(Account)` → the method belongs on `Account`
- `getAddressesFrom(Message)` → feature envy; `Message` should provide its own addresses
- `validateAndSave()` → two responsibilities; extract two methods

Rename first. The right name often reveals the right design.

---

## Succinctness Reveals Inadequate Abstractions

When you cannot write terse code at the call site, the problem is the abstraction layer, not the calling code. Refactor the abstraction until calling code reads simply. Succinctness is an economic indicator — it signals that the abstraction is doing its job.

---

## Refactoring: What It Is and When to Stop

Refactoring is continuous, small-step reorganization that preserves behavior. It is not rewriting. It is safe only when tests catch mistakes.

**When to refactor:** when code is unclear, duplicated, or blocking a new feature.
**When to stop:** when the code no longer smells and the new feature is unblocked. Stop there — not at a theoretical canonical form.

Do not refactor stable, working code speculatively. Refactor the path you are walking, not hypothetical future paths.

---

## Well-Written Code

Well-written code is correct **and**: well-tested, well-formatted, well-factored, and well-commented. All five properties are required. Source code is a communication medium between developers — not a set of instructions for a compiler.

---

## Weinberg Test for Long Functions

To evaluate whether a function is too long: memorize a chunk of it, close the file, rewrite it from memory. If mistakes occur, the chunk is too long or too complex. This operationalizes the 7±2 cognitive load principle as a concrete test.

---

## YAGNI Applies to Features, Not Cleanliness

YAGNI (You Are Not Going to Need It) governs feature additions — not refactoring, not naming, not structural clarity. Once and Only Once and YAGNI sit at opposite ends of a continuum. Refactoring moves code between them. Clean structure is foundational, not optional.

---

## Related Skills

- `code-quality` — clang-format, naming conventions, smell checklist, pre-commit gate
- `cpp-patterns` — C++-specific idioms for these principles
- `writing-plans` — Simplicity Principles reference for planning with YAGNI
