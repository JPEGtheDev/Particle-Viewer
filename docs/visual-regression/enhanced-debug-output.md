# Enhanced Debug Camera Output

## Overview

The debug camera overlay (`--debug-camera` or `-d` flag) now provides comprehensive information for camera positioning, composition analysis, and visual regression test setup.

## Enhanced Information

### Original Information
- **Pos**: Camera position in world space
- **Target**: Where camera is looking (cameraPos + cameraFront)
- **Up**: Camera up vector
- **Proj**: Projection type, FOV, near/far planes
- **View**: Viewport dimensions

### New Information (Added)

#### Camera Orientation
- **Front**: Normalized direction vector camera is facing
- **Yaw**: Horizontal rotation angle in degrees
- **Pitch**: Vertical rotation angle in degrees

#### Distance Metrics
- **Dist to Target**: Distance from camera to target point (length of cameraFront)
- **COM**: Center of mass position (subject center)
- **Dist to COM**: Distance from camera to center of mass
- **Dir to COM**: Normalized direction vector from camera to COM

#### Composition Analysis (for Visual Regression Tests)
- **Est. Coverage**: Estimated percentage of viewport filled by subject
- **For 50% coverage**: Recommended camera distance for 50% viewport coverage
- **For 40% coverage**: Recommended camera distance for 40% viewport coverage

## Use Cases

### 1. Visual Regression Test Setup

When setting up a visual regression test:

1. **Launch application** with debug flag
2. **Navigate to desired viewing angle** (get the direction/angle right)
3. **Note the "Dir to COM"** - this preserves your viewing angle
4. **Check "Est. Coverage"** - is subject too large/small?
5. **Use recommended distances** - adjust camera distance for desired framing

**Example workflow:**
```
Current view:
- Dist to COM: 29.23 units
- Est. Coverage: ~75% of viewport (too large)
- For 40% coverage: dist=51.16 units (better for baseline)

Action: Move camera to ~51 units while keeping same direction
```

### 2. Understanding Camera State

The enhanced output helps you understand:
- **Where you're looking** (Target vs COM) - are you looking AT the subject or PAST it?
- **How close you are** (Dist to COM) - zoomed in for detail or pulled back for overview?
- **How it will frame** (Est. Coverage) - will subject fill most of frame or have blank space?

### 3. Composition Planning

The coverage estimates and recommended distances help plan composition:
- **50% coverage**: Subject fills half the viewport (comfortable framing)
- **40% coverage**: Subject fills 40% with balanced blank space (typical for baselines)
- **Current coverage**: See if you need to zoom in/out

## Calculation Details

### Estimated Coverage
```
coverage = (subject_size / distance_to_COM) / tan(FOV/2)
```

Assumes:
- Subject size: ~12 units (typical particle cube after transScale=0.25)
- Subject centered in view
- Simplified projection (doesn't account for 3D depth fully)

### Recommended Distances
```
distance = subject_size / (coverage_percent * tan(FOV/2))
```

For 45° FOV and 12-unit subject:
- 50% coverage: ~51.4 units
- 40% coverage: ~64.3 units

## Important Notes

### For Visual Regression Tests

⚠️ **Key Lesson**: Debug output shows WHERE you're looking, not WHAT you're framing.

When creating visual regression baselines:
1. ✅ **DO** extract viewing direction (Front, Dir to COM)
2. ✅ **DO** calculate distance using coverage recommendations
3. ❌ **DON'T** blindly copy Pos/Target coordinates
4. ❌ **DON'T** assume current distance is optimal for baseline

See `docs/visual-regression/camera-positioning-lessons-learned.md` for detailed analysis.

### Accuracy Limitations

- Coverage estimates assume subject is **centered** in view
- Calculations use **simplified subject size** (12 units)
- Actual coverage may vary based on:
  - Subject actual size and shape
  - 3D depth and perspective effects
  - Subject position relative to camera

Use estimates as **guidelines**, not exact values. Iterate based on visual results.

## Examples

### Example 1: Zoomed In View
```
Dist to COM: 29.23 units
Est. Coverage: ~75% of viewport
For 40% coverage: dist=64.28 units
```
**Interpretation**: Camera is close, subject fills most of frame. To get more blank space around subject (better for baseline), move to ~64 units.

### Example 2: Well-Framed View
```
Dist to COM: 51.16 units
Est. Coverage: ~43% of viewport
For 50% coverage: dist=51.43 units
```
**Interpretation**: Good framing with balanced composition. Close to ideal for visual regression baseline.

### Example 3: Too Far Away
```
Dist to COM: 100.00 units
Est. Coverage: ~22% of viewport
For 50% coverage: dist=51.43 units
```
**Interpretation**: Subject is small with lots of blank space. Move closer to ~51 units for better framing.

## Technical References

- Implementation: `src/debugOverlay.hpp`
- Camera getters: `src/camera.hpp`
- Lessons learned: `docs/visual-regression/camera-positioning-lessons-learned.md`
- Visual regression tests: `tests/visual-regression/RenderingRegressionTests.cpp`
