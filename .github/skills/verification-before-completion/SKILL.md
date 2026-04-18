---
name: verification-before-completion
description: Use before claiming any work is complete, fixed, or passing. Required before every commit and PR. Evidence before assertions, always.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.1"
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
NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION EVIDENCE.
EVIDENCE MUST BE INLINE. NEVER REFERENCED.
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

## The Confidence Vocabulary Gate

### Banned Without Evidence

These words and phrases **cannot appear in any response** unless fresh verification output is shown inline:

| Phrase | Why it fails |
|--------|-------------|
| `"Done"` / `"Complete"` / `"Fixed"` | Completion claims without verification are false confidence |
| `"Works"` / `"Working"` | Shows output proving it works, or use process language |
| `"Tests pass"` / `"Build succeeds"` | Show the command output, not the claim |
| `"I'm confident"` / `"I'm sure"` | Confidence is not evidence |
| **`"Should work"`** | **BANNED. No substitute. Run the verification.** |
| `"That should do it"` | BANNED. Same reason. |

### Why "Should Work" Is Banned

"Should work" combines the tone of having verified with the reality of not having verified. It is undetectable false confidence. A user reading "should work" cannot tell whether you ran the gate or not. Replace it with either:
- Evidence: `"Ran [command]: [output]. It works."`
- Honest uncertainty: `"Haven't verified yet — running the gate now."`

### Required Evidence Format

Evidence must be **inline**, not referenced:

❌ `"I ran the tests and they passed."`  
✅ `"Ran ./build/tests/ParticleViewerTests: **247 passed, 0 failures.** [exit 0]"`

The inline format is self-auditing. The user cannot independently verify a reference claim. They can verify inline output by reading it.

### Process Language (always available)

Use freely when verification hasn't been run:
- `"Running verification now..."`
- `"Haven't run the gate yet — doing that now"`
- `"Identified likely cause — confirming before claiming fix"`
- `"Uncertain about X — dispatching subagent to confirm"`

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

## The Trust Ledger

Every session has a trust balance. This determines whether the user can lean on your outputs without second-guessing them — which determines speed.

| Deposits (earn trust → enable speed) | Withdrawals (trust tax → force verification overhead) |
|--------------------------------------|------------------------------------------------------|
| Verified claim with inline evidence | Any "should work" used without running the gate |
| Finding a failure before the user does | Fix that doesn't address root cause |
| `"I don't know — dispatching subagent"` | Empty output treated as success |
| Delivering exactly what was committed | Completion claim followed by "oh, also there's X" |
| Inline output matches what was stated | Expressing satisfaction before running commands |

**High trust = user acts on outputs directly. Low trust = user re-runs every command themselves.**

Every withdrawal forces the user into verification mode for the rest of the session. It is not the individual mistake that costs — it is losing the ability to move fast.

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
