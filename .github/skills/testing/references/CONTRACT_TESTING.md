# Contract Testing Reference

Source: Ward Cunningham's C2 wiki audit — patterns for writing tests against interfaces and abstract types.

---

## Testing Abstract Base Classes

Write a test fixture for the abstract type using a minimal mock implementation. This test fixture becomes the contract: every concrete subclass must pass it.

```cpp
// Abstract typed fixture — parameterised over each concrete implementation
template <typename T>
class IOpenGLContextTest : public ::testing::Test {
protected:
    void SetUp() override { ctx_ = std::make_unique<T>(); }
    std::unique_ptr<IOpenGLContext> ctx_;
};

TYPED_TEST_SUITE_P(IOpenGLContextTest);

TYPED_TEST_P(IOpenGLContextTest, Clear_DoesNotThrow) {
    EXPECT_NO_THROW(ctx_->Clear(0, 0, 0, 1));
}

REGISTER_TYPED_TEST_SUITE_P(IOpenGLContextTest, Clear_DoesNotThrow);

// Instantiate once per concrete type — all contract tests run automatically:
using GLContextImpls = ::testing::Types<MockOpenGLContext>;
INSTANTIATE_TYPED_TEST_SUITE_P(AllImpls, IOpenGLContextTest, GLContextImpls);
```

`std::unique_ptr` owns the context — no manual `TearDown` required. If any inherited test is inappropriate for a concrete subclass, that signals the behavior does not belong in the base type — redesign the hierarchy.

---

## Two-Phase Composite Operation

Validate all children before executing any. See `cpp-patterns/references/SAFETY_PATTERNS.md — Two-Phase Composite for GL State Safety` for the full C++ example.

Behavioral contract angle: `validate()` must be idempotent and free of side effects. If validation raises an error, no execution has occurred — the system is in its original state. Test `validate()` and `execute()` independently.

---

## Micro-Iteration Test Loop (XP Test FAQ)

One failing test at a time. The loop:

1. Write one assertion (the expected result — before the code exists)
2. Compile → fail (no method)
3. Add stub method → compile
4. Run → fail (assertion fails)
5. Add one line of implementation
6. Compile and run → pass
7. Refactor
8. Repeat

Write assertions **first** (Write Tests Backwards): state the expected outcome before writing setup or implementation. Forces goal-first thinking.

---

## One Active Failing Test

Maintain exactly one active failing test at a time. When a test reveals that several more tests are needed, write their names on a list — do not make them all fail simultaneously. Work through the list one entry at a time.

This prevents analysis paralysis from dozens of simultaneous failures blocking feedback.

---

## Zero-One-Infinity Boundary Rule (Unfit Testing)

For every numeric parameter, test: **zero (or null), one (typical), maximum (or many).**

```cpp
TEST(ParticleBuffer, HandlesEmpty)   { /* 0 particles */ }
TEST(ParticleBuffer, HandlesSingle)  { /* 1 particle  */ }
TEST(ParticleBuffer, HandlesLarge)   { /* 1M particles */ }
```

Skipping boundary tests is the primary source of boundary-condition bugs.

---

## Requirements as Executable Code

Write acceptance criteria as passing tests — not paper documents. Tests are unambiguous and cannot fall out of sync with the implementation. Unit tests express expected behavior; acceptance tests demonstrate working features.

---

## Writing Testable Classes

Decompose large methods into units that can be tested individually. Extract `getTaxRate()` and `calculateTaxPayment()` from `determineTaxPayment()`. Isolate business logic from infrastructure (file I/O, GL state mutations, event dispatch).

In C++: `friend class` declarations for test access are an accepted trade-off when the alternative is making methods public for testing purposes only.

---

## Zero-Button Testing

Automated test runs triggered without manual intervention. Tests run continuously during development; feedback arrives before the developer context-switches away. Goal: zero interruption to flow, minimal latency from change to signal.

In practice: integrate tests into the build target so `cmake --build build` also runs the suite.

---

## Related Skills

- `contract-testing` — iron law: every abstract type requires a contract test fixture
- `testing` — test taxonomy, AAA pattern, naming conventions
- `systematic-debugging` — when tests surface bugs, trace to root cause before patching
