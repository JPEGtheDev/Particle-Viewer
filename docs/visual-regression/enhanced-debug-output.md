---
title: Enhanced Debug Camera Output
description: Overview and navigation guide for the --debug-camera overlay, which provides camera positioning, composition analysis, and visual regression test setup information.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debug, opengl]
related:
  - docs/visual-regression/enhanced-debug-output-overlay-fields.md
  - docs/visual-regression/enhanced-debug-output-use-cases.md
  - docs/visual-regression/enhanced-debug-output-calculations.md
  - docs/visual-regression/camera-positioning-lessons-learned.md
---

# Enhanced Debug Camera Output

The `--debug-camera` (or `-d`) flag enables a comprehensive camera overlay for positioning, composition analysis, and visual regression test setup.

## Contents

| File | What it covers |
|------|----------------|
| [Overlay Fields](enhanced-debug-output-overlay-fields.md) | All fields displayed — original and new additions |
| [Use Cases](enhanced-debug-output-use-cases.md) | Workflows for regression setup, camera state analysis, and composition planning |
| [Calculations & Examples](enhanced-debug-output-calculations.md) | Coverage formulas, distance calculations, and worked examples |

## Quick Reference

- **Target** = `Pos + Front` (lookat point, always 1 unit away — not your subject)
- **COM** metrics only appear when center-of-mass is active (non-zero)
- Coverage estimates use a simplified 12-unit subject size
- Use **Dir to COM** to preserve viewing angle; recalculate distance for framing

## Related

- [Overlay Fields](enhanced-debug-output-overlay-fields.md) — full field reference for the debug overlay
- [Use Cases](enhanced-debug-output-use-cases.md) — step-by-step workflows
- [Calculations & Examples](enhanced-debug-output-calculations.md) — formulas and worked examples
- [Camera Positioning Lessons Learned](camera-positioning-lessons-learned.md) — pitfalls when using debug output for baselines
