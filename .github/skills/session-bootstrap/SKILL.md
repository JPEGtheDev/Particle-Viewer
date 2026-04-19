---
name: session-bootstrap
description: Use at the start of every session.
---

## Iron Law

```
READ THE SKILL. THEN ACT. NEVER ACT THEN READ.
```

Violating the letter of this rule is violating the spirit of this rule.

YOU MUST read all required skills before writing any code or sending any plan. No exceptions.
Skills contain rules that change what you do. Reading a skill AFTER acting defeats the purpose.

**Announce at start:** "I am using the session-bootstrap skill to load required skills for this session."

---

## Skill Refresh — Mandatory

**Stale skill context is worse than no skill.** Skills evolve. Context windows truncate. Skills loaded early in a session may no longer be active when you need them.

### When to Reload a Skill

Reload the relevant skill(s) immediately when ANY of these occur:

1. **Picking up a new todo** — reload the skill(s) for that todo's domain before starting work
2. **After 3 user prompts** without a skill reload — reload the skill for whatever you are currently doing
3. **After a user correction or redirect** — the correction is evidence the skill was misapplied or is stale; reload it
4. **After context compaction** — any compaction event requires full skill reload for the active domain

**Announce the reload:** "Reloading `[skill-name]` — [reason: new todo / 3 prompts / correction]."

Do NOT say "I remember the skill content." A remembered skill is an unverified skill. Load fresh.

### `honesty` Is Always Active

Treat as permanently loaded. Load the full skill for postmortems and communication audits.

---

## On Start — Minimum Skill Loads by Task Type

**`honesty` is always active.** It is hardcoded into the session-start hook and applies to
every turn, every task, every model. You do not need to explicitly load it — but load the
full skill when running a postmortem or when communication quality is being audited.

Before writing code, read the skill(s) relevant to your task. If the task touches multiple
domains, read multiple skills in parallel (they are independent reads).

| If the task involves…                        | MUST read these skills BEFORE acting               |
|----------------------------------------------|----------------------------------------------------|
| Any implementation work                      | `execution`                                        |
| Planning a multi-step task                   | `writing-plans`                                    |
| Unclear approach or design choices           | `brainstorming`                                    |
| Writing or editing C++ code                  | `execution`, `code-quality`                        |
| Writing C++ with GL/SDL3/runtime patterns    | `execution`, `code-quality`, `cpp-patterns`        |
| Writing or editing tests                     | `execution`, `code-quality`, `testing`             |
| Writing visual regression tests              | `execution`, `code-quality`, `testing`, `visual-regression-testing` |
| Creating a PR or commit                      | `versioning`, `verification-before-completion`     |
| Finishing / closing a branch                 | `finishing-a-development-branch`                   |
| Requesting code review                       | `requesting-code-review`                           |
| Receiving code review feedback               | `receiving-code-review`                            |
| CI/CD or workflow changes                    | `workflow`                                         |
| Build system or dependency changes           | `build`                                            |
| Writing or editing documentation             | `documentation`                                    |
| Bug fixes or error resolution                | `execution`, `systematic-debugging`                |
| Any failure or unexpected behavior           | `systematic-debugging`, `verification-before-completion` |
| Dispatching subagents                        | `subagent-driven-development`                      |
| Parallel agent work / A/B testing            | `subagent-driven-development`, `using-git-worktrees` |
| Creating user stories                        | `user-story-generator`, `user-story-estimation`    |
| Creating or editing a skill file             | `writing-skills`, `skill-reviewer`                 |
| Resuming from a prior session with pending SQL todos | `writing-plans`; dispatch Skeptic before first implementation step |
| Schema design, new data structure, or plan with ≥5 implementation items | `brainstorming`, `writing-plans` |
| Auditing communication quality or postmortem | `honesty`, `session-postmortem`                    |

If unsure, read `code-quality` — it applies to every code task.

**Loading protocol:**
1. Identify task type(s) from the table above
2. Load all required skills before writing a single line of code or sending a plan
3. Announce each skill load: "I am using the [skill-name] skill to [purpose]."

BEFORE PROCEEDING with any session work, verify:
1. Task type(s) identified from the On Start table above
2. All required skills for this task type loaded (in parallel if multiple domains)
3. Skill load announcement made for each loaded skill
4. If resuming a prior session: SQL pending todos checked; Skeptic dispatched before first implementation step

✓ All 4 met → proceed with session work
✗ Any unmet → complete the unmet step now before writing code or sending a plan

---

## On Finish — Self-Evaluate and Compact

**Self-evaluation fires at session END.** If a major task completes mid-session with no further work planned, treat it as session end and execute On Finish now.

**Before your final message to the user**, execute all of the following:

1. **Read** `.github/skills/self-evaluation/SKILL.md` and follow its steps.
2. **Identify lessons learned** — mistakes made, user corrections, patterns discovered.
3. **Check existing skills** — is the lesson already documented? If yes, skip.
4. **Apply updates** — for High/Medium priority lessons, update the relevant skill file
   and bump its version in the YAML frontmatter. Commit the skill update with the
   session's work.
5. **Compact** — scan any files you touched for bloated comments or duplicated docs.
   Migrate detail to skills/docs and leave 1-line references.
6. **Report** — include a `### Session Self-Evaluation` block in your final message:
   ```
   ### Session Self-Evaluation
   Lessons: [count] | Skills updated: [list or "None"] | Compacted: [files or "None"]
   ```

If you have nothing to report, still include the block with zeroes. This ensures the
behavior is habitual, not conditional.

---

## Red Flags — STOP

- A task just completed and no new user message has arrived — **STOP. Is this the session's last task? If so, treat it as session end. Load self-evaluation NOW before responding.**
- Starting implementation when SQL has pending todos from a prior session without dispatching Skeptic — **STOP. Dispatch Skeptic before the first implementation step.**
- Starting to code before reading the required skill — **STOP. Load the skill now. Do not write one line first.**
- Skipping the skill-load announcement — **STOP. State "I am using the [skill] skill to [purpose]." No skip.**
- Finishing a session without running `self-evaluation` — **STOP. Read `.github/skills/self-evaluation/SKILL.md` now.**
- Treating the "On Finish" steps as optional — **STOP. They are mandatory. Execute every step.**
- Saying "I remember the skill content" — **STOP. Memory degrades. Skills update. Load fresh every session.**

---

## Rationalization Prevention

| Rationalization                                    | Why it fails                                       | Correct action                              |
|----------------------------------------------------|----------------------------------------------------|---------------------------------------------|
| "This task is simple — I don't need the skill"     | Simple tasks still violate iron laws when unskilled | Load the skill. Takes 5 seconds.           |
| "I remember the skill from last session"           | Memory degrades; skills update; load fresh          | Load the skill now.                        |
| "I'll self-evaluate if anything went wrong"        | Self-evaluation finds what you didn't notice wrong | Always self-evaluate. No conditional.      |
| "Skipping announcement to save space"              | Announcement is the commitment mechanism           | State it. No skip.                         |
| "I'll skim the skill — I know the gist"            | Skimming misses updates and specific gate conditions | Read fully. The gate conditions are the point. |

## Related Skills

- `self-evaluation` — the On Finish step calls this skill directly
- `honesty` — always-active; hardcoded into session-start hook; never needs explicit loading
- `writing-skills` — governs skill authoring; load when creating or editing a skill
