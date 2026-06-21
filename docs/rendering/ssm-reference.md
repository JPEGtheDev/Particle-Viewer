# SSM Rendering — Quick Reference

Screen-Space Metaball parameters and known gotchas for the Particle-Viewer SSM pipeline.

## Pipeline Stages

```
Depth prepass (metaball_depth.frag)
  -> Splat pass — additive GL_RGBA32F FBO (metaball_splat.frag)
  -> H + V separable Gaussian blur (metaball_blur.frag)
  -> Composite — binary threshold discard (metaball_composite.frag)
```

## Production Parameters (frame ~360, N=65536 collision simulation)

| Parameter | Uniform / field | Recommended value | Notes |
|-----------|----------------|-------------------|-------|
| Blob Radius | `blobRadius` / `ssm_blob_radius` | 2.0 | World-space display units |
| Scale | `scale` | 250.0 | Multiplies sprite size |
| Blur Amount | `blurAmount` / `ssm_blur_amount` | **29 at 4K** / 10 at 720p | See resolution scaling below |
| Threshold | `threshold` / `ssm_threshold` | 0.1 | Lower = fewer star-hole gaps |
| Depth Cull Range | `u_depth_cull_range` | `blobRadius * 0.5` = 1.0 | Code: `viewer_app.cpp drawSSMScene()` |

## Depth Cull Range

Controls how many display-units behind the front surface can contribute density. Tighter = less material bleed-through from inner layers.

- **`blobRadius * 3.0`** (old default): iron cores bleed through silicate shells
- **`blobRadius * 0.5`** (current): best balance between occlusion and surface completeness
- Going below 0.5x blobRadius produces no visible improvement; statistics plateau

Set in `src/viewer_app.cpp`, `drawSSMScene()`:
```cpp
glUniform1f(..., "u_depth_cull_range"), window_.ssm_blob_radius * 0.5f);
```

## Blur Resolution Scaling

`blurAmount` covers ±N **pixels** regardless of viewport resolution. At production (3840×2108) sprites are ~2.93x larger in pixels than at test (1280×720), so inter-particle gaps are proportionally larger. The same `blurAmount` that fills gaps at 720p leaves visible star-hole artifacts at 4K.

**Scale rule:**
```
blur_production = blur_test * (production_height / 720.0)
               = 10        * (2108 / 720)
               ≈ 29
```

Parameters that do NOT need scaling between resolutions: `blobRadius`, `threshold`, `u_depth_cull_range` — all world-space or dimensionless.

## FBO Texture Wrap Mode

All three SSM FBOs (density, intermediate, blur) must use `GL_CLAMP_TO_EDGE`. The default `GL_REPEAT` causes the Gaussian blur kernel to wrap-sample from the opposite screen edge, producing a color bleed stripe at viewport boundaries.

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
```

Required on all 3 textures in both `viewer_app.cpp` (initSSM) and the VR tests.

## Shader File Workflow

Shaders are copied from `src/shaders/` to `Viewer-Assets/shaders/` at **configure time**, not build time.

After editing any `.vert` or `.frag` file:
```bash
cmake -B build -S .          # reconfigure — copies shaders
cmake --build build           # build
```

`cmake --build build` alone will NOT pick up shader edits. The old shader stays in `Viewer-Assets/shaders/` until reconfigure runs.

## Color Blending Limitation

SSM uses depth-integrated color averaging: `color = blurred.rgb / blurred.a`. When two material types occupy the same depth band (within `u_depth_cull_range`), their colors average. This is fundamental to screen-space metaballs.

Tightening `u_depth_cull_range` reduces but cannot eliminate blending when materials are genuinely spatially interleaved. Per-material separation requires either:
- 4 separate density passes (one per category) composited front-to-back, or
- Marching cubes (issue #124) — solid geometry occludes interior materials by construction

## Particle Category Encoding

In the binary `PosAndVel` file, `w` component encodes material category as a float:

| w value | Category | Color |
|---------|----------|-------|
| 0.0 | Fe body 1 | Red `(1, 0, 0)` |
| 1.0 | Si body 1 | Blue `(0.2, 0.6, 1.0)` |
| 2.0 | Fe body 2 | Magenta `(1, 0, 1)` |
| 3.0 | Si body 2 | Orange `(0.89, 0.59, 0)` |
| 500.0 | Debug/rainbow | Per-instance index hash |

`kSimToDisplayScale = 0.25` — multiply raw km-scale positions by 0.25 for display space. Camera coordinates in the UI debug overlay are in display space.
