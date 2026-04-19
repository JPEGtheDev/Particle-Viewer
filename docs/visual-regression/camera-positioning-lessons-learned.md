---
title: Visual Regression Camera Positioning — Lessons Learned
description: Analysis of a camera positioning mistake when creating visual regression baselines, and guidelines for correct camera setup in future tests.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, camera, opengl]
related:
  - docs/visual-regression/camera-positioning-root-cause.md
  - docs/visual-regression/camera-positioning-guidelines.md
  - docs/visual-regression/enhanced-debug-output.md
---

# Visual Regression Camera Positioning: Lessons Learned

When implementing the particle cube visual regression test (#68), using debug camera
coordinates directly produced incorrect composition. This document is a navigation
guide to the analysis and guidelines.

## The Core Insight

> **Debug output shows WHERE you're looking, not WHAT you're framing.**

For regression baselines, calculate camera distance based on desired composition —
not debug coordinates from an arbitrary camera state.

## Contents

| File | What it covers |
|------|----------------|
| [Root Cause Analysis](camera-positioning-root-cause.md) | What went wrong, why, and the composition gap |
| [Guidelines & Prevention](camera-positioning-guidelines.md) | Solution applied, lessons learned, and a setup checklist |

## Quick Summary

- Debug coordinates are **accurate** — they report camera state correctly.
- They represent **current state**, not **optimal framing**.
- Viewing angle/direction: extract from debug output.
- Viewing distance: **calculate** from composition goals.

## Related

- [Root Cause Analysis](camera-positioning-root-cause.md) — what went wrong and why
- [Guidelines & Prevention](camera-positioning-guidelines.md) — lessons and checklist
- [Enhanced Debug Output](enhanced-debug-output.md) — full debug overlay reference
