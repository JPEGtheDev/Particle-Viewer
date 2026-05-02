---
title: Camera Positioning Guidelines & Prevention
description: Solution applied to fix visual regression test #68, lessons learned, and a checklist for setting up camera positions in future tests.
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, camera, opengl]
related:
  - docs/visual-regression/camera-positioning-lessons-learned.md
  - docs/visual-regression/camera-positioning-root-cause.md
  - docs/visual-regression/enhanced-debug-output.md
---

# Camera Positioning Guidelines & Prevention

## Solution Applied

The fix preserved the viewing **direction** from debug output and recalculated
camera **distance** based on desired composition.

### Calculation

```python
# Original camera distance to cube center
dist_original = 29.23  # units

# Scale factor for desired framing
scale_factor = 1.75

# New camera distance
dist_new = dist_original * scale_factor  # = 51.16 units

# New position = cube_center - direction * dist_new
# Result: (-23.60, 25.21, -30.93)
```

### Outcome

- Cube fills ~40–50% of viewport ✓
- Balanced blank space around cube ✓
- Same viewing angle preserved ✓
- Matches reference image composition ✓

## Lessons Learned

### For Visual Regression Tests

1. **Don't blindly copy debug coordinates** — they may represent a zoomed-in view.
2. **Separate angle from distance** — preserve direction from debug; recalculate distance.
3. **Define composition goals first** — what percentage of the viewport should the subject fill?

### For Debug Output Users

1. **Trust debug calculations** — they accurately report camera state.
2. **Consider context** — was the camera zoomed in or panned at capture time?
3. **Use debug as a starting point** — extract direction and up vector, then recalculate distance.

## Prevention Checklist

- [ ] Identify desired composition (viewport coverage %)
- [ ] Measure subject size in world units
- [ ] Extract viewing direction from debug or reference
- [ ] Calculate ideal camera distance using the formula below
- [ ] Generate test baseline
- [ ] Visually compare to reference
- [ ] Adjust if needed (typical scale factor: 1.5× – 2.0×)

## Formula Reference

```python
# Camera distance for desired composition
distance = subject_size / (coverage_percent * tan(FOV_radians / 2))

# Example: 12-unit cube, 45° FOV, 45% coverage
# distance ≈ 51 units
```

| Coverage | Distance (12-unit cube, 45° FOV) |
|----------|----------------------------------|
| 50% | ~51.4 units |
| 40% | ~64.3 units |

## Related

- [Camera Positioning Lessons Learned](camera-positioning-lessons-learned.md) — overview
- [Root Cause Analysis](camera-positioning-root-cause.md) — what went wrong
- [Enhanced Debug Output](enhanced-debug-output.md) — debug overlay reference for composition analysis
