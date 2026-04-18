---
name: execution
description: Autonomous execution protocol for implementing tasks, stories, and bug fixes. Governs planning, subagent use, verification, elegance, and self-correction. Use when executing any non-trivial implementation work.
license: MIT
compatibility: Designed for GitHub Copilot and similar AI coding agents
metadata:
  author: JPEGtheDev
  version: "1.8"
  category: execution
  project: Particle-Viewer
---

## Iron Law

```
PLAN BEFORE CODE. PROVE BEFORE SHIPPING. DELEGATE BEFORE DROWNING.
```

This skill governs planning and iteration. For debugging use `systematic-debugging`. For completion verification use `verification-before-completion`.

**Announce at start:** "I am using the execution skill to [plan/implement/delegate] [brief description]."

---

# Instructions for Agent

## How This Skill is Invoked

This skill is **mandatory for all implementation work**. `copilot-instructions.md` requires it for any task involving code changes, bug fixes, or story execution. Read it before writing code.

---

## Core Principles

These govern every decision during execution:

- **Keep It Simple:** Prefer the straightforward approach. Only introduce abstraction when it pays for itself in clarity or reuse.
- **Senior-Level Standards:** Diagnose root causes. Refuse temporary workarounds. Treat every change as if it will be reviewed by your most critical colleague.
- **Surgical Precision:** Touch only what the task requires. A smaller diff is almost always a better diff.
- **Drive to Completion:** Act on what you know. Resolve blockers yourself. Waiting for permission wastes everyone's time.

---

## Phase 1: Scope and Sequence

**Establish a clear plan before writing code** whenever the task involves 3+ steps or any architectural judgment.

### Step 0: Clarify Expectations First

**Before any plan is built** — restate your understanding of the requirements in your own words. This is the earliest-possible catch for "solved the wrong problem." Do not skip it because the requirements seem obvious.

```
BEFORE planning, state:
"Here is what I understand you're asking for:
1. [requirement 1]
2. [requirement 2]
...
[UNCLEAR: anything ambiguous or assumed, marked explicitly]

My optimization target: [user's stated outcome], not [convenient proxy].
I will flag if I find myself optimizing for the proxy instead.
"
```

**Rules:**
- Every ambiguity must be labeled `[UNCLEAR: ...]` — do not silently assume
- If the requirements are a user story with acceptance criteria: number each criterion and confirm you can map it to a verifiable test
- If the requirements have gaps: name the gap and state your assumption. The user can then correct before you've written a single line of code
- A "this seems obvious" feeling is not permission to skip this step — it is the warning sign that you need it most

### Decision Table

| Situation | Response |
|---|---|
| Obvious single-file fix | Implement immediately — no ceremony needed |
| Changes spanning multiple files | Outline the sequence first |
| Architectural or design choices | Specify the approach before touching code |
| Ambiguous requirements | Clarify Expectations first — don't plan around unknown requirements |
| User story with acceptance criteria | Decompose criteria into ordered checkpoints |

### Building the Plan

1. **Create a todo list** (`manage_todo_list`) with concrete, verifiable items
2. **Specify expected changes** up front — files, functions, test additions
3. **Bake in proof steps** — plan how you will verify each change, not just what you will build
4. **Sanity-check coverage** — does the plan address every acceptance criterion?
5. **Abandon broken plans early** — if progress stalls, scrap the approach and redesign
6. **Name known downsides proactively** — before presenting the plan, state any trade-offs, risks, or limitations the user did not ask about. "Diplomatically honest, not dishonestly diplomatic." Omitting relevant unflattering information is a form of dishonesty.
7. **Disclose decision rationale** — when choosing between valid approaches, name the alternatives considered and the specific reason the chosen approach was preferred. Transparency of reasoning, not just results.

### Smart Trust Planning Gate

Before finalizing **any** plan — answer these 5 questions. If you cannot answer them from existing knowledge, dispatch a research subagent first.

**This gate precedes writing code. It is not optional for any multi-step task.**

