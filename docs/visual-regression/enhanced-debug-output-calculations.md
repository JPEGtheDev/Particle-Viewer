---
title: Enhanced Debug Output — Calculations & Examples
description: Coverage formulas, recommended distance calculations, and worked examples for the --debug-camera composition analysis fields.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debug, opengl]
related:
  - docs/visual-regression/enhanced-debug-output.md
  - docs/visual-regression/enhanced-debug-output-overlay-fields.md
  - docs/visual-regression/enhanced-debug-output-use-cases.md
---

# Calculations & Examples

## Estimated Coverage

With COM active:

```text
coverage = (subject_size / distance_to_COM) / tan(FOV/2)
```

Without COM:

```text
coverage = (subject_size / reference_distance) / tan(FOV/2)
# reference_distance = 50.0 units (default assumption)
```

Assumptions:
- Subject size: ~12 units (typical particle cube, `transScale=0.25`)
- Subject centered in view
- Simplified projection (does not fully account for 3D depth)

## Recommended Distances

```text
distance = subject_size / (coverage_percent * tan(FOV/2))
```

For 45° FOV and a 12-unit subject:

| Target Coverage | Distance |
|-----------------|----------|
| 50% | ~51.4 units |
| 40% | ~64.3 units |

## Accuracy Limitations

- Estimates assume the subject is **centered** in view.
- Calculations use a **simplified subject size** (12 units).
- Without COM, uses a **default reference** (50 units) as an estimate.
- Actual coverage varies with subject shape, 3D depth, and off-center position.

Use these as **guidelines**, not exact values. Iterate based on visual results.

## Worked Examples

### Example 1: Zoomed In (With COM)

```text
Dist to COM: 29.23 units
Est. Coverage: ~75% of viewport
For 40% coverage: dist=64.28 units
```

Camera is close; subject fills most of the frame. Move to ~64 units for a balanced baseline.

### Example 2: Well-Framed (With COM)

```text
Dist to COM: 51.16 units
Est. Coverage: ~43% of viewport
For 50% coverage: dist=51.43 units
```

Good framing — close to ideal for a regression baseline.

### Example 3: Default Particle Cube (Without COM)

```text
Front: (0.58, -0.37, 0.72)
Est. Coverage: ~43% (ref=50.0u)
For 50% coverage: dist=51.43 units
For 40% coverage: dist=64.28 units
```

No simulation loaded; COM is (0,0,0). Use **Front** to preserve viewing direction,
then pick a distance from the table above.

### Example 4: Too Far Away (With COM)

```text
Dist to COM: 100.00 units
Est. Coverage: ~22% of viewport
For 50% coverage: dist=51.43 units
```

Subject is small with too much blank space. Move closer to ~51 units.

## Related

- [Enhanced Debug Output (overview)](enhanced-debug-output.md) — navigation and quick reference
- [Overlay Fields](enhanced-debug-output-overlay-fields.md) — field definitions
- [Camera Positioning Lessons Learned](camera-positioning-lessons-learned.md) — pitfalls when using debug output for baselines
