---
name: session-bootstrap
version: 1.0.0
description: >
  Use at the start of every session to determine which skills to load. Contains the
  On Start skill routing table (minimum skills by task type) and the On Finish
  self-evaluation steps. LOAD THIS SKILL FIRST before any other domain skill.
---

## Iron Law

```
READ THE SKILL. THEN ACT. NEVER ACT THEN READ.
```

Violating the letter of this rule is violating the spirit of this rule.

Skills contain rules that change what you should do. Reading a skill AFTER acting defeats
the purpose. If you find yourself thinking "I'll check the skill after I start" — stop.
Read the skill first.

---

## On Start — Minimum Skill Loads by Task Type

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
| Creating or editing a skill file             | `writing-skills`                                   |
| Reviewing trust/honesty behavior             | `honesty`                                          |
| Running a session retrospective              | `session-postmortem`                               |

If unsure, read `code-quality` — it applies to nearly every code task.

**Loading protocol:**
1. Identify task type(s) from the table above
2. Load all required skills before writing a single line of code or sending a plan
3. Announce each skill load: "I am using the [skill-name] skill to [purpose]."

---

## On Finish — Self-Evaluate and Compact

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

- Starting to code before reading the required skill
- Skipping the skill-load announcement
- Finishing a session without running `self-evaluation`
- Treating the "On Finish" steps as optional

---

## Rationalization Table

| Rationalization                                    | Why it fails                                       | Correct action                              |
|----------------------------------------------------|----------------------------------------------------|---------------------------------------------|
| "This task is simple — I don't need the skill"     | Simple tasks still violate iron laws when unskilled | Load the skill. Takes 5 seconds.           |
| "I remember the skill from last session"           | Memory degrades; skills update; load fresh          | Load the skill now.                        |
| "I'll self-evaluate if anything went wrong"        | Self-evaluation finds what you didn't notice wrong | Always self-evaluate.                      |
| "Skipping announcement to save space"              | Announcement is the commitment mechanism           | State it. No skip.                         |
