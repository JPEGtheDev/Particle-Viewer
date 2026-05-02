---
title: Enhanced Debug Output — Use Cases
description: Step-by-step workflows for using the --debug-camera overlay to set up visual regression tests, analyze camera state, and plan composition.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debug, opengl]
related:
  - docs/visual-regression/enhanced-debug-output.md
  - docs/visual-regression/enhanced-debug-output-calculations.md
  - docs/visual-regression/camera-positioning-lessons-learned.md
---

# Use Cases

## 1. Visual Regression Test Setup (With COM)

When setting up a test with simulation data (COM active):

1. Launch with `--debug-camera` and load the simulation.
2. Navigate to the desired viewing angle.
3. Note **Dir to COM** — this preserves your viewing direction toward the subject.
4. Check **Est. Coverage** — is the subject too large or too small?
5. Use the recommended distances to reach the desired framing.

Example:

```text
Dist to COM: 29.23 units
Est. Coverage: ~75% of viewport (too large)
For 40% coverage: dist=64.28 units
```

Action: Move camera to ~64 units along the same direction.

## 2. Visual Regression Test Setup (Without COM)

When working with the default particle cube (no simulation, COM = (0,0,0)):

1. Launch with `--debug-camera`.
2. Navigate to the desired angle.
3. Note the **Front** vector to preserve viewing direction.
4. Check **Est. Coverage (est.)** — rough estimate using 50-unit reference.
5. Use recommended distances as a starting point; iterate on visual results.

## 3. Understanding Camera State

The overlay answers these questions:

| Question | Field to check |
|----------|----------------|
| Where am I looking? | **Target** (lookat point = Pos + Front) |
| Where is my subject? | **COM** (if active) |
| Am I looking at or past the subject? | Compare **Target** vs **COM** |
| How close am I? | **Dist to COM** |
| How will it frame? | **Est. Coverage** |

## 4. Composition Planning

Coverage targets for regression baselines:

| Coverage | Description |
|----------|-------------|
| ~50% | Subject fills half the viewport (comfortable framing) |
| ~40% | Balanced blank space — typical for baselines |

See [Calculations & Examples](enhanced-debug-output-calculations.md) for the formulas.

## Key Do's and Don'ts

| ✅ DO | ❌ DON'T |
|-------|---------|
| Extract viewing direction (Front vector) | Blindly copy Pos/Target coordinates |
| Calculate distance from coverage goals | Assume current distance is optimal |
| Use Dir to COM when COM is active | Rely on COM if it's not being used |

## Related

- [Enhanced Debug Output (overview)](enhanced-debug-output.md) — navigation and quick reference
- [Calculations & Examples](enhanced-debug-output-calculations.md) — formulas and worked examples
- [Camera Positioning Lessons Learned](camera-positioning-lessons-learned.md) — pitfalls and analysis
