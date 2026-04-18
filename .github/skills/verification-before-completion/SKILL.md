---
name: verification-before-completion
description: Use before claiming any work is complete, fixed, or passing. Required before every commit and PR. Evidence before assertions, always.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.0"
  category: verification
  project: Particle-Viewer
---

# Instructions for Agent

## How This Skill is Invoked

This skill is **mandatory before any completion claim**. It applies before:
- Saying "tests pass", "build succeeds", "bug is fixed", "feature complete", or "PR is ready"
- Opening a commit message
- Opening a pull request

When activated, announce: **"I am using the verification-before-completion skill. Running verification now."**

---

## Iron Law

```
NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION EVIDENCE
```

If you haven't run the verification command in this session — after your most recent change — you cannot claim it passes. Evidence before assertions, always.

---

## The Gate Function

Apply this BEFORE any completion claim or expression of satisfaction:

```
BEFORE claiming any status or expressing satisfaction:
1. IDENTIFY: What command proves this claim?
2. RUN: Execute the FULL command (fresh, in this session, after the latest change)
3. READ: Full output — check exit code, count failures, read warnings
4. VERIFY: Does output confirm the claim?
   - If NO: State actual status with evidence
   - If YES: State claim WITH evidence inline
5. ONLY THEN: Make the claim
```

Skipping any step = lying, not verifying.

---

## The Verification Commands

```bash
# Build
cmake --build build

# Tests
./build/tests/ParticleViewerTests

# Format check
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run -Werror

# Full pre-commit gate (run all three before every commit)
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
cmake --build build && ./build/tests/ParticleViewerTests
```

---

## Common Failure Modes

| Claim | Requires | Not Sufficient |
|-------|----------|----------------|
| "Tests pass" | Test command output: 0 failures | Previous run, "should pass", CI green |
| "Build succeeds" | cmake output: exit 0 | Linter passing, "looks right" |
| "Formatted correctly" | `clang-format --dry-run`: 0 errors + `git diff` inspected | "I ran clang-format" without checking diff |
| "Bug is fixed" | Test reproducing original bug: passes | Code changed, assumed fixed |
| "PR is ready" | All three gate commands: pass | Tests passing only |
| "Feature complete" | Every acceptance criterion verified | Tests passing |

---

## Red Flags — STOP

If you find yourself thinking any of the following, you are about to make an unverified claim. **STOP. Run the verification commands. Then state your claim.**

- "Should work now"
- "Probably passes"
- "I'm confident it's right"
- "I ran it earlier this session" — earlier ≠ after the current change
- "The build was clean before my change"
- "CI will catch anything I missed" — CI is a safety net, not your verification
- "Just a small change, can't have broken anything"
- Expressing satisfaction ("Great!", "Done!", "That should do it!") before running commands
- About to write a commit message without having run the gate commands

**All of these mean: Run the verification commands NOW. Then state your claim.**

---

## Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "Should work now" | RUN the verification. |
| "I'm confident" | Confidence ≠ evidence. |
| "Small change, can't break things" | Small changes cause subtle failures. |
| "I ran it earlier" | Earlier run does not cover current changes. |
| "CI will catch it" | CI is a safety net, not your job. |
| "Just this once" | No exceptions. |
| "I already manually verified" | Manual ≠ automated. Results can't be reproduced or cited. |
| "Build passed, tests must be fine" | Build and tests are separate gates. |

---

## Integration

- After debugging: always verify with this skill before claiming the fix worked
- Before every commit: run the full pre-commit gate
- Before opening a PR: run all three verification commands and read the output
- See `systematic-debugging` skill for how to investigate failures found during verification
