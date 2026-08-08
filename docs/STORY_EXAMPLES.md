---
title: "Story Examples"
description: "Worked story-writing examples for Particle Viewer -- acceptance criteria, technical notes, and subtask breakdowns for calibrating new stories."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, planning, stories, templates]
related:
  - "ESTIMATION_EXAMPLES.md"
  - "PROJECT_CONTEXT.md"
---

# Particle-Viewer Story Examples

Worked story examples imported from the development harness, kept for calibrating how new stories are written (acceptance criteria style, technical-notes detail, subtask breakdown). These are illustrative, not current-state documentation: the refactoring example below names `FramebufferManager`/`WindowManager` as a target design, but the delivered shape (per [Estimation Examples](ESTIMATION_EXAMPLES.md)) is a `ViewerApp` class plus plain data structs.

---

## Acceptance Criteria: Framebuffer Capture

Example of Given/When/Then acceptance criteria for a feature story:

- [ ] **Given** an OpenGL context with a bound framebuffer, **When** `captureFrame()` is called, **Then** a PPM file is written to disk within 16ms
- [ ] Image data is captured with correct RGBA format and proper vertical flip (OpenGL origin compensation)
- [ ] Handles edge cases: no active framebuffer, resolution changes mid-capture
- [ ] File I/O succeeds with no memory leaks (validated with Valgrind)

---

## Technical Notes: Refactoring Example

Example Technical Notes section for a refactoring story that extracts global state into owning classes:

**Current State (Before):**
- Global variables: `framebuffer`, `window`, `particle_set`
- Issue: Hard to test in isolation, can't run tests in parallel

**Target State (After):**
- `FramebufferManager` class encapsulating framebuffer state
- `WindowManager` class managing window lifecycle
- All state passed as constructor dependencies

**Files to Create/Modify:**
- `src/graphics/FramebufferManager.hpp` (new)
- `src/graphics/FramebufferManager.cpp` (new)
- `src/main.cpp` (refactored to use new manager)

**Definition of Done:**
- [ ] Old code deleted (no dead code left behind)
- [ ] All tests pass on CI
- [ ] Code review completed
- [ ] No performance regression (benchmark before/after)
- [ ] Refactoring checklist applied (remove duplication, improve names, etc.)

---

## Subtask Breakdown Example

Example of breaking a larger story into estimated, dependency-ordered subtasks:

**Parent Story:** Create Framebuffer Capture Utility

### Subtask 1: Design FramebufferCapture Class
- [ ] Define public API (methods, error codes)
- [ ] Document memory ownership (who deletes buffers?)
- [ ] Sketch architecture: OpenGL initialization, capture, conversion

**Estimate:** S (2-3 hours)

---

### Subtask 2: Implement Frame Capture
- [ ] Write FramebufferCapture::captureFrame() method
- [ ] Test with manual OpenGL context in isolated test
- [ ] Handle edge cases (no framebuffer bound, resolution mismatch)

**Estimate:** M (4-6 hours)
**Depends on:** Subtask 1

---

### Subtask 3: Implement PPM File Writing
- [ ] Write pixel data to PPM format
- [ ] Ensure vertical flip is correct
- [ ] Test with ImageMagick `display` to verify output

**Estimate:** S (2-3 hours)
**Depends on:** Subtask 2

---

### Subtask 4: Unit Tests & Integration
- [ ] Write GoogleTest tests for FramebufferCapture
- [ ] Mock OpenGL calls
- [ ] Test in headless mode with Xvfb

**Estimate:** M (4-6 hours)
**Depends on:** Subtasks 2-3

---

## BDD-Style Acceptance Criteria

Example of a Given/When/Then acceptance criterion parameterized with a specific resolution:

- [ ] **Given** a valid OpenGL context, **When** captureFrame() is called with resolution 1920x1080, **Then** a 1920x1080 PPM file is created within 16ms

---

## Metric-Based Acceptance Criteria

Example of acceptance criteria phrased as measurable metrics rather than Given/When/Then:

- [ ] Frame capture latency <=16ms (60 FPS budget)
- [ ] Memory overhead <=50MB per capture
- [ ] Zero memory leaks (Valgrind report)

---

## File/Component-Specific Acceptance Criteria

Example of acceptance criteria that pin down exactly which files carry the tests and the implementation, including the current-state to target-state renaming convention:

- [ ] Tests in `tests/core/FramebufferCaptureTests.cpp` (or target-state: `tests/graphics/FramebufferCaptureTests.cpp`)
- [ ] Implementation in `src/FramebufferCapture.{hpp,cpp}` (or target-state: `src/graphics/framebuffer_capture.{hpp,cpp}`)
