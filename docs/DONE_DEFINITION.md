---
title: "Definition of Done Stages"
description: "What each completion stage means for Particle Viewer as a desktop application with no deploy stage."
domain: cross-cutting
subdomain: ""
tags: [process, verification, done]
related:
  - "TESTING_STANDARDS.md"
---

# Particle-Viewer Done Definition

For this project (a desktop application, not a deployed service), the equivalent stages are:
- "Locally verified" -- builds + tests pass after the change
- "Gate passed" -- full pre-commit gate (build + tests + format + diff review) completed
- "Committed" -- code committed with passing gate
- **"Done"** -- committed, PR merged, branch clean

Use stage vocabulary. Never use "Done" for a stage that is not the final stage.