| # | Question | What it confronts |
|---|----------|-------------------|
| 1 | **What does success look like concretely?** State an outcome with a verification method, not an activity. "Implement X" is an activity. "X works when Y, verified by running Z" is an outcome. | Vague plans that drift |
| 2 | **What is this plan NOT addressing?** Name every requirement, acceptance criterion, and edge case the plan glosses over, defers, or assumes away. This is the highest-signal question. Most plan failures are omissions, not mistakes. | Hidden scope, skipped requirements |
| 3 | **What are the top 3 ways this plan could fail?** State them explicitly. Inability to name failure modes means the plan is not concrete enough. For each failure mode named: state the **signal** that would indicate it is occurring and the **response** if it occurs. An unmitigated named failure mode is worse than an unnamed one — you see the wall and drive toward it anyway. | Unexamined risk and blast radius |
| 4 | **Do I have the capability to execute this?** Are there libraries, patterns, or component behaviors I need to research before writing code? "I think I know" requires a dispatch to confirm. | Rework caused by wrong assumptions |
| 5 | **What would a skeptic say?** State the strongest argument against this approach. If you can't counter it, revise the plan before proceeding. | Approach chosen from comfort, not correctness |

**Any unanswered question, or any answer that reveals a gap: stop. Revise the plan. Then proceed.**

For any plan with 3+ todos or an architectural decision, dispatch a **Skeptic Agent** (see Phase 3) to answer questions 2 and 5 independently, before you start implementation.

### Decomposing a User Story

When handed an INVEST story with acceptance criteria:

1. Turn each criterion into a pass/fail checkpoint
2. Map checkpoints to files and functions
3. Sequence by dependency (tests-first is often the right order)
4. Gauge whether the scope fits a single session — propose a split if not
5. **Commitment sizing:** Before committing to scope, apply a realism check: can this be completed and verified in this session? If not, commit only to the verifiable portion and explicitly scope the remainder as a separate commitment. An over-committed partial delivery is worse than a smaller honest delivery.

### Planning Heuristics: YAGNI, Simplest Thing, and PPP

Apply these three cuts to every plan before it is finalized:

**YAGNI — You Ain't Gonna Need It**
Scan the plan for anything not directly required by the current acceptance criteria. If a todo item cannot be traced back to a specific criterion, it does not belong in this plan. Cut it.

> Forbidden rationalization: "We'll probably need it later." If it's not in the criteria, it's not in the plan.

**Simplest Thing That Could Possibly Work**
After the Smart Trust gate, ask: "Is there a simpler implementation that satisfies all acceptance criteria?" Simpler means: fewer files touched, fewer abstractions introduced, fewer dependencies added. If a simpler approach exists and the only argument against it is "the complex approach is more elegant," use the simple approach.

> Note: Simple is not the same as quick. A simple solution can take longer to find. But it is always preferable to a complex solution that passes tests.

**PPP — Plain Programmer's Purpose**
Before adding any todo item, state what the function, class, or change you're planning does in one plain sentence: "This [function/class] takes [X] and returns/does [Y]." If you cannot state it simply, the todo is not specific enough. Decompose or clarify before it enters the plan.

> The inverse also applies during execution: if you cannot describe a planned todo in plain language immediately before writing it, stop. You do not understand the requirement well enough to implement it. Re-read the requirements and re-apply PPP before proceeding.

These three heuristics together prevent the most common planning failure: scope that grows to fill available time rather than stopping at requirements.

### Rationalization Prevention

| Excuse | Reality |
|--------|---------|
| "I'll start coding, plan as I go" | Unplanned multi-step work creates cascading mistakes. Write the plan. |
| "It's obvious what needs to be done" | Obvious tasks still have sequencing and dependency risks. Apply Clarify Expectations. |
| "The todo list wastes time" | Unmarked todos get skipped. The list IS the audit trail. |
| "I'll update the todo list later" | Later never comes. Update BEFORE starting, AFTER finishing each item. |
| "This is a small change, no plan needed" | Small changes become large ones. A 3-item plan takes 2 minutes. |
| "I understand the requirements, no need to restate them" | Misunderstood requirements are the most expensive bug. Restate them anyway. |
| "We'll probably need this later" | YAGNI. If it's not in the criteria, it's not in this plan. |

