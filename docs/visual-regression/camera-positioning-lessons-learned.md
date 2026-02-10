# Visual Regression Test Camera Positioning: Lessons Learned

## Issue Summary

When implementing the particle cube visual regression test (#68), using debug camera output coordinates directly resulted in incorrect composition. This document analyzes the root cause and provides guidance for future visual regression tests.

## The Problem

### Reference Image
- Cube well-framed with ~50% blank space around it
- Appeared at ideal viewing distance
- Debug overlay showed: Pos(-16.72, 16.49, -8.95), Target(-15.99, 16.02, -8.04)

### Initial Test Output (using debug coordinates)
- Cube too large, filling ~80% of viewport
- Minimal blank space
- Appeared much closer than intended

## Root Cause Analysis

### 1. Debug Output Was CORRECT ✓

The debug overlay (`src/debugOverlay.hpp`) accurately reported:
- Camera position: Where the camera is located
- Camera target: Where the camera is looking (via `cameraPos + cameraFront`)
- Up vector: Camera orientation

**No bugs in debug output code.** All calculations were mathematically correct.

### 2. Incorrect Assumption ✗

**What was assumed:**
> "Debug coordinates from the reference image will produce the same visual composition in the test"

**Reality:**
- Debug coordinates showed a specific camera state during user interaction
- User had navigated close to the cube for a detailed view
- Camera was looking PAST the cube center (target 28 units from cube)
- Camera was only 29 units from cube center (too close for desired framing)

### 3. Missing Context

Debug output shows **WHERE** the camera is looking, not **WHAT** is being framed.

Visual composition depends on:
- **Distance from camera to subject**: 29 units (debug) vs. 51 units (needed)
- **Field of view**: 45° (fixed)
- **Subject size in viewport**: ~12 unit cube
- **Viewing angle**: Preserved across both setups

**Viewport coverage formula (simplified):**
```
coverage ≈ (subject_size / distance) * tan(FOV/2) * 2
```

| Setup | Distance | Coverage | Result |
|-------|----------|----------|--------|
| Debug coords | 29 units | ~70-80% | Cube too large |
| Adjusted | 51 units | ~40-50% | Cube well-framed |

### 4. The Actual Issue

The debug coordinates represented:
- A **specific moment** during user interaction
- A **zoomed-in view** for detailed inspection
- **NOT** an optimal framing for visual regression baseline

The test needed:
- **Optimal framing** with balanced composition
- **Consistent** baseline for detecting regressions
- **Representative** view of default scene

## Solution Applied

### Approach
1. Extracted viewing **direction** from debug coordinates (preserved angle)
2. Calculated proper **distance** based on desired composition
3. Moved camera 1.75x further back along same viewing axis

### Calculation
```python
# Original camera distance to cube
dist_original = 29.23 units

# Scale factor for desired framing
scale_factor = 1.75

# New camera distance
dist_new = dist_original * scale_factor = 51.16 units

# New position = cube_center - direction * dist_new
# Result: (-23.60, 25.21, -30.93)
```

### Result
- Cube fills ~40-50% of viewport ✓
- Balanced blank space around cube ✓
- Same viewing angle preserved ✓
- Matches reference image composition ✓

## Lessons Learned

### For Visual Regression Tests

1. **Don't blindly copy debug coordinates**
   - Debug output is accurate but may not represent ideal framing
   - Analyze the composition requirements first

2. **Separate angle from distance**
   - Viewing angle/direction: Usually matches debug output
   - Viewing distance: Calculate based on desired composition

3. **Consider composition goals**
   - What % of viewport should subject fill?
   - How much blank space is appropriate?
   - Is this a close-up or overview shot?

4. **Calculate ideal camera distance**
   ```python
   # For desired viewport coverage
   ideal_distance = subject_size / (coverage_ratio * tan(FOV/2))
   
   # Example: 12-unit cube, 45° FOV, 45% coverage
   ideal_distance ≈ 51 units
   ```

5. **Iterate if needed**
   - Generate baseline with calculated position
   - Visually compare to reference
   - Adjust scale factor if needed (1.5x - 2.0x typical range)

### For Debug Output Users

1. **Debug coordinates are accurate** ✓
   - Trust the debug overlay calculations
   - They correctly report camera state

2. **Context matters**
   - Debug shows current state, not ideal state
   - Consider what the user was doing when screenshot was taken
   - Was camera zoomed in/out? Panned? Rotated?

3. **Use debug as starting point**
   - Extract viewing direction
   - Extract orientation (up vector)
   - **Recalculate distance** based on requirements

## Prevention for Future Tests

### Checklist
- [ ] Identify desired composition (viewport coverage %)
- [ ] Calculate subject size in world units
- [ ] Extract viewing direction from debug/reference
- [ ] Calculate ideal camera distance using formula
- [ ] Generate test baseline
- [ ] Visually compare to reference
- [ ] Adjust if needed (typical range: 1.5x - 2.0x scale factor)

### Formula Reference
```python
# Camera distance for desired composition
distance = subject_size / (coverage_percent * tan(FOV_radians / 2))

# Where:
# - subject_size: Object dimensions in world units
# - coverage_percent: Desired viewport fill (0.4 - 0.5 typical)
# - FOV_radians: Field of view in radians (45° = 0.785 rad)
```

## References

- Issue: #68 Render Particle Cube from Angle Visual Regression Test
- Debug output implementation: `src/debugOverlay.hpp`
- Test implementation: `tests/visual-regression/RenderingRegressionTests.cpp`
- Related PR commits:
  - Initial (incorrect): Used debug coords directly, cube too large
  - Final (correct): Calculated ideal distance, cube well-framed

## Key Takeaway

> **Debug output shows WHERE you're looking, not WHAT you're framing.**
> 
> For visual regression tests, calculate camera distance based on desired composition,
> not debug coordinates from arbitrary camera states.
