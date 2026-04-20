---
name: oop-principles
description: Use when adding inheritance, designing interfaces, or reviewing any class hierarchy for Particle-Viewer.
---

## Iron Law

```
CHECK IS-A / HAS-A AND SOLID BEFORE APPROVING ANY CLASS HIERARCHY
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST run the Is-A / Has-A gate and verify all five SOLID principles before adding or approving any inheritance relationship. No exceptions.

**Announce at start:** "I am using the oop-principles skill to review [class/hierarchy]."

---

## BEFORE PROCEEDING

- [ ] Is this a new `class B : public A` relationship?
- [ ] Does B pass a full substitution test — can any code accepting A accept B without behavioral change?
- [ ] Does the proposed hierarchy satisfy all five SOLID principles?

✓ All met → proceed
✗ Any unmet → use composition or redesign before adding the inheritance

---

## Is-A / Has-A Gate

Ask before any inheritance: "Does B truly **is-a** A in all contexts, or does B merely **have** some of A's behaviors?"

If the answer is "has," use composition. Inheritance creates permanent coupling. Composition allows each piece to vary independently.

## SOLID Quick Check

| Principle | Gate question |
|---|---|
| Single Responsibility | Does this class have one reason to change? |
| Open / Closed | Does new behavior require editing this class, or extending it? |
| Liskov Substitution | Can B replace A everywhere without behavioral surprise? |
| Interface Segregation | Do all clients use all methods of this interface? |
| Dependency Inversion | Does the high-level module depend on an abstraction, not a concrete? |

See `references/OOP_PRINCIPLES.md` for full principle definitions, violation signals, Template Method vs Strategy, Principle of Least Authority, and YAGNI for generality.

---

## Rationalization Prevention

| Excuse | Reality |
|---|---|
| "Inheritance is the natural model here" | Natural is not the same as correct. Run the Is-A test. |
| "The base class is just for code reuse" | Code reuse is composition's job. Inheritance is for substitutability. |
| "Only one interface method is unused by this client" | That is an Interface Segregation violation. Split the interface. |
| "We'll need to extend this later" | Open/Closed means extension via new code, not anticipating extension by weakening constraints. |
| "The substitution test passes today" | Does it pass under all documented invariants, including error semantics and state preconditions? |

---

## Related Skills

- [`architecture-review`](.github/skills/architecture-review/) — parent skill; layer boundary rules and dependency direction apply to any hierarchy this skill reviews
- [`contract-testing`](.github/skills/contract-testing/) — sibling skill; every interface approved by this skill requires a contract test fixture
- [`cpp-safety`](.github/skills/cpp-safety/) — sibling skill; any hierarchy involving resource-owning types must also satisfy destructor and constructor safety rules
