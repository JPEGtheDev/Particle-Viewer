---
name: cpp-safety
description: Use when writing or reviewing any C++ class that owns resources, has a destructor, or acquires in a constructor.
---

## Iron Law

```
DESTRUCTORS NEVER THROW — EVERY RESOURCE IS OWNED BY A SCOPE-BOUND GUARD
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST wrap every destructor body in try/catch and ensure every resource acquisition is handed to an owning guard before the next acquisition begins. No exceptions.

**Announce at start:** "I am using the cpp-safety skill to review [class] for destructor and constructor safety."

---

## BEFORE PROCEEDING

- [ ] Does this class own heap memory, OpenGL handles, file handles, or other operating system resources?
- [ ] Does its destructor contain code that can fail or throw?
- [ ] Does its constructor acquire more than one resource?

✓ No owned resources → skip this skill
✗ Any owned resource → apply the rules below before shipping

---

## Destructor Rule

Throwing from a destructor during stack unwinding calls `std::terminate`. No other destructors in the call stack will run.

```cpp
MyClass::~MyClass() {
    try {
        cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Cleanup failed: " << e.what() << "\n";
        // never rethrow — a destructor that throws during unwinding terminates the program
    }
}
```

## Constructor Rule

If the constructor acquires resource A then throws while acquiring resource B, resource A leaks — the destructor is never called on a partially-constructed object. Each acquisition must be wrapped in its own scope-bound owning guard (a local object whose destructor releases it) before the next acquisition begins.

**Scope-bound resource ownership** means each resource has exactly one object responsible for releasing it, and that object's lifetime is tied to a scope — not a raw pointer. When the owning object goes out of scope, the resource is released automatically, including on exception paths.

See `references/SAFETY_PATTERNS.md` for scope-bound ownership patterns, the Non-Virtual Interface idiom, and Command-Query Separation in OpenGL code.

---

## Rationalization Prevention

| Excuse | Reality |
|---|---|
| "The cleanup is simple, it won't throw" | Destructors that cannot throw today acquire that property for all future edits. Wrap it now. |
| "`std::terminate` is acceptable here" | It is not acceptable during stack unwinding. It prevents all remaining destructors from running. |
| "The second allocation almost never fails" | "Almost never" is not a safety guarantee. Wrap in a scope-bound guard. |
| "Owning guards add boilerplate" | The boilerplate is the guarantee. Inline cleanup without an owning guard is a future leak. |
| "I'll add the try/catch if it causes a problem" | The problem is a silent crash during exception handling. It will not be obvious the destructor caused it. |

---

## Related Skills

- [`cpp-patterns`](.github/skills/cpp-patterns/) — parent skill; OpenGL smell catalog, DRY patterns, and Non-Virtual Interface idiom live there
- [`oop-principles`](.github/skills/oop-principles/) — sibling skill; resource-owning types with inheritance must also pass the Is-A / Has-A gate
- [`systematic-debugging`](.github/skills/systematic-debugging/) — sibling skill; use when a crash or leak points to destructor or constructor failure
