# Structured Ideation Reference

Source: Ward Cunningham's C2 wiki audit — techniques for design exploration, collaborative critique, and avoiding premature convergence.

---

## Work Backward from Ideal (Pseudo-Code First)

Start every design session by writing the ideal interface in free-form domain notation — before any production language constraints apply:

```
// Ideal API (unconstrained):
particles.simulate(dt).under(gravity, drag).clampTo(bounds)
```

Then work backward to production code. The gap between the ideal and what the language allows reveals where abstractions are missing. This technique prevents language constraints from limiting design thinking.

---

## Vague vs Abstract (Dijkstra)

Abstraction ignores irrelevant detail but can answer precise questions at the level of the abstraction. Vagueness cannot answer questions at any level.

> "The purpose of abstraction is not to be vague, but to create a new semantic level in which one can be absolutely precise." — Dijkstra

Before finalizing an API or interface, test it: can every caller question be answered precisely from the abstraction? If not, the abstraction is vague, not abstract. Redesign.

---

## Visitor from Mars (Fresh-Eye Review)

When a new contributor joins, formally designate them as a "Visitor from Mars" — an outside observer who has not yet become blind to friction points. Task them with documenting:

- Places where the code surprised them
- Conventions that were not written down
- Design decisions that required asking a teammate to understand

Capture this feedback before the new contributor "goes native" and stops noticing friction. The window is typically the first two weeks.

Apply at code review: ask reviewers to note anything that required rereading.

---

## Writers Workshop Pattern (Structured Critique)

Adapt the literary writers' workshop format for design and code review:

1. **Safe setting:** establish that critique targets the work, not the author
2. **Author is silent:** the author observes while others discuss; does not defend
3. **Summarize first:** one reviewer restates the work's intent in their own words — confirms understanding before critique
4. **Positive feedback first:** state what works well and why
5. **Suggestions for improvement:** frame as questions ("I wondered if...") not directives
6. **Author responds last:** addresses clarifications only, not defenses

The goal is learning, not verdict. This format reduces defensive responses and surfaces signal that direct critique suppresses.

---

## Interaction Design: Minimize Undesirable Interactions

Design is the process of finding a minimal concept set where desirable interactions are maximized and undesirable interactions are minimized. Every additional user choice or configuration option is a **cost**, not a benefit — it adds an interaction surface that must be tested and maintained.

Before adding a feature or a parameter, ask: does this eliminate a worse interaction, or does it add a new one?

---

## When Not to Use Design Patterns

Patterns are workarounds for language limitations that disappear in more expressive languages. Apply a pattern only when:
- At least two independent forces are genuinely in tension
- A simpler solution does not resolve the same forces
- Three real instances of the problem have appeared (Rule of Three)

Forcing a pattern onto code that does not need it adds indirection with no payoff. "Using patterns because they're in the book" is itself an anti-pattern.

---

## Evolutionary Design (XP Model)

XP does not eliminate design — it distributes design throughout development:

- **Day 1:** establish a system metaphor (4-5 key classes, a brief description)
- **Per capability:** run a Class-Responsibility-Collaboration session (~4 new classes at a time)
- **Continuously:** code experimentally to "sense what objects want to do"; refactor toward the emerging structure

The source code is the design. Minimal artifacts, maximum feedback. Long feedback cycles (months) allow bad design to calcify; short cycles (days) keep design malleable.

---

## Wiki / Knowledge Base Design Principles (Ward Cunningham)

For any collaborative knowledge base (skills, documentation, wiki):

- **Simple:** low friction to contribute; no complex markup required
- **Observable:** changes are visible; diffs are readable
- **Convergent:** duplication is recognized and removed over time
- **Tolerant:** imperfect contributions are welcome; the system improves them
- **Precise:** titles are unambiguous; content answers the question the title asks

Anti-patterns: ThreadMode prose (unresolved debate left in the document), WalledGardens (information only accessible to insiders), VerbalDiarrhea (more words than content).

---

## Related Skills

- `brainstorming` — hard gate: load before any design with unclear approach
- `writing-plans` — Simplicity Principles reference; YAGNI; Skeptic Agent
- `architecture-review` — OOP Principles reference; layer boundary gate
