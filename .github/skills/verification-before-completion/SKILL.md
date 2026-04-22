---
name: verification-before-completion
license: MIT
description: Use when about to claim work is complete, fixed, or passing, before any completion claim, commit, or PR.
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
YOU MUST RUN VERIFICATION COMMANDS IN THIS SESSION BEFORE ANY COMPLETION CLAIM.
EVIDENCE MUST BE INLINE. NEVER REFERENCED.
No exceptions.
```

Violating the letter of this rule is violating the spirit of this rule.

**Announce at start:** "I am using the verification-before-completion skill to verify [work item] before claiming completion."

---

## The Done Definition Problem

**"Done" is the most overloaded word in software development.** Every misuse is a false completion claim.

| What you finished | Correct vocabulary | NOT "Done" |
|-------------------|--------------------|------------|
| Unit tests pass locally | "Verified locally, ready for pipeline" | ~~Done~~ |
| CI green on branch | "CI green, ready for acceptance review" | ~~Done~~ |
| Staging deployed and tested | "Staging verified, pending production" | ~~Done~~ |
| Production deployed, monitored, working | **"Done"** (the only correct use) | — |

**Gate:** Marking a todo "done" when unit tests pass = wrong definition. Invisible work remains.

For this project (a desktop application, not a deployed service), the equivalent stages are:
- "Locally verified" — builds + tests pass after the change
- "Gate passed" — full pre-commit gate (build + tests + format + diff review) completed
- "Committed" — code committed with passing gate
- **"Done"** — committed, PR merged, branch clean

Use stage vocabulary. Never use "Done" for a stage that is not the final stage.

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

✓ All met → proceed
✗ Any unmet → return to step 1; do not make the claim
```

Skipping any step = lying, not verifying.

---

## 4 Cores Final Check

The verification gate confirms **Results** — tests pass, build succeeds. But it cannot confirm the first three cores. Run this check *after* the gate, before claiming done.

| Core | Question | What it catches |
|------|----------|-----------------|
| **Integrity** | Does the implementation match exactly what was committed — no more, no less? | Scope creep, partial delivery, undisclosed limitations |
| **Intent** | Am I solving the stated problem, or the comfortable version of it? | Solving an easier adjacent problem; avoiding the hard requirement |
| **Capabilities** | Is this the right solution, or just the first one that worked? | Technically-passing implementation that creates future maintenance burden |
| **Results** | Can I show verifiable inline evidence? | Covered by the gate above — cite it explicitly |

**Failing any core = do not claim done. Resolve the gap first.**

The Results core is covered by the verification commands above. The first three are quality checks that evidence alone cannot catch — a test suite can pass while the wrong problem is solved perfectly.

