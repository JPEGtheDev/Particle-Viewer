---
title: "Estimation Examples"
description: "Validated real-world effort estimates from completed Particle Viewer stories, for calibrating new estimates against actual outcomes."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, planning, estimation, calibration]
related:
  - "PROJECT_CONTEXT.md"
  - "DONE_DEFINITION.md"
---

# Particle-Viewer Estimation Examples

Validated real-world estimates from completed Particle-Viewer stories. Use to calibrate future estimates.

Each story below is sized with a T-shirt size (S/M/L, for Small/Medium/Large scope) and a "requests" count. A "request" here means one GitHub Copilot coding-agent premium request -- the billing unit GitHub meters per agent turn -- as recorded in the linked issue or epic. Premium-request cost varies by model, so the model used (e.g. Sonnet 4.5, Opus 4.6) is noted alongside each figure instead of a difficulty tier: the tracker does not use a "Standard"/"Premium" difficulty tier for individual stories. In epic issues, "Premium Requests" names the billing unit for the whole epic's total, not a per-story split, so the model name is the only per-story tier data the tracker actually records.

---

## Framebuffer Capture (M, Sonnet 4.5, estimated 30-40 requests)

Estimate from epic #20; issue #19 itself does not record a request estimate or an actual count.

- Implementation: 15-20 (OpenGL capture, PPM (Portable Pixmap) writing, vertical flip)
- Testing: 8-12 (unit tests, headless validation)
- Iteration: 5-6 (edge cases, performance tweaks)
- Docs: 2-3 (API comments, README update)

---

## Global State Elimination (L, Opus 4.6, estimated 80-100 requests)

Estimate from epic #26 for issue #30 (Modernize Data Types & Encapsulation). Delivered together with #31 and #32 in one PR, so no separate actual request count was recorded for #30 alone. The delivered shape is the `ViewerApp` class plus plain data structs (`WindowConfig`, `RenderResources`, `SphereParams`, `ShaderPaths`) replacing the globals formerly in `clutter.hpp` -- not manager classes.

- Implementation: 40-50 (`ViewerApp` class + config/resource structs, RAII (Resource Acquisition Is Initialization) ownership of GL/SDL3 resources, dependency-injected `IOpenGLContext`)
- Testing: 20-25 (unit tests for `ViewerApp` and structs, integration tests)
- Iteration: 15-20 (fixing circular deps, refactoring existing code)
- Docs: 5-8 (architecture diagrams, migration guide)

---

## Add & Enforce Coding Standards (S, Sonnet 4.5, estimated 20-25, actual 3 requests)

- Real-world data from #27: Foundation work was simpler than anticipated once style guide was in place
- *Lesson:* "Foundational" work can be lighter if prerequisites are clear; calibrate accordingly

---

## Related

- [Project Context](PROJECT_CONTEXT.md) -- component inventory and architecture overview these estimates are drawn from
- [Definition of Done Stages](DONE_DEFINITION.md) -- what "actual" effort is measured against for a completed story