---

## Phase 2: Disciplined Iteration

### Keep Commitments

When you announce what you will do — in the session-start announcement, in a plan summary, or in a response — those statements are **commitments**, not intentions.

**Rules:**
- Every announced item must be delivered, OR explicitly acknowledged as undelivered before the response ends
- The format for a missed commitment: `COMMITMENT NOT MET: I committed to [X]. I could not complete it because [specific reason]. Next step: [concrete action]`
- Never let a commitment expire silently — do not end a response with an announced item quietly dropped
- "I'll get to it next turn" is not a completion — only "I completed X, verified by [evidence]" is

This behavior is the foundation of predictable trust. Consistent commitment-keeping means the user can plan around your output without second-guessing whether announced work was actually done.

**Mid-session expectations drift:** If user feedback mid-session reveals your understanding of a requirement was wrong, stop and re-execute Step 0 (Clarify Expectations) before continuing. Do not silently absorb the correction and continue on the old plan.

### The Work Loop

For every planned item, before writing code:

**PPP Gate (Plain Programmer's Purpose):** State what the function or class you are about to write does in one plain sentence: "This function takes [X] and returns [Y] by doing [Z]." If you cannot state it simply, you do not understand the requirement well enough to code it. Stop and re-read the requirements.

```
1. Flag it as in-progress
2. PPP: State the purpose of the code you're about to write (one sentence)
3. Make the change
4. Prove it works (compile, test, inspect diff)
5. Flag it as done
6. Commit when you reach a logical boundary
7. Advance to the next item
```

**A task is not done until you can demonstrate it works.** That means:
- Clean compilation
- All tests green
- Diff reviewed for side-effects
- You would confidently submit this for code review


### Communicating Progress

- Summarize at the milestone level, not the keystroke level
- State what changed, what was validated, and what comes next
- When all items are complete, give a brief accounting of files touched, tests added, and notable decisions
- **Temporal declaration:** When a plan requires more turns than the user likely expects, state this proactively at plan time: "This will take approximately N responses. Here is what each will deliver: [sequence]." Do not surface this mid-execution — declare it at plan time.
- **Attention cost:** Before sending any response longer than 200 words or creating new work for the user, ask: is this the most respectful use of their attention? Compress or defer non-essential content.

### Commit Rhythm

- One commit per logical unit — not per file, not per session
- Every commit must build and pass tests on its own
- Follow conventional commit format (see `versioning` skill)
- Never lump unrelated changes together

### Red Flags — STOP

If you catch yourself thinking any of these:
- "I'll mark it done after I clean up a few things"
- "The tests will probably still pass"
- "I'll commit everything at the end"
- "Just one more small change before I verify"
- "This is done enough to move on"
- "Evidence contradicts the plan but I'll finish this step first" — **Stop. Confront reality immediately.** State what the plan assumed, what evidence shows, and what that means for remaining todos. Revise the plan before proceeding, even if it voids completed work. Continuing on a plan you know is wrong is not progress.

**All of these mean: Stop. Run the full verification gate before advancing. See `verification-before-completion` skill.**

---

## Phase 3: Delegating to Subagents

**Iron Law:**

```
DISPATCH BEFORE GUESSING — SUBAGENTS ARE CHEAP, WRONG ASSUMPTIONS ARE EXPENSIVE
```

### Why Subagents Are Mandatory (Not Optional)

Using subagents is not a convenience — it is a discipline:

- **Main context fills up fast.** Once full, early instructions — including these rules — are evicted. Keep the main context lean.
- **Subagents search without your existing bias.** They are better at confirming or denying theories precisely because they lack your assumptions.
- **Parallel dispatch is faster.** Five explore agents running simultaneously beat five sequential searches in one context.
- **Subagents keep your diff clean.** Exploratory dead-ends, failed theory checks, and interim summaries never appear in your main context history.

### The Empirical Evidence Mandate

This is the most important rule in this phase. No theory, assumption, or belief is acceptable as a basis for action. Every claim must be backed by empirical evidence before you proceed.

**Acceptable evidence:**
- Code you read yourself — opened the file, saw the line
- Test output you ran yourself — executed the command, read the result
- Documentation cross-checked against the actual source code
- A/B test results — implemented both, compared outcomes
- A targeted test written specifically to prove or disprove the theory

**Unacceptable:**
- Memory ("I remember this worked before")
- Confidence ("I'm sure the parser handles this")
- Assumption ("It should work because Y")
- Prior success ("It worked last time")

**Decision rule:** If you cannot point to a specific file, line, or test run that supports your claim, you do not have evidence. Dispatch a subagent to get it.

### Extend Smart Trust — Calibration Rule

Trust is extended in proportion to: (1) prior verification of the source's reliability in this domain, and (2) the cost of being wrong.

| Source reliability | Cost of being wrong | Action |
|---|---|---|
| Unverified | High | Verify before acting |
| Unverified | Low | Act, note the assumption |
| Verified | High | Act with confidence |
| Verified | Low | Act immediately |

Apply this to: subagent findings, user-stated constraints, existing skill rules, and your own memory. The rule prevents both blind trust (acting on unverified claims) and zero trust (re-verifying everything, which is its own cost). Memory is always "unverified" — prior context evicts.

#### Red Flags → STOP

If you catch yourself forming any of these thoughts, stop immediately and dispatch a subagent before proceeding:

- "I think the issue is..."
- "This should work because..."
- "I'm confident that..."
- "It probably passes..."
- "Based on how it usually works..."
- "I remember that..."

**All of these mean: Stop. Dispatch a subagent to confirm. Get evidence first.**

Concrete examples for this project:

| Thought | Required action |
|---------|----------------|
| "I think `SDL3Context` handles this edge case" | Dispatch explore agent → read `SDL3Context.cpp` |
| "The shader should compile fine with this change" | Run `cmake --build build` and read the output |
| "I'm pretty sure the Google Test fixture handles teardown" | Read the test output or the GTest docs directly |
| "CMake probably finds this dependency automatically" | Run CMake, read the configure output |
| "clang-tidy is probably fine with this pattern" | Run clang-tidy, read the diagnostics |

### Git Worktrees for Parallel Subagent Work

When a subagent needs to **modify files** (not just read them), give it its own worktree. This avoids branch switching in the main context and lets multiple subagents work in parallel safely.

```bash
# Create a worktree for a subagent
git worktree add ../particle-viewer-agent-1 -b feat/subagent-work

# List active worktrees
git worktree list

# Remove after merging
git worktree remove ../particle-viewer-agent-1
```

**Workflow:**
1. Create the worktree before dispatching the subagent
2. Pass the worktree path as the working directory in the subagent prompt
3. Subagent makes changes inside its worktree — no interference with main context
4. After the subagent completes, review its diff: `git -C ../particle-viewer-agent-1 diff main`
5. Cherry-pick or merge the changes into the main branch
6. Remove the worktree: `git worktree remove ../particle-viewer-agent-1`

Read-only subagents (explore agents doing research) do not need a worktree.

### Subagent Dispatch Table

| Task | Dispatch | Why |
|------|----------|-----|
| Exploring unfamiliar APIs or libraries | Yes — explore agent | Keep investigation out of main context |
| Scanning codebase for patterns (5+ files) | Yes — explore agent | Parallel search is faster |
| Confirming a theory or assumption | Yes — explore agent | Unbiased confirmation, satisfies evidence mandate |
| **Validating a plan before implementation** | **Yes — Skeptic Agent (see below)** | **No stake in the plan being good; finds omissions** |
| Code review (per-file) | Yes — code-review agent, 1 per file | Cheap, thorough, out-of-band |
| Skill review | Yes — general-purpose with skill-reviewer skill | Dispatch 1 per skill file |
| Digesting multiple large files | Yes — explore agent | Collect summaries, not raw text |
| Writing a single function inline | No — do inline | Fast enough for main context |
| Hard problem needing multiple approaches | Yes — fan out to multiple general-purpose agents | Compare results empirically |
| Architecture validation | Yes — general-purpose with architecture-review skill | Unbiased structural check |

### The Skeptic Agent

Dispatch this agent **before starting implementation** on any plan with 3+ todos or an architectural decision. Its job is not to approve the plan — it is to find what is missing or wrong.

**When to dispatch:**
- Any plan with 3+ concrete todos
- Any change that touches 2+ layers in the architecture
- Any time the answer to Smart Trust Question 2 ("What is this NOT addressing?") feels uncertain

**Prompt template:**

```
You are a Skeptic Agent. Your job is to find what this plan is missing.
You have no stake in the plan being good. Do not validate what looks correct.

Requirements/acceptance criteria:
[PASTE THE FULL REQUIREMENTS]

Plan under review:
[PASTE THE FULL PLAN WITH ALL TODOS]

Answer ONLY these questions. Be specific and direct.

1. What is this plan NOT addressing that the requirements ask for?
   List every omission, deferred item, or assumption that was not explicitly validated.

2. What assumptions is this plan making that might be wrong?
   Name the assumption, then state what would need to be true for it to hold.

3. What is the strongest argument against this approach?
   State the best case for doing it differently.

4. What is the most likely way this plan fails in practice?
   Name the failure mode, not a generic risk.

If you genuinely find no gaps after thorough analysis, state that explicitly
and explain what you checked. A clean result is valid — but absence of
findings must be earned, not assumed.

Return a structured report with the 4 sections above.
```

**How to use the result:**
- If the agent surfaces a gap: revise the plan before writing code
- If the agent returns clean: record that confirmation and proceed
- Do NOT skip revision because "the gaps are small" — a gap in the plan is a defect not yet written into code; it is cheap to fix now

### Reviewer Dispatch Pattern

When reviewing work — code, skills, architecture, infrastructure — dispatch **one agent per file**:

- Do not review 5 files in one agent. Review them in 5 parallel agents.
- Each reviewer agent receives: the file path, the skill to apply, and the specific question to answer.
- Collect all results before acting on any of them.
- This is faster and produces more thorough coverage than one agent reviewing everything sequentially.

### When NOT to Dispatch

- Quick grep/glob searches you can do in one tool call — do inline
- Reading a single known file — do inline
- Trivial single-step commands — do inline
- Do not speculatively launch agents "just in case" — dispatch only when you have a clear objective

### Delegation Quality Rules

- **One clear objective per subagent** — no multi-part briefs; focused tasks yield better results
- **State the return format explicitly** — tell it exactly what to give back
- **Provide complete context** — subagents are stateless; they cannot refer back to the conversation
- **Accept findings unless they conflict with evidence you have verified yourself**
- **If a subagent finds something unexpected:** treat it as data, verify it before acting on it

### Delegation Anti-Patterns

| Anti-pattern | Why it fails |
|---|---|
| "I'll just check this quickly myself" (for 5+ files) | Fills context, slower, biased by existing assumptions |
| One agent reviewing multiple large files | Coverage is shallow; 1 agent per file is the rule |
| Dispatching without a clear return format | Agent returns noise; you waste time extracting signal |
| Acting on a subagent's finding without verifying | Subagents can be wrong; treat findings as hypotheses until confirmed |
| Hoarding work in main context to avoid dispatch overhead | Dispatch overhead is trivial; context loss is not |

---

## Phase 4: Prove It Before You Ship It

**REQUIRED: Use the `verification-before-completion` skill.**

Before claiming any task done:
- Build: `cmake --build build`
- Tests: `./build/tests/ParticleViewerTests`
- Format: `find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i`
- Diff: `git diff` — read every hunk for accidental changes

The verification-before-completion skill contains the complete gate function, common failure modes, and rationalization table. Read it if you are tempted to claim completion without running these commands.

---

## Phase 5: Pursue Clarity

After something works but before you commit, pause: **"Is there a cleaner way to express this?"**

### Apply When

- A working solution feels convoluted or brittle
- Hindsight reveals a simpler path: "Given what I now know, I'd write it differently"
- Nearby code could be simplified without expanding scope

### Behavior Preservation Gate (for Refactoring)

Refactoring means changing structure without changing behavior. Before and after any refactoring pass:

1. Run the full test suite — confirm green baseline **before** any refactoring changes
2. Make only structural changes (rename, extract, move)
3. Run the full test suite — confirm green after each discrete change
4. If a test turns red during refactoring: you changed behavior, not structure. Revert the last change and re-examine.

**Never introduce new behavior during refactoring.** If you discover a bug while refactoring, stop. Record it. Finish the refactor. Then fix the bug as a separate commit. The commit history must distinguish "refactored" from "fixed."

### Skip When

- The fix is already obvious and direct
- The change is purely mechanical (formatting, renames)
- Improvement would require unrelated refactoring

### Self-Review

Before presenting your work:
- Is this the most straightforward implementation?
- Did I add complexity that the problem didn't demand?
- Would I be comfortable defending every line in review?

### Explicit Trade-Off Discipline

Every significant design or implementation choice involves trade-offs. Unstated trade-offs are silent risks.

**Template:** When choosing between approaches, use this structure:
```
Approach A: [what it gives] / [what it costs]
Approach B: [what it gives] / [what it costs]
Choice: [A or B] because [specific reason the give outweighs the cost in this context]
```

This is required when:
- Two or more valid approaches exist
- You've rejected a simpler approach in favor of a more complex one
- A design decision involves a correctness/performance/maintainability tradeoff
- The choice affects testability, coupling, or interface stability

It is NOT required for trivial decisions (variable names, formatting choices, single-function implementations with one obvious path).

**Pain as Signal:** If an implementation is difficult to test, hard to mock, or requires complex setup to use — that is a design signal, not a complexity signal. Painful tests indicate LoD violations, excessive coupling, or hidden dependencies. Fix the design, not the test harness.

### Mode Declaration

Before beginning a multi-phase response (e.g., researching → planning → implementing), declare the current mode:

```
MODE: [Research / Planning / Implementation / Verification / Review]
```

This is a one-line declaration at the start of the relevant section. It tells the user what kind of output to expect and makes phase transitions explicit. A user who sees "MODE: Implementation" knows code is coming. A user who sees "MODE: Research" knows questions are being answered before action is taken.

**Required for:** any response that combines two or more phases, or any response to a complex task where the user might not know whether you're planning or doing.

---

## Phase 6: Resolve Errors Autonomously

**REQUIRED: Use the `systematic-debugging` skill.**

When you encounter a bug, failing test, build error, or CI failure:
1. STOP before proposing fixes
2. Load the `systematic-debugging` skill
3. Follow its 4-phase protocol
4. Apply the fix only after Phase 1 (root cause) is complete

**Prohibited:**
- Proposing a fix you haven't traced to root cause
- "Just try this" attempts
- Asking whether you should fix an obvious error — fix it
- Settling for a workaround when a proper fix is reachable

The systematic-debugging skill contains the complete 4-phase protocol, 3-strikes-architecture rule, and rationalization table.

---

## Phase 7: Continuous Skill Refinement and Right Wrongs

**Correct course in real time, not just at session end.**

### Right Wrongs — Error Correction Protocol

When a mistake is discovered — by you or by the user — the response must follow this structure:

```
1. ACKNOWLEDGE: "I was wrong about [X]."
   No minimizing. No "to be fair". No "as I mentioned earlier". No "technically...".
   Plain statement.

2. STATE WHAT WAS WRONG: "I stated [Y]. The correct answer is [Z]."
   Specific. Not "I may have been unclear" — that shifts responsibility.

3. STATE THE IMPACT: "This affected [what work / what understanding]."
   If the mistake caused wasted work or wrong decisions, say so explicitly.

4. FIX IT: [actual correction, with evidence if the correction is a claim]
```

**Forbidden responses to discovered mistakes:**
- "To be fair, you could interpret it as..." — minimizing
- "As I mentioned, there was some ambiguity..." — blame-shifting
- "That's technically correct but..." — deflecting
- Moving on without acknowledging — the most common failure mode

**Listen First — before responding to any correction:**
Read the user's feedback fully before drafting any part of a response. State internally — not in the response — what the user's point actually is, separate from whether you agree with it. Responding before understanding is a Listen First failure. A defense written before the criticism is understood is not a response; it is a collision.

**Symmetric Right Wrongs:** When a user instruction will produce a defective result, the agent has standing to say so: "I'll proceed if you confirm, but this instruction will cause [specific consequence]. Confirming your intent before acting." Do not proceed silently on instructions you know are wrong.

The Right Wrongs protocol is fast. A mistake acknowledged correctly takes less time than the downstream confusion of a mistake glossed over. Applying this protocol is a trust deposit — users who see mistakes acknowledged clearly stop second-guessing other outputs.

### Continuous Refinement

After any mistake or user correction during a session:

1. **Name the failure mode** — what exactly went wrong?
2. **Search existing skills** — is this already documented?
3. **If it's new, update the relevant skill now** — don't defer to the post-session review
4. **Write a concrete prevention rule** — vague advice doesn't prevent recurrence
5. **Keep tightening** — revisit rules that still produce mistakes and sharpen them

This complements the mandatory end-of-session self-evaluation. The difference: this happens the moment you learn something, not after the work is finished.

### Starting a New Session

Before beginning work, **read every skill your task requires** (per the auto-load table in `copilot-instructions.md`). This is how lessons from prior sessions carry forward. Read to internalize, not to check a box.

### Concrete Example

```
Failure: Assumed empty clang-format output meant all files were clean
Fix: Added "inspect git diff after formatting" to code-quality pre-commit checklist
Rule: "Verify formatting changes via diff — silence is not confirmation"
```

---

## Skill Dispatch Table

When the task involves these domains, dispatch to the relevant skill FIRST:

| Domain | Skill to Load |
|--------|---------------|
| Encountering a bug or failure | `systematic-debugging` |
| Claiming work is complete | `verification-before-completion` |
| Writing tests | `testing` |
| Writing/editing C++ | `code-quality` |
| Creating commits or PRs | `versioning` |
| CI/CD work | `workflow` |
| Build or dependencies | `build` |
| Confirming a theory or assumption | `explore agent` + empirical evidence mandate (Phase 3) |
| Reviewing code files | `code-review agent`, 1 per file |
| Reviewing skill files | `general-purpose agent` with `skill-reviewer` skill |

---

## Prohibited Patterns

These behaviors are explicitly disallowed:

1. **Waiting for a green light** — if the task is defined, begin executing
2. **Surfacing problems without fixes** — always pair a diagnosis with a resolution
3. **Declaring done without proof** — every completion claim needs supporting evidence
4. **Doubling down on a stuck approach** — halt, rethink, restart
5. **Gold-plating simple work** — match effort to actual complexity
6. **Winging complex work** — multi-step tasks get a written plan
7. **Walking past defects** — if you spot an issue while working, address or log it
8. **Believing empty output** — always cross-check with a second method
9. **Bypassing skill dispatch** — claiming a fix is done without `verification-before-completion`, or proposing a fix without `systematic-debugging`. The skills exist to prevent the patterns in this list.

---

## Quick Reference

```
Task arrives
    ↓
Trivial (1-2 steps)?
    Yes → Implement, verify, commit
    No  ↓
Plan: build todo list with verifiable items
    ↓
Per item:
    Start → Implement → Bug/Error? → Load systematic-debugging
                ↓
            Prove → Complete → Commit
    ↓
    Stuck? → Stop → Re-plan → Resume
    ↓
All done? → Load verification-before-completion → Run gates
    ↓
Self-evaluate (per self-evaluation skill)
    ↓
Ship it
```
