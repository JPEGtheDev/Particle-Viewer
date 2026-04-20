---
name: contract-testing
description: Use when writing tests for any interface, abstract base class, or type with multiple implementations.
---

## Iron Law

```
EVERY ABSTRACT TYPE REQUIRES A CONTRACT TEST FIXTURE
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST write a contract test fixture before shipping any interface with multiple implementations. No exceptions.

**Announce at start:** "I am using the contract-testing skill to write contract tests for [interface]."

---

## BEFORE PROCEEDING

- [ ] Is this an interface, abstract class, or type with 2+ implementations?
- [ ] Does a contract test fixture exist for it?
- [ ] Does every concrete implementation pass all contract tests?

✓ All met → proceed
✗ Any unmet → write the contract test fixture before adding any new implementation

---

## What a Contract Test Is

A contract test describes the behavioral invariants all implementations must satisfy. Violating a contract test violates the Liskov Substitution Principle.

```cpp
class IOpenGLContextContractTest : public ::testing::Test {
protected:
    virtual std::unique_ptr<IOpenGLContext> createContext() = 0;
    void SetUp() override { context_ = createContext(); }
    std::unique_ptr<IOpenGLContext> context_;
};

TEST_F(IOpenGLContextContractTest, Clear_DoesNotThrow) {
    EXPECT_NO_THROW(context_->Clear(0, 0, 0, 1));
}
```

Each concrete implementation runs the full contract fixture. A failing contract test means the hierarchy is wrong — fix the hierarchy, not the test.

See `references/CONTRACT_TESTING.md` for full patterns and examples.

---

## Rationalization Prevention

| Excuse | Reality |
|---|---|
| "Integration tests cover the contract" | Integration tests verify composition, not behavioral invariants. Contract tests are explicit. |
| "There is only one implementation" | Write the fixture when the interface is defined. A second implementation arrives later. |
| "The interface is simple, nothing to test" | Simple interfaces still have invariants (no-throw, non-null return, state preconditions). |
| "The mock already tests the behavior" | Mocks verify interactions, not behavioral contracts. Both are needed. |
| "I'll add contract tests when a bug appears" | A contract test prevents the bug. Adding it after means the protection was absent when it was needed. |

---

## Related Skills

- [`testing`](.github/skills/testing/) — parent skill; Test Doubles taxonomy, the "saw the test fail" gate, and the AAA naming pattern all apply here
- [`oop-principles`](.github/skills/oop-principles/) — sibling skill; contract tests directly enforce Liskov Substitution — run the Is-A / Has-A gate first
- [`architecture-review`](.github/skills/architecture-review/) — sibling skill; interfaces with contract tests must also pass layer boundary review
