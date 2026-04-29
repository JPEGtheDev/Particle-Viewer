---
title: Enhanced Debug Output — Overlay Fields
description: Reference for all fields displayed by the --debug-camera overlay, including camera orientation, distance metrics, simulation tracking, and composition analysis.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debug, opengl]
related:
  - docs/visual-regression/enhanced-debug-output.md
  - docs/visual-regression/enhanced-debug-output-use-cases.md
  - docs/visual-regression/enhanced-debug-output-calculations.md
---

# Overlay Fields

## Original Fields

| Field | Description |
|-------|-------------|
| **Pos** | Camera position in world space |
| **Target** | Lookat point (`Pos + Front`) |
| **Up** | Camera up vector |
| **Proj** | Projection type, FOV, near/far planes |
| **View** | Viewport dimensions |

## New Fields

### Camera Orientation

| Field | Description |
|-------|-------------|
| **Front** | Normalized direction vector the camera faces |
| **Yaw** | Horizontal rotation angle (degrees) |
| **Pitch** | Vertical rotation angle (degrees) |

### Distance Metrics

| Field | Description |
|-------|-------------|
| **Dist to Target** | Always 1.0 unit (`Target = Pos + Front`) |

### Simulation Tracking (conditional — COM must be active/non-zero)

| Field | Description |
|-------|-------------|
| **COM** | Center of mass position |
| **Dist to COM** | Distance from camera to COM |
| **Dir to COM** | Normalized direction vector toward COM |

> COM is **not** used for viewport rendering. These fields only appear when a
> simulation with a non-zero COM is loaded.

### Composition Analysis

| Field | Description |
|-------|-------------|
| **Est. Coverage** | Estimated percentage of viewport filled by subject |
| **For 50% coverage** | Recommended camera distance for 50% fill |
| **For 40% coverage** | Recommended camera distance for 40% fill |

When COM is active, coverage uses distance to COM. Otherwise, a default reference
distance of 50 units is assumed.

## Understanding "Target"

**Target** (`Pos + Front`) shows the lookat point — always 1 unit away. It is
**not** the subject position. Do not confuse it with COM or the rendered particle
center.

## Related

- [Enhanced Debug Output (overview)](enhanced-debug-output.md) — navigation and quick reference
- [Use Cases](enhanced-debug-output-use-cases.md) — how to use these fields in practice
- [Calculations & Examples](enhanced-debug-output-calculations.md) — formulas behind coverage estimates
