---
title: "Definition of Done Stages"
description: "What each completion stage means for Particle Viewer as a desktop application with no deploy stage."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, process, verification, done]
related:
  - "TESTING_STANDARDS.md"
  - "VERIFICATION_COMMANDS.md"
  - "PROJECT_CONTEXT.md"
---

# Particle-Viewer Done Definition

Particle Viewer is a desktop application with no deploy stage. Its completion stages are:
- "Locally verified" -- builds and tests pass after the change
- "Gate passed" -- the pre-commit gate defined in [Verification Commands](VERIFICATION_COMMANDS.md) (format, build, tests) has completed
- "Committed" -- code committed with a passed gate; a manual diff review is a separate, non-automated step performed before committing (this repo has no pre-commit hook that enforces it)
- **"Done"** -- committed, PR merged, branch clean

Use stage vocabulary. Never use "Done" for a stage that is not the final stage.

## Related

- [Verification Commands](VERIFICATION_COMMANDS.md) -- the three commands that make up the pre-commit gate
- [Testing Standards](TESTING_STANDARDS.md) -- testing expectations and patterns
- [Project Context](PROJECT_CONTEXT.md) -- per-story Definition of Done Checklist (code quality, testing, documentation, and review/merge criteria)
