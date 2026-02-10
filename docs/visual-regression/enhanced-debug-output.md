# Enhanced Debug Camera Output

## Overview

The debug camera overlay (`--debug-camera` or `-d` flag) now provides comprehensive information for camera positioning, composition analysis, and visual regression test setup.

## Enhanced Information

### Original Information
- **Pos**: Camera position in world space
- **Target**: Where camera is looking (cameraPos + cameraFront) - the lookat point
- **Up**: Camera up vector
- **Proj**: Projection type, FOV, near/far planes
- **View**: Viewport dimensions

### New Information (Added)

#### Camera Orientation
- **Front**: Normalized direction vector camera is facing
- **Yaw**: Horizontal rotation angle in degrees
- **Pitch**: Vertical rotation angle in degrees

#### Distance Metrics
- **Dist to Target**: Distance from camera to target point (always 1.0 unit, as Target = Pos + Front)

#### Simulation Tracking (Conditional - only shown if COM is active/non-zero)
- **COM**: Center of mass position (particle simulation tracking)
- **Dist to COM**: Distance from camera to center of mass
- **Dir to COM**: Normalized direction vector from camera to COM

**Note**: COM (Center of Mass) is **optional** and only used for tracking particle simulations. If COM is not specified or is at origin (0,0,0), these metrics won't be displayed as they're not relevant for viewport rendering.

#### Composition Analysis (for Visual Regression Tests)
- **Est. Coverage**: Estimated percentage of viewport filled by subject
  - When COM is active: calculated using distance to COM
  - When COM is not active: estimated using default reference distance
- **For 50% coverage**: Recommended camera distance for 50% viewport coverage
- **For 40% coverage**: Recommended camera distance for 40% viewport coverage

## Understanding "Target"

**Target** is the point where the camera is looking at, calculated as `Pos + Front`. It's always 1 unit away from the camera position in the direction the camera is facing. This is **not** necessarily where your subject is—it's just the lookat point used by the camera's view matrix.

For visual regression tests, don't confuse Target with the subject center. Target shows WHERE you're looking, not WHAT you're framing.

## Use Cases

### 1. Visual Regression Test Setup (With COM)

When setting up a visual regression test with simulation data (COM active):

1. **Launch application** with debug flag and load simulation
2. **Navigate to desired viewing angle** (get the direction/angle right)
3. **Note the "Dir to COM"** - this preserves your viewing angle toward the subject
4. **Check "Est. Coverage"** - is subject too large/small?
5. **Use recommended distances** - adjust camera distance for desired framing

**Example workflow:**
```
Current view (with COM active):
- Dist to COM: 29.23 units
- Est. Coverage: ~75% of viewport (too large)
- For 40% coverage: dist=51.16 units (better for baseline)

Action: Move camera to ~51 units while keeping same direction
```

### 2. Visual Regression Test Setup (Without COM)

When setting up tests without simulation data (default particle cube, COM not active):

1. **Launch application** with debug flag
2. **Navigate to desired viewing angle**
3. **Note Front vector** - preserves viewing direction
4. **Check "Est. Coverage (est.)"** - rough estimate based on default reference
5. **Use recommended distances** - calculate proper camera distance

**Note**: Without COM, coverage is estimated. You'll need to iterate based on visual results or manually calculate the subject center position.

### 3. Understanding Camera State

The enhanced output helps you understand:
- **Where you're looking** (Target) - the lookat point (Pos + Front), always 1 unit away
- **Subject position** (COM if active) - where the actual particles/subject are
- **Are you looking AT or PAST the subject?** (compare Target vs COM)
- **How close you are** (Dist to COM if available) - zoomed in or pulled back?
- **How it will frame** (Est. Coverage) - will subject fill most of frame or have blank space?

### 4. Composition Planning

The coverage estimates and recommended distances help plan composition:
- **50% coverage**: Subject fills half the viewport (comfortable framing)
- **40% coverage**: Subject fills 40% with balanced blank space (typical for baselines)
- **Current coverage**: See if you need to zoom in/out

## Calculation Details

### Estimated Coverage

