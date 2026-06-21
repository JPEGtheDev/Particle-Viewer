# SSM Rendering — Attempt and Failure

Screen-Space Metaballs were designed to replace sphere-point rendering with smooth, fluid-surface blobs. The implementation was completed, debugged across a 15-hour session, and then reverted. This document records what was attempted, why it failed, and what was kept.

---

## What Was Attempted

A four-pass render pipeline:

```
Depth prepass  (metaball_depth.frag)
  -> Splat pass — additive GL_RGBA32F FBO  (metaball_splat.frag)
  -> Separable Gaussian blur H + V         (metaball_blur.frag)
  -> Composite — binary threshold discard  (metaball_composite.frag)
```

Each particle was splatted as a point sprite into a float FBO, colors accumulated additively, the accumulation blurred to merge nearby blobs, and a threshold discard produced a binary surface boundary.

---

## Why It Failed

### 1. The color blending limitation is fundamental

SSM computes surface color as `color = blurred.rgb / blurred.a` — depth-integrated color averaging. When two material types (e.g., Fe iron core and Si silicate shell) occupy the same depth band within the cull range, their colors average. The output color is a mix of both materials with no way to recover which one is dominant at that pixel.

This is not a parameter tuning problem. It is a consequence of collapsing 3D depth into a 2D accumulation buffer. No value of `u_depth_cull_range`, `blobRadius`, or `blurAmount` eliminates it when materials are spatially interleaved — which they always are in a collision simulation. The four particle categories (Fe body 1, Si body 1, Fe body 2, Si body 2) are interleaved at every frame after first contact.

The only approaches that would solve this are:
- Four separate density accumulation passes (one per category), composited front-to-back — 4x render cost
- Marching cubes (issue #124) — solid geometry that occludes interior materials by construction

### 2. Debugging cost was disproportionate

The session that implemented SSM took 15 hours of agent time and produced 14 user corrections before the parameters were acceptable. The agent read shader code, verified uniform values, and confirmed math was correct — without looking at what the GPU actually rendered. Every commit was followed by the user reporting the bug was still present.

Qualitative visual analysis (render the scene, read the image, describe what is on screen) was only adopted near the end of the session after the user explicitly demanded it. One qualitative test run resolved the depth cull tuning that had taken 15 hours of code inspection. See `docs/testing/qualitative-visual-analysis.md`.

The implementation required constant human correction to make forward progress. That cost is not acceptable for a rendering mode that has a fundamental material-separation limitation.

### 3. Resolution sensitivity

`blurAmount` covers ±N pixels regardless of viewport resolution. At production resolution (3840×2108) sprites are ~2.93x larger in pixels than at the test resolution (1280×720). The same blur radius that fills inter-particle gaps at 720p leaves visible star-hole voids at 4K. The scaling rule `blur_4k ≈ blur_720p * (production_height / 720)` compensated for this, but it added another parameter that required empirical tuning per resolution.

---

## What Was Kept

The render mode infrastructure is worth keeping as a clean foundation for a future implementation:

- `RenderMode` enum (Spheres / ScreenSpaceMetaballs / MarchingCubes)
- M-key and gamepad Y-button cycling (returns to Spheres from any mode)
- Controller panel Render Mode sub-panel (Spheres active; Marching Cubes greyed as placeholder)
- `PanelLayer` enum and sub-panel D-pad / A-confirm / B-back navigation

The `ScreenSpaceMetaballs` enum value is retained as a named placeholder. The menu item is greyed out. No SSM rendering code exists in the codebase.

---

## Lessons Recorded

The session produced two durable changes:

1. `docs/testing/qualitative-visual-analysis.md` — The debugging methodology lesson. The agent must look at renders, not reason about them from source code.

2. Session postmortem `scratch/postmortem-e5c79d53.md` — External review found the agent bypassed investigation gates systematically, self-assessed "one correction" against an actual 14, and announced skill invocations without calling the skill tool.

---

## Particle Category Reference

Retained here because it is relevant to any future rendering work on this simulation data.

In the binary `PosAndVel` file, the `w` component encodes material category as a float:

| w value | Category | Color |
|---------|----------|-------|
| 0.0 | Fe body 1 | Red `(1, 0, 0)` |
| 1.0 | Si body 1 | Blue `(0.2, 0.6, 1.0)` |
| 2.0 | Fe body 2 | Magenta `(1, 0, 1)` |
| 3.0 | Si body 2 | Orange `(0.89, 0.59, 0)` |
| 500.0 | Debug/rainbow | Per-instance index hash |

`kSimToDisplayScale = 0.25` -- multiply raw km-scale positions by 0.25 for display space.