**Worst common violation — Intent:** An agent solves the easy part of a requirement (the part it's comfortable with) and presents it as the full solution. The tests pass. The build is clean. The requirement is not met. The 4 Cores check catches this by asking "did I solve what was asked, or what was convenient?"

---

## Combine Verification Instruments

**No single verification technique removes more than ~70% of defects.** (Capers Jones research, cited in McConnell's *Code Complete*.)

| Technique | Defect removal efficiency |
|-----------|--------------------------|
| Unit testing alone | 30–40% |
| Code inspection/review | 45–70% |
| **Both combined** | **>90%** |

The IBM Cleanroom result: formal inspections achieved <0.1 defects/KLOC vs. industry average 15–50.

**The implication:** "Tests pass" with no additional review still has a statistically likely defect rate of 30–70%. Tests catch execution-time defects. Code review catches logical, requirements, and interface defects that tests cannot surface.

**Gate for this project:**

Before claiming any PR ready:
1. **Tests pass** — execution-time defects caught
2. **Diff reviewed** — logical and interface defects caught (`git diff` read hunk by hunk)
3. **Format verified** — `clang-format --dry-run` clean
4. **Scope verified** — changes touch only what the task required; no accidental changes

**Rationalization:** "Tests pass — I'm done." Counter: no single technique removes >70% of defects. Tests + diff review is the minimum combined instrument set.

---

## Pre-PR Checklist (PRR)

Before creating any PR — answer all 6 questions:

| # | Question | Why it matters |
|---|----------|----------------|
| 1 | **Blast radius:** Are the files touched the minimum necessary? | Excess scope = unintended side-effects |
| 2 | **Monitoring:** Does CI detect regressions in the new behavior? | New behavior with no test = invisible breakage |
| 3 | **Failure mode:** Does this fail loudly (error/exception) or silently? | Silent failures require active monitoring to catch |
| 4 | **Rollback path:** Can this be reverted with `git revert`? | Irreversible changes need extra review |
| 5 | **Dependency appropriateness:** Are you connecting to the right components? | Architectural violations that pass tests |
| 6 | **Open issues:** Are there P0/P1 open action items in this area? | Shipping over an unresolved incident |

A "no" or "unknown" on any item = resolve it or document it explicitly in the PR description before opening.

---

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
✅ `"Ran <project-test-runner>: **247 passed, 0 failures.** [exit 0]"`

The inline format is self-auditing. The user cannot independently verify a reference claim. They can verify inline output by reading it.

### Process Language (always available)

Use freely when verification hasn't been run:
- `"Running verification now..."`
- `"Haven't run the gate yet — doing that now"`
- `"Identified likely cause — confirming before claiming fix"`
- `"Uncertain about X — dispatching subagent to confirm"`

---

## The Verification Commands

> **Note (Particle-Viewer specific):** The commands below use this project's build system and test runner. Adapt `cmake --build build`, `./build/tests/ParticleViewerTests`, and the `find src tests` glob for your own project.

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
- Verified 1 of N parallel edits (N ≥ 3) — **STOP. View at least 3 of the N edited files. "They all look the same" is an assumption, not evidence. A malformed edit still counts as a changed file.**

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
| "They all follow the same pattern" | If the pattern was wrong once, it was wrong for all N. For N ≥ 3 parallel edits of the same structure: view at least 3 of the edited files before committing — not 1. |

---

## Integration

- After debugging: always verify with this skill before claiming the fix worked
- Before every commit: run the full pre-commit gate
- Before opening a PR: run all three verification commands and read the output
- See `systematic-debugging` skill for how to investigate failures found during verification

---

## Agent Delegation Verification

When a subagent reports that it completed work, **do not propagate its claim without verifying.**

Subagents can:
- Report success on work that was only partially done
- Fail silently (exit 0 with no actual file changes)
- Write to the wrong path (worktree vs. main working tree)

**The delegation verification gate:**

```
Agent reports: "Done" / "Complete" / "Fixed" / "Committed"
    ↓
1. Check VCS diff — do the changes exist?
   git diff HEAD  OR  git -C [worktree-path] diff HEAD
    ↓
2. Verify changes match the stated intent
   Read the diff — do the files changed match what was requested?
    ↓
3. Run verification commands on the output
   cmake --build build && ./build/tests/ParticleViewerTests
    ↓
4. ONLY THEN claim the subagent's work is complete
```

✓ All steps passed → claim the subagent's work is complete
✗ Any step failed → do not relay the completion claim; investigate and fix before claiming done

**Gate rule:** An agent's "Done" claim is a hypothesis. Your verification makes it a fact.

| Agent claim | Your response |
|-------------|---------------|
| "I committed [X]" | `git log --oneline -5` — does the commit exist? |
| "I updated the file" | `git diff HEAD [file]` — are the changes present? |
| "All tests pass" | Run `./build/tests/ParticleViewerTests` yourself |
| "I created the skill" | `ls .github/skills/[skill-name]/SKILL.md` — does it exist? |

**Never relay a subagent's completion claim to the user without first running this gate.**

---

## Research and Evaluation Gate

Before claiming any test result, evaluation finding, or research conclusion is valid, answer these three questions. If any answer is no — stop. The result is not valid.

```
BEFORE claiming any test result or evaluation finding:
1. BASELINE: Is there a control condition? What does the system do WITHOUT the treatment?
2. VALIDITY: Does the test measure what it claims to measure — or only that the subject can follow instructions directly in front of it?
3. SKEPTIC: Has a Skeptic reviewed the test design BEFORE results were interpreted?
   If NO: dispatch a Skeptic now. Do not interpret results from an unreviewed design.
```

| Research claim | Requires | Not sufficient |
|---|---|---|
| "Model follows skill gate" | Control run (no skill) showing gate violation + treatment run showing gate compliance | Treatment run alone |
| "Skill content causes behavior change" | Control + treatment + placebo (same-length irrelevant doc) | Control + treatment only |
| "Test methodology is valid" | Skeptic review before agents dispatched | Skeptic review after results collected |
| "Gate compliance confirmed" | Tool-call telemetry showing Skill tool invoked first | Announcement text in output alone |

**This gate is not only for software tests.** It applies to any investigation where you are drawing a conclusion from observed behavior — including agent runs, manual experiments, and informal evaluations.