**When COM is active:**
```
coverage = (subject_size / distance_to_COM) / tan(FOV/2)
```

**When COM is not active:**
```
coverage = (subject_size / reference_distance) / tan(FOV/2)
# where reference_distance = 50.0 units (default assumption)
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

### COM (Center of Mass) Usage

⚠️ **COM is optional**: COM metrics are only shown when COM is active (non-zero). COM is used for:
- Tracking particle simulations
- Optional simulation analysis

COM is **NOT** used for viewport rendering. If you're working with the default particle cube (no simulation loaded), COM will be (0,0,0) and COM-related metrics won't be displayed.

**When COM is shown**: The composition analysis uses distance to COM as the reference.
**When COM is not shown**: The composition analysis uses a default reference distance (50 units) as an estimate.

### For Visual Regression Tests

⚠️ **Key Lesson**: Debug output shows WHERE you're looking (Target), not WHAT you're framing (subject).

**Target vs Subject:**
- **Target**: Always Pos + Front, the lookat point, 1 unit away
- **Subject**: Could be at COM (if active), or at a calculated position based on rendered particles
- These are often **different locations**!

When creating visual regression baselines:
1. ✅ **DO** extract viewing direction (Front vector)
2. ✅ **DO** calculate distance using coverage recommendations
3. ✅ **DO** use Dir to COM if COM is active
4. ❌ **DON'T** blindly copy Pos/Target coordinates
5. ❌ **DON'T** assume current distance is optimal for baseline
6. ❌ **DON'T** rely on COM if it's not being used

See `docs/visual-regression/camera-positioning-lessons-learned.md` for detailed analysis.

### Accuracy Limitations

- Coverage estimates assume subject is **centered** in view
- Calculations use **simplified subject size** (12 units)
- Without COM, uses **default reference distance** (50 units estimate)
- Actual coverage may vary based on:
  - Subject actual size and shape
  - 3D depth and perspective effects
  - Subject position relative to camera

Use estimates as **guidelines**, not exact values. Iterate based on visual results.

## Examples

### Example 1: Zoomed In View (With COM)
```
--- Simulation Tracking ---
COM: (6.09, 6.09, 6.09)
Dist to COM: 29.23 units
--- Composition (using COM) ---
Est. Coverage: ~75% of viewport
For 40% coverage: dist=64.28 units
```
**Interpretation**: Camera is close, subject fills most of frame. To get more blank space around subject (better for baseline), move to ~64 units.

### Example 2: Well-Framed View (With COM)
```
--- Simulation Tracking ---
COM: (6.09, 6.09, 6.09)
Dist to COM: 51.16 units
--- Composition (using COM) ---
Est. Coverage: ~43% of viewport
For 50% coverage: dist=51.43 units
```
**Interpretation**: Good framing with balanced composition. Close to ideal for visual regression baseline.

### Example 3: Default Particle Cube (Without COM)
```
Target: (-15.99, 16.02, -8.04)
  (lookat point: Pos + Front)
Front: (0.58, -0.37, 0.72)
--- Composition (est.) ---
Est. Coverage: ~43% (ref=50.0u)
For 50% coverage: dist=51.43 units
For 40% coverage: dist=64.28 units
```
**Interpretation**: Working with default particle cube, no simulation loaded. COM is (0,0,0) so not displayed. Coverage is estimated using default reference. Use Front vector to maintain viewing angle, then calculate proper distance based on desired framing.

### Example 4: Too Far Away (With COM)
```
--- Simulation Tracking ---
COM: (6.09, 6.09, 6.09)
Dist to COM: 100.00 units
--- Composition (using COM) ---
Est. Coverage: ~22% of viewport
For 50% coverage: dist=51.43 units
```
**Interpretation**: Subject is small with lots of blank space. Move closer to ~51 units for better framing.

## Technical References

- Implementation: `src/debugOverlay.hpp`
- Camera getters: `src/camera.hpp`
- Lessons learned: `docs/visual-regression/camera-positioning-lessons-learned.md`
- Visual regression tests: `tests/visual-regression/RenderingRegressionTests.cpp`
