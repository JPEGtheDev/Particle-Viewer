---
title: "Integration Testing: Mocking and Troubleshooting"
description: "MockOpenGL setup, coverage considerations, and troubleshooting for integration tests."
domain: testing
subdomain: integration-testing
tags: [testing, integration-testing, gtest, mocks, c++]
related:
  - "integration-tests.md"
  - "integration-tests-patterns.md"
  - "../TESTING_STANDARDS.md"
---

# Integration Testing: Mocking and Troubleshooting

## MockOpenGL Setup

Integration tests use `MockOpenGL` to intercept OpenGL calls without a real GPU context.

Always call both `reset()` and `initGLAD()` in `SetUp()`:

```cpp
void SetUp() override
{
    MockOpenGL::reset();
    MockOpenGL::initGLAD();  // Required — do not omit
}
```

`reset()` clears call counters and state. `initGLAD()` installs the mock function pointers so OpenGL calls route to the mock instead of the driver.

### Mock State Helpers

```cpp
MockOpenGL::setCompileStatus(GL_TRUE);   // Make shader compilation succeed
MockOpenGL::setLinkStatus(GL_TRUE);      // Make program linking succeed
```

Call counters (e.g., `MockOpenGL::compileShaderCalls`) are reset to zero by `reset()` and incremented on each matching GL call.

## Coverage Considerations

Integration tests should **not** be counted towards code coverage metrics.

**Rationale:**
- Integration tests exercise paths across multiple components and inflate coverage metrics
- Coverage should measure how well **unit tests** exercise individual components
- Counting integration tests can mask gaps in unit test coverage

Currently all tests contribute to coverage reports. The intended future improvement is to generate separate coverage reports for unit tests only.

## Troubleshooting

### Test Data Files Not Found

Check that `SetUp()` creates required files and `TearDown()` cleans them up. Verify paths match what the test fixture is passing to `SettingsIO`.

### Segfault on GL Calls

`MockOpenGL::initGLAD()` was not called. Add it to `SetUp()`.

### Static Const Linker Errors

Use an enum instead of `static const` for integer constants in test fixtures:

```cpp
// Avoid:
static const int NUM_PARTICLES = 50;

// Prefer:
enum { NUM_PARTICLES = 50 };
```

### Tests Pass Alone but Fail Together

Each test is modifying shared state. Ensure `MockOpenGL::reset()` is called in `SetUp()` and that test fixtures create fresh data — never share state across tests.

### Coverage Report Missing Integration Tests

Integration test **files** do not need coverage — the source under test is in `src/`. Coverage should report on `src/*.hpp` and `src/*.cpp`, not the test files themselves.

## Debugging Tips

1. **Isolate with filter**: `--gtest_filter="SuiteName.TestName"`
2. **Verbose output**: `--gtest_verbose=1`
3. **Add debug output** to `createTestData()` to verify file creation
4. **Check mock state**: Add a `MockOpenGL::reset()` at the top of the specific test if you suspect state leak

## Related

- [Integration Tests Overview](integration-tests.md) — Running tests and directory structure
- [Test Patterns](integration-tests-patterns.md) — Fixture, AAA patterns, and adding new tests
- [Testing Standards](../TESTING_STANDARDS.md) — Project-wide test guidelines
