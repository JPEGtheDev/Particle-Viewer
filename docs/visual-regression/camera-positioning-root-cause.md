---
title: Camera Positioning Root Cause Analysis
description: Root cause analysis of the composition failure in visual regression test #68, where debug camera coordinates produced incorrect framing.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, camera, opengl]
related:
  - docs/visual-regression/camera-positioning-lessons-learned.md
  - docs/visual-regression/camera-positioning-guidelines.md
  - docs/visual-regression/enhanced-debug-output.md
---

# Camera Positioning Root Cause Analysis

## Issue Summary

When implementing the particle cube visual regression test (#68), debug camera
coordinates were copied directly as the test camera position. The result: the cube
filled ~80% of the viewport instead of the intended ~50% with balanced blank space.

## Reference vs. Test Output

| | Composition |
|-|-------------|
| **Reference image** | Cube well-framed, ~50% blank space |
| **Initial test** | Cube too large, ~80% viewport, minimal blank space |

The debug overlay at the time of the reference screenshot showed:

```text
Pos: (-16.72, 16.49, -8.95)
Target: (-15.99, 16.02, -8.04)
```

## Root Cause

### Debug Output Was Correct ✓

The debug overlay (`src/debugOverlay.hpp`) accurately reported camera position,
target (lookat point), and up vector. There were no bugs.

### The Incorrect Assumption ✗

> "Debug coordinates from the reference image will produce the same visual composition in the test."

Reality:
- The debug screenshot was taken while the user was **zoomed in** for a detailed view.
- Camera was only **29 units** from the cube center — too close for desired framing.
- Target was 28 units from the cube center — camera was looking **past** the cube.

### What Composition Depends On

| Factor | Debug setup | Needed |
|--------|-------------|--------|
| Distance to subject | 29 units | 51 units |
| FOV | 45° | 45° |
| Coverage | ~70–80% | ~40–50% |
| Viewing angle | Preserved | Preserved |

Simplified coverage formula:

```text
coverage ≈ (subject_size / distance) * tan(FOV/2) * 2
```

Debug coordinates preserved the viewing **angle** correctly but not the **distance**.

## Key Takeaway

Debug output reflects **when** a screenshot was taken — which may be during
close-up inspection, not optimal framing. Always separate angle (from debug) from
distance (calculated from composition goals).

## Related

- [Camera Positioning Lessons Learned](camera-positioning-lessons-learned.md) — overview
- [Guidelines & Prevention](camera-positioning-guidelines.md) — solution and checklist
- [Enhanced Debug Output](enhanced-debug-output.md) — full overlay field reference
