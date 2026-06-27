## Feature Specification

### User Goal

The user examining a paused or slowly-playing N-body simulation can switch to Marching Cubes
render mode to view a geometrically accurate 3D isosurface of particle density -- with correct
silhouettes from all camera angles -- suitable for close-up inspection and high-quality recording.

Live/Freeze is about running the simulation vs. pausing to fly around: Live keeps the mesh
synchronized to each incoming timestep; Freeze locks the mesh so the user can orbit without
the surface changing.

### Observable Behaviors

**Mode Selection**

- The user can select Marching Cubes from the Render Mode sub-panel
  (Controller Panel -> RENDER MODE -> Marching Cubes)
- The user can cycle to Marching Cubes using the M key
  (Spheres -> Marching Cubes -> Spheres), provided Marching Cubes is available (GL 4.3+)
- On systems without GL 4.3 compute shaders, Marching Cubes is greyed out in the sub-panel
  with the tooltip "Requires OpenGL 4.3 compute shaders"; M key skips the greyed entry
- Screen-Space Metaballs remains in the sub-panel as a greyed-out placeholder (no change)

**Isosurface Rendering**

- In Marching Cubes mode, the user sees a solid 3D isosurface mesh rendered with Phong lighting
- Mesh vertex colors are the weighted average of contributing particle category colors
- Category 500 (rainbow) particles contribute:
  `vec3(index % 40 / 40.0, index % 1600 / 1600.0, index % 64000 / 64000.0)`
  where index is the particle's position in the buffer (same formula as the sphere vertex shader)
- The user can orbit the camera without the mesh changing geometry (camera movement does not
  trigger recompute)

**Live/Freeze Toggle**

- Default state on launch: Live
- In Live mode: the mesh recomputes automatically when the simulation advances to a new timestep
- In Freeze mode: the mesh holds its last computed state; new timesteps do not trigger recompute
- Toggle is exposed in the Render Mode sub-panel alongside the other MC parameters
- No keyboard shortcut for the toggle
- [UNCLEAR: manual refresh affordance] -- The AC says "holds until manually refreshed" but no
  UI affordance (button or shortcut) has been defined. Implementation will use a "Refresh Mesh"
  button in the sub-panel as the default choice pending future user decision
- [UNCLEAR: Freeze indicator] -- Whether a "new data available" indicator appears in Freeze mode
  when a new timestep arrives is not defined. Implementation will omit the indicator unless
  specified

**Runtime Parameter Tuning**

- Grid Resolution: 64^3 / 128^3 / 256^3 selector (default 128^3)
- Iso-value: slider range 0.0 to 2.0 (default 0.5)
- Particle Influence Radius: slider range 0.1 to 10.0 world units (default 2.0)
- All three values are persisted to window.cfg and restored on the next launch
- The parameter controls appear in the existing Render Mode sub-panel layer when Marching Cubes
  is the active mode; the sub-panel swaps its content to show MC-specific items
- On hardware where 256^3 exceeds available VRAM, the 256^3 option is greyed out and
  auto-downgraded to 128^3; a visible UI notification is shown

**Screenshots and Recording**

- Screenshots and frame recordings capture correct Marching Cubes output
- When M is pressed during an active recording session: no mode switch occurs; a
  non-recording UI notification (tooltip/overlay) indicates the key is disabled; this
  notification is NOT captured in screenshots or recordings

**Capability Unavailable**

- On systems without GL 4.3 compute shaders, Marching Cubes is greyed out with the tooltip
  "Requires OpenGL 4.3 compute shaders"
- The visual regression test for this feature issues GTEST_SKIP on such systems

### Acceptance Criteria

AC1: A user on a GL 4.3+ system can select Marching Cubes from the Render Mode sub-panel
     and sees a lit 3D mesh isosurface replace the previous rendering.

AC2: A user on a GL < 4.3 system sees Marching Cubes greyed out in the sub-panel with the
     tooltip "Requires OpenGL 4.3 compute shaders"; pressing M does not cycle to Marching Cubes.

AC3: A user who orbits the camera while in Marching Cubes mode sees no change in the mesh
     geometry (camera movement does not trigger recompute). Phong lighting changes correctly
     as the camera orbits because normals are stored in world space; the surface does not
     appear flat or incorrectly lit from any angle.

AC4: A user in Live mode who advances the simulation one timestep sees the mesh update.
     The same user in Freeze mode who advances one timestep sees no mesh update.

AC5: A user who adjusts Grid Resolution, Iso-value, or Particle Influence Radius sees the
     mesh update. On the next app launch, the same parameter values are restored.

AC6: A user on hardware with insufficient VRAM for 256^3 sees the 256^3 option greyed out
     (auto-downgraded to 128^3) with a visible UI notification.

AC7: A user who takes a screenshot or records frames in Marching Cubes mode gets correct MC
     output in the captured file. Pressing M during an active recording produces no mode switch
     and shows a non-recording notification that is not captured in the output.

AC8: The visual regression test using the 64-particle 4x4x4 grid fixture (authored in this
     story) passes on Mesa/Xvfb in CI, or issues GTEST_SKIP if GL 4.3 compute shaders are
     unavailable at runtime.

AC9: Unit tests for the RGB color blending math (weighted average formula and category color
     mapping including cat 500) pass independently of any OpenGL context or ImGui state.

AC10: No performance regression is observed in Spheres mode (manual verification; frame-time
      measurements documented in the PR description -- not an automated CI gate).

AC11: In a dense particle cloud, interior particles produce no visible geometry from any
      external camera angle. The Marching Cubes render displays only the outer isosurface
      shell. Camera orbit around a dense cluster (both while the simulation is running and
      while paused) reveals no interior particles breaking through the surface. Guaranteed
      by: (a) MC algorithm produces no triangles in interior voxels where all 8 density
      field corners exceed the isovalue; (b) GL_DEPTH_TEST occludes back geometry;
      (c) GL_CULL_FACE removes back-facing triangles. Validated by the 4x4x4 VR fixture
      (see AC8): the central density region produces no triangles; only the outer boundary
      transitions generate visible mesh.

### Out of Scope

- Screen-Space Metaballs rendering (SSM entry remains greyed out; no behavior change)
- CPU-side marching cubes fallback
- LOD streaming or adaptive resolution
- Automated CI gate for frame-time performance regression
- Real-time isosurface re-extraction on camera movement

---

## Implementation Notes

These are technical findings from code inspection. They inform todo planning and do not appear
in the Feature Specification.

### Foundation Status (from #123 / PR #131)

- RenderMode enum: Present. Contains Spheres, ScreenSpaceMetaballs, MarchingCubes.
- Sub-panel infrastructure: Present. imgui_menu.cpp has Spheres, MarchingCubes (always-greyed
  placeholder), Back -- 3 items total. SSM was removed from the panel (#123 failure).
- PanelLayer enum: Present (Main, RenderMode). No new layer needed.
- MenuState struct: Present but missing MC fields (see gaps below).
- glReadPixels post-composite fix: Confirmed in place. No action needed.
- cycleRenderMode(): Currently maps all non-Spheres modes back to Spheres. Must be updated.
- M key guard during recording: Already present (viewer_app.cpp, and imgui_menu.cpp
  `item("Render Mode", !state.is_recording, ...)`). Must verify it covers MC cycling.

### Code Gaps

1. MenuState (imgui_menu.hpp): No fields for grid_resolution, iso_value, influence_radius,
   live_freeze_mode. All must be added with defaults:
   - grid_resolution = 128 (enum or int: 64 / 128 / 256)
   - iso_value = 0.5f
   - influence_radius = 2.0f
   - live_freeze_mode = Live (enum: Live / Freeze)

2. windowConfig.hpp saveWindowConfig/loadWindowConfig: No keys for MC params. Must add
   four new keys: mc_grid_resolution, mc_iso_value, mc_influence_radius, mc_live_freeze.
   WindowConfigTests.cpp must be updated to cover round-trip persistence for these keys.

3. imgui_menu.cpp sub-panel (lines 286-335): Must add MC-specific items (sliders and
   Live/Freeze toggle) that appear when the active render mode is MarchingCubes.
   Current assertion in RenderModeSubPanelTests.cpp (panel_item_count == 3) will fail
   and must be updated to reflect the new item count when MC is selected.

4. viewer_app.hpp cycleRenderMode(): Must skip greyed modes at runtime. cycleRenderMode()
   is currently a constexpr static function with no runtime context -- it needs access to
   the GL 4.3 capability flag to skip unavailable modes. Design decision: pass capability
   as parameter or make it a member function with context.

5. viewer_app.cpp drawScene(): No MC branch. Must add:
   - compute dispatch for density_field.comp
   - compute dispatch for marching_cubes.comp
   - mesh render pass (mesh.vert / mesh.frag with Phong lighting)
   Mesh cache invalidation: dirty flag set on timestep change, cleared after recompute.
   Camera movement must NOT set the dirty flag.

6. New shaders required (src/shaders/):
   - density_field.comp (accumulate particle influence into GL_TEXTURE_3D GL_R32F)
   - marching_cubes.comp (MC algorithm, outputs triangles into SSBO via atomic counter)
   - mesh.vert / mesh.frag (Phong lighting, per-vertex weighted color)
   MC 256-entry lookup table: uploaded once as UBO or GL_TEXTURE_1D.

7. SSBO overflow: Pre-allocate for maximum expected triangle count. On overflow, clamp
   (truncate mesh at max triangles) and log a warning. Do not crash. Implementer must
   calculate and document the pre-allocation size based on 256^3 grid worst-case surface.

8. Zero-particle case: Render nothing (blank framebuffer). No crash.

9. Mode switch mesh retention: Retain the cached mesh in GPU memory when switching away
   from Marching Cubes mode, provided VRAM budget allows. If VRAM is constrained, evict
   to system memory or discard. Implementer to determine threshold during implementation.

### VRAM Detection for 256^3 Guard

- Use an injectable IVramQuery interface for testability (user decision Q4: "interface it,
  detect it").
- Threshold = minimum VRAM required for 256^3 operation:
  - Density field (GL_R32F): 256 * 256 * 256 * 4 bytes = 64 MB
  - SSBO (mesh output): implementer to calculate worst-case pre-allocation and add to threshold
- IVramQuery::availableMB() -> int. Real implementation uses GL extensions where available
  (GL_NVX_gpu_memory_info on NVIDIA, GL_ATI_meminfo on AMD) with a conservative fallback of
  128 MB on unknown hardware (which causes 256^3 to be greyed out by default on Mesa CI
  unless the extension is available).
- Auto-downgrade logic: resolveGridResolution(requested, IVramQuery&) -> effective_resolution.
  Unit-testable with a MockVramQuery. No ImGui dependency.

### Cat 500 Color Formula

Source: sphereVertex.vs line 38:
```
fColor = vec3(gl_InstanceID % 40 / 40.0f, gl_InstanceID % 1600 / 1600.0f, gl_InstanceID % 64000 / 64000.0f);
```
In C++ unit test terms: `vec3(index % 40 / 40.0f, index % 1600 / 1600.0f, index % 64000 / 64000.0f)`
where index is the particle's 0-based position in the buffer.

### 64-Particle 4x4x4 Fixture

Not present in tests/visual-regression/. Must be authored in this story.
- 64 particles arranged in a 4x4x4 grid with fixed positions and fixed category assignments
- Camera position must be hardcoded (see docs/visual-regression/camera-positioning-lessons-learned.md)
- Baseline image must be committed alongside the test
- Test issues GTEST_SKIP if GLAD_GL_VERSION_4_3 is 0 at runtime

### Sub-Panel Item Count After MC Activation

Current: 3 items (Spheres, Marching Cubes greyed, Back)
After this story, in Marching Cubes mode the sub-panel must show MC-specific items.
Implementer to determine final item count and update RenderModeSubPanelTests.cpp assertions.
Mode-specific items (sliders, toggle, refresh button) appear only when MC is the active mode;
the sub-panel swaps content within the existing RenderMode layer.

### Tooltip Text

Existing tooltip on greyed MC entry: "Mode not supported"
Required: "Requires OpenGL 4.3 compute shaders"
Update: imgui_menu.cpp, the tooltip string at the MarchingCubes item.

---

## Effort Estimate

**Size:** L (Large)
**Reasoning:** GPU compute pipeline (new to codebase), 4 new shaders, IVramQuery interface,
MenuState/windowConfig extensions, cycleRenderMode refactor, sub-panel UI additions, and a
VR test fixture that does not yet exist -- spanning 20 implementation todos across 7 phases,
requiring multiple sessions.

---

## Smart Trust Gate

1. **Success looks like:** All 10 ACs pass, unit test suite green, VR test passes on Mesa/Xvfb
   CI or issues GTEST_SKIP (documented behavior, not a failure).

2. **Not addressing:** SSM rendering (greyed placeholder, no change), CPU-side MC fallback,
   LOD streaming, automated frame-time CI gate, shader compilation failure graceful recovery
   (no AC covers this), mode-switch mesh eviction threshold (implementer decides).

3. **Top 3 failure modes:**
   - Compute shader dispatch incorrect -> density field garbage -> mesh garbage -> VR test fails.
     Signal: VR baseline diff. Response: add CPU readback test for density field values.
   - SSBO pre-allocation too small for even the 64-particle fixture -> truncated mesh.
     Signal: VR baseline diff with missing geometry. Response: increase pre-allocation size.
   - Mesa/CI GL 4.3 unavailable -> all compute tests GTEST_SKIP -> zero VR coverage.
     Signal: CI shows all MC tests as SKIP. Response: verify Mesa version; SKIP is correct
     behavior per AC8, not a build failure.

4. **Capability gaps:** Compute shaders are new to this codebase. SSBO dispatch pattern, atomic
   counter reset, and GL_TEXTURE_3D setup must be verified against GLAD headers during
   implementation. IVramQuery GL extension availability (GL_NVX_gpu_memory_info) must be
   confirmed -- likely not available on Mesa without explicit GLAD extension generation.

5. **Skeptic argument:** "cycleRenderMode() is currently constexpr; making it runtime-aware
   breaks its signature and all callers. The SSBO worst-case pre-allocation for 256^3 is
   likely 100-500 MB depending on surface complexity -- this may exceed Mesa/CI available
   VRAM. The 64-particle fixture VR baseline must be captured on the same Mesa version as CI
   or the pixel diff will fail."

---

## Implementation Plan

### Phase 1: Infrastructure (pure C++, no GPU dependency)

**Todo 1: MenuState MC fields**
Files: src/ui/imgui_menu.hpp, tests/core/MenuStateTests.cpp
- Add GridResolution enum (Grid64=64, Grid128=128, Grid256=256) to imgui_menu.hpp
- Add LiveFreezeMode enum (Live, Freeze) to imgui_menu.hpp
- Add to MenuState: grid_resolution=Grid128, iso_value=0.5f, influence_radius=2.0f,
  live_freeze=Live, mc_refresh_requested=false, mc_vram_downgrade_notification=false
- RED: test MenuState() defaults match spec; GREEN: add fields; COMMIT

**Todo 2: windowConfig MC persistence**
Files: src/windowConfig.hpp, tests/core/WindowConfigTests.cpp
- Add keys: mc_grid_resolution (int), mc_iso_value (float), mc_influence_radius (float),
  mc_live_freeze (int: 0=Live, 1=Freeze)
- Load: read keys with defaults (128, 0.5f, 2.0f, 0) when absent
- RED: test round-trip save/load of all 4 keys + default fallback; GREEN: add keys; COMMIT

**Todo 3: IVramQuery interface + resolveGridResolution (ALL VRAM tests here)**
Files: src/IVramQuery.hpp (new), tests/core/VramQueryTests.cpp (new)
- IVramQuery { virtual ~IVramQuery() = default; virtual int availableMB() const = 0; }
- MockVramQuery : IVramQuery { explicit MockVramQuery(int mb); }
- resolveGridResolution(int requested_res, const IVramQuery& vram, bool& was_downgraded) -> int
  - VRAM threshold for 256^3 = 280MB (64MB density field + 216MB SSBO for 2M triangles)
  - If available < 280MB, return 128, was_downgraded=true
  - 128^3 and 64^3 always succeed (no downgrade, was_downgraded=false)
- Concrete IVramQuery implementation (GladVramQuery):
  - Query GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX (NVIDIA) or
    GL_TEXTURE_FREE_MEMORY_ATI (AMD) if available
  - Return 128MB as conservative fallback on unknown hardware (causes 256^3 to grey out
    by default on Mesa CI unless extension is present; behavior is correct per AC6)
- RED: Write ALL of the following tests:
    resolveGridResolution(256, MockVramQuery(200)) -> 128, was_downgraded=true
    resolveGridResolution(256, MockVramQuery(512)) -> 256, was_downgraded=false
    resolveGridResolution(256, MockVramQuery(280)) -> 256, was_downgraded=false (exactly at threshold)
    resolveGridResolution(128, MockVramQuery(50)) -> 128, was_downgraded=false
    resolveGridResolution(64, MockVramQuery(10)) -> 64, was_downgraded=false
  GREEN: implement; COMMIT
Note: Todo 18 is removed -- all VRAM tests are here in Todo 3.

**Todo 4: cycleRenderMode capability-aware refactor**
Files: src/viewer_app.hpp, tests/core/RenderModeTests.cpp
- cycleRenderMode(RenderMode current, bool compute_available) -> RenderMode
  - compute_available=true: Spheres->MarchingCubes->Spheres
  - compute_available=false: Spheres->Spheres (skip MC)
- Remove constexpr; update all callers in viewer_app.cpp (lines 846 and 951) to pass
  compute_shaders_available_
- MUST update RenderModeTests.cpp lines 33-50 in the SAME commit:
  - Remove stale comment at line 33 ("cycleRenderMode always returns Spheres")
  - Replace three existing tests (all expect Spheres) with:
    CycleRenderMode_FromSpheres_ComputeAvailable_ReturnsMC
    CycleRenderMode_FromMC_ComputeAvailable_ReturnsSpheres
    CycleRenderMode_FromSpheres_ComputeUnavailable_ReturnsSpheres
  Old expectations are wrong after this change -- do not leave them as failing tests
- RED: write new tests and confirm they fail; GREEN: refactor cycleRenderMode, update callers
  and tests together; run full suite; COMMIT

### Phase 2: MC Lookup Table

**Todo 5: MC edge/triangle lookup tables**
Files: src/marching_cubes_tables.hpp (new)
- Standard 256-entry edgeTable[] and triTable[][16] (Paul Bourke / Lorensen-Cline tables)
- Namespace mc_tables { constexpr int edge_table[256]; constexpr int tri_table[256][16]; }
- No dedicated test (correctness verified by VR test); COMMIT

### Phase 2b: Color Blending (pure C++, no GL, directly unit-testable)

**Todo 5b: blendColors() free function in marching_cubes_color.hpp**
Files: src/marching_cubes_color.hpp (new), tests/core/MCColorBlendingTests.cpp (new)
- blendColors(const std::vector<glm::vec4>& particles, glm::vec3 voxel_center,
              float influence_radius) -> glm::vec3
  - For each particle: weight = gaussian_falloff(distance(particle.xyz, voxel_center), influence_radius)
  - category color: switch on int(particle.w):
      0 -> vec3(1,0,0); 1 -> vec3(0.2,0.6,1); 2 -> vec3(1,0,1); 3 -> vec3(0.89,0.59,0)
      500 -> vec3(idx%40/40.0f, idx%1600/1600.0f, idx%64000/64000.0f) where idx is buffer index
      default -> vec3(0.5,0.5,0.5)
  - result = sum(weight_i * color_i) / sum(weight_i); if sum(weights)==0, return vec3(0,0,0)
- Free function (not a class method): callable without GL context, no mocking needed
- RED: write tests (see Todo 17 for test list); GREEN: implement; COMMIT
- Note: Todo 17 is the TDD execution of this todo (tests written here first, used again in 17)

### Phase 3: Shaders

**Todo 6: density_field.comp**
Files: src/shaders/density_field.comp (new)
- layout(local_size_x=8, local_size_y=8, local_size_z=8): for each voxel, sum gaussian
  falloff from all particles within influence_radius world units
- Inputs: SSBO of particle positions (vec4 xyz+category), uniforms: grid_size, grid_origin,
  voxel_size, influence_radius, particle_count
- Output: image3D (GL_R32F) density field
- COMMIT

**Todo 7: marching_cubes.comp**
Files: src/shaders/marching_cubes.comp (new)
- Reads density field texture, iso_value; looks up MC tables from UBO
- Outputs triangles into SSBO (vec3 pos + vec3 normal + vec3 color per vertex)
- Atomic counter tracks vertex count; compute shader respects SSBO size limit (clamp, no crash)
- Per-vertex normals: computed as the gradient of the density field at each vertex position
  (central finite differences across adjacent voxels), stored in WORLD SPACE. Do NOT
  transform normals into view space -- they must remain world-space so that Phong lighting
  is view-angle-invariant as the camera orbits (AC3 + AC11).
- Per-vertex color: weighted average of contributing particle category colors at each vertex
  position; category 500 uses formula: vec3(index%40/40.0, index%1600/1600.0, index%64000/64000.0)
- COMMIT

**Todo 8: mesh.vert + mesh.frag**
Files: src/shaders/mesh.vert (new), src/shaders/mesh.frag (new)
- mesh.vert: reads position + normal + color from SSBO (gl_VertexID indexing).
  - Position: transformed by projection * view * model (standard MVP)
  - Normal: passed through AS-IS (world-space, no view transform). If model matrix is
    identity (no scaling), normals require no transformation. Do NOT multiply normals by
    the view matrix -- that would break lighting when the camera rotates (AC3).
  - Outputs: fColor (vec3), fWorldNormal (vec3, world-space), fLightDir (vec3)
- mesh.frag: Phong diffuse using WORLD-SPACE quantities:
    uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);  // same as sphere shaders
    float diffuse = max(0.0, dot(normalize(fWorldNormal), normalize(lightDirection)));
    color = vec4(fColor * diffuse, 1.0);
  Lighting is view-angle-invariant: the light stays in world space as the camera orbits.
- COMMIT

### Phase 4: GL Resource Management

**Todo 9: MCRenderer class shell (resource lifecycle only)**
Files: src/MCRenderer.hpp (new), src/MCRenderer.cpp (new)
- MCRenderer(int grid_res): allocates GL_TEXTURE_3D density field (GL_R32F),
  SSBO for triangle output (max_triangles=2,000,000 * 3 * sizeof(MCVertex)),
  atomic counter buffer, MC lookup table UBO (uploads edge_table + tri_table once)
  - SSBO cap: 2M triangles * 3 vertices * (12+12+12 bytes) = 216MB. This covers
    the 64-particle test fixture with large margin; 256^3 complex scenes may truncate
    (clamp + log warning per Implementation Notes item 7).
- ~MCRenderer(): deletes all GL objects
- resize(int new_grid_res): reallocates density texture + SSBO if resolution changes
- markDirty(): sets dirty_flag_ = true; called on timestep change only
- Zero-particle safety: MCRenderer::render() checks particles.empty() before any dispatch;
  if empty, clears the atomic counter to 0 and skips all compute calls (blank output, no crash)
- No render call yet; COMMIT

**Todo 10: MCRenderer::render() - compute dispatch + mesh draw**
Files: src/MCRenderer.hpp, src/MCRenderer.cpp
- render(particles, params, density_shader, mc_shader, mesh_shader, IVramQuery& vram):
  1. If !dirty_flag_: return (cache hit, no recompute)
  2. Bind density texture, dispatch density_field.comp (groups = ceil(grid_res/8) per axis)
  3. glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)
  4. Reset atomic counter to 0
  5. Dispatch marching_cubes.comp (groups = ceil(grid_res/8) per axis)
  6. glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT)
  7. Read vertex count from atomic counter
  8. Draw mesh:
       glEnable(GL_DEPTH_TEST);          // occludes back geometry from any camera angle
       glEnable(GL_CULL_FACE);           // removes back-facing triangles (inner surface)
       glCullFace(GL_BACK);
       // bind SSBO as vertex data, draw vertex_count vertices as GL_TRIANGLES
       glDisable(GL_CULL_FACE);          // restore for other passes that may need it
  9. dirty_flag_ = false
- COMMIT

### Phase 5: Integration

**Todo 11: Wire MCRenderer into ViewerApp + render dispatch**
Files: src/viewer_app.hpp, src/viewer_app.cpp
- ViewerApp members: unique_ptr<MCRenderer> mc_renderer_, bool compute_shaders_available_
  (plus Shader members: density_shader_, mc_shader_, mesh_shader_)
- On startup: compute_shaders_available_ = (GLAD_GL_VERSION_4_3 != 0); if true, compile
  the 4 MC shaders and construct mc_renderer_(128)
- Render dispatch lives at the SAME level as cam_->RenderSphere() (viewer_app.cpp line 342),
  NOT inside drawScene(). The pattern:
    if (render_mode_ == RenderMode::Spheres) { cam_->RenderSphere(); }
    else if (render_mode_ == RenderMode::MarchingCubes && mc_renderer_) {
        mc_renderer_->render(part_->translations, mc_params_, ...);
    }
  cam_->RenderSphere() is suppressed in MC mode (no sphere overdraw).
- On timestep load: call mc_renderer_->markDirty() ONLY when live_freeze_ == Live
- Zero-particle guard: if !part_ || part_->n == 0, skip markDirty() and render call;
  mc_renderer_->render() also guards internally (see Todo 9)
- Pass compute_shaders_available_ to cycleRenderMode() in M key handler (line 846) and
  Y gamepad handler (line 951)
- COMMIT

**Todo 12: Sub-panel MC capability gate + tooltip**
Files: src/ui/imgui_menu.cpp
- Pass compute_shaders_available (bool) into renderControllerPanel() or MenuState
- MC sub-panel item: enabled = compute_shaders_available; tooltip when disabled =
  "Requires OpenGL 4.3 compute shaders"
- Update RenderModeSubPanelTests.cpp: verify tooltip text and disabled state behavior
- COMMIT

### Phase 6: Sub-panel UI

**Todo 13: MC parameter sliders**
Files: src/ui/imgui_menu.cpp
- When active_render_mode == MarchingCubes AND panel_layer == RenderMode: render below mode
  selector: Grid Resolution radio/combo (64/128/256), Iso-value slider (0.0-2.0, step 0.01),
  Particle Influence Radius slider (0.1-10.0, step 0.1)
- Changes write to MenuState fields and set panel_action.mc_params_changed = true
- Callers in ViewerApp respond to mc_params_changed: call mc_renderer_->markDirty() +
  mc_renderer_->resize() if grid resolution changed
- Define named constants for panel item counts in imgui_menu.hpp (or a new header):
    enum class SubPanelItemCount : int {
        kBaseRenderMode = 3,          // Spheres + MC (greyed) + Back
        kMarchingCubesMode = 8        // 3 base + GridRes + IsoValue + Radius + LiveFreeze + Back
                                      // (exact count determined during implementation;
                                      //  conditional Refresh button in Freeze mode may be +1)
    };
- Replace the hardcoded `3` literal in RenderModeSubPanelTests.cpp line 69 with
  `static_cast<int>(SubPanelItemCount::kBaseRenderMode)` (Spheres-mode tests stay correct)
- Add new test: when MC is active mode, panel_item_count ==
  `static_cast<int>(SubPanelItemCount::kMarchingCubesMode)`
- COMMIT

**Todo 14: Live/Freeze toggle + Refresh Mesh button**
Files: src/ui/imgui_menu.cpp
- In MC sub-panel: Live/Freeze toggle button (state from MenuState.live_freeze)
- Refresh Mesh button: visible when live_freeze == Freeze; emits mc_refresh_requested=true
- ViewerApp: on mc_refresh_requested, call mc_renderer_->markDirty() regardless of live_freeze
- ViewerApp: on timestep load, call mc_renderer_->markDirty() only when live_freeze == Live
- COMMIT

**Todo 15: VRAM downgrade notification**
Files: src/viewer_app.cpp
- After resolveGridResolution() call (on startup and on grid resolution change):
  if was_downgraded, set MenuState.mc_vram_downgrade_notification = true
- In imgui_menu.cpp (or viewer_app.cpp): render an ImGui::Text overlay "256^3 not available:
  insufficient VRAM -- using 128^3" when mc_vram_downgrade_notification is true
- Notification remains until dismissed or next app launch
- COMMIT

**Todo 16: M-key-during-recording notification**
Files: src/viewer_app.cpp
- Existing guard (line 845): !recording_.is_active already blocks M key
- Add: when M is pressed and recording_.is_active, set a non-recording overlay flag
- Render overlay in imgui_menu.cpp: "M key disabled during recording" -- NOT captured by
  glReadPixels (which reads the FBO, not the ImGui overlay)
- COMMIT

### Phase 7: Tests

**Todo 17: RGB color blending unit tests**
Files: tests/core/MCColorBlendingTests.cpp (new)
- blendColors() lives in src/marching_cubes_color.hpp (see Todo 5b)
- Tests (RED phase was in Todo 5b; this todo is GREEN + COMMIT for the test file):
  - Two particles equidistant: result = average of their colors
  - One particle dominant: result closer to that particle's color
  - Category 500 at index 0: returns vec3(0.0f, 0.0f, 0.0f)
  - Category 500 at index 40: returns vec3(1.0f, 0.025f, 0.000625f) (40%40/40=1.0,
    40%1600/1600=0.025, 40%64000/64000=0.000625)
  - Empty contributing set: returns vec3(0.0f, 0.0f, 0.0f) (no div-by-zero)
  - Zero-weight sum (all particles outside influence radius): returns vec3(0.0f,0.0f,0.0f)
- COMMIT

Todo 18: REMOVED. All VRAM tests are in Todo 3 (see note there).

**Todo 19: 64-particle 4x4x4 fixture**
Files: tests/visual-regression/fixtures/Grid4x4x4Fixture.hpp (new)
- 64 particles at positions (i,j,k) * voxel_spacing for i,j,k in {-1.5, -0.5, 0.5, 1.5}
- All category 0 (red) for deterministic color output
- Particle scale matching kSimToDisplayScale (0.25) so positions map into typical view frustum
- Camera: read docs/visual-regression/camera-positioning-lessons-learned.md before choosing;
  commit camera position alongside fixture
- COMMIT (fixture + camera; no baseline yet)

**Todo 20: Visual regression test + baseline capture**
Files: tests/visual-regression/RenderingRegressionTests.cpp
- Add test MarchingCubesRenders_64ParticleGrid_MatchesBaseline:
  - GTEST_SKIP (first): if GLAD_GL_VERSION_4_3 == 0 (no compute shader support)
  - GTEST_SKIP (second): if baseline file is absent -- save current render to baseline
    path and skip (first-run baseline generation, matches existing VR test pattern at
    RenderingRegressionTests.cpp lines 287-296)
  - If both conditions pass: render one frame; compare to baseline via PixelComparator::compare()
  - Baseline path: tests/visual-regression/baselines/mc_64particle_grid.png
- After first run on a GL 4.3 machine: commit the generated baseline image
- CI: subsequent runs compare against committed baseline within MAX_DIFF_RATIO
- COMMIT

Pre-PR Checklist (AC10):
- Before opening the PR: run the app in Spheres mode with a representative dataset
  (e.g., the default simulation) and record frame-time (e.g., ImGui fps overlay or
  external tool). Document the measurement in the PR description. No automated gate --
  this is a manual verification step required by AC10.

---

## Phase 8: Spatial Grid Density Acceleration

Replaces the O(N) per-voxel particle loop in `density_field.comp` with a uniform spatial grid.
Each voxel checks only particles in its 3x3x3 neighboring cells instead of all N.

Expected speedup: 10-20x at 64^3 with 65536 particles (density pass: ~1s -> ~50ms).
Thermal benefit: reduces sustained 100% GPU utilization during playback.

### Skeptic Findings (addressed below in todo design)

1. Rebuild trigger incomplete: The grid cell size equals influence_radius. If the user
   changes influence_radius via slider (without loading new particles), the existing grid
   is stale. Rebuild must fire on influence_radius change, not only on particle upload.

2. "Numerically identical" is not achievable: Floating point addition is not associative.
   The spatial grid processes particles in cell-sorted order; the original shader uses
   particle-buffer order. Different summation order -> different mantissa rounding.
   Acceptance criterion: within 2/255 pixel tolerance on the VR test baseline (same as
   existing VR tests), NOT bit-for-bit identical.

3. Architecture coupling: If ViewerApp binds SSBOs to GL slots 2/3 before calling
   MCRenderer::render(), any future code added inside MCRenderer before the density
   dispatch that also uses slots 2/3 silently breaks the spatial grid. Fix: MCRenderer
   accepts the SSBO handles as parameters to render() and binds them itself, right
   before the density dispatch.

4. Pathological cell count: influence_radius=0.01 over a 100-unit extent -> 10000^3 cells.
   Cap num_cells per axis at grid_size (max 256) to prevent memory blowup.

### Todo 21: SpatialGrid unit tests (RED)

Files: tests/core/SpatialGridTests.cpp (new)

Tests:
- Single particle at (0,0,0) with ir=1.0: maps to cell (0,0,0)
- Particle at exact cell boundary: maps to expected cell (not off-by-one)
- cell_starts[c] to cell_starts[c+1] contains exactly the particles whose positions
  map to cell c (iterate and verify for a 3-particle case spanning 2 cells)
- Empty particle list: cell_starts all zero, sorted_particles empty, no crash
- All particles in same cell: cell_starts[cell+1] - cell_starts[cell] == particle_count
- When influence_radius is halved and build() is called again: cell assignments update
  correctly (verifies rebuild-on-ir-change behavior)
- num_cells per axis capped at max_cells_per_axis (4 particles with ir=0.001 over a
  100-unit extent, max=64: num_cells_x <= 64)

RED: write all tests; verify they fail (SpatialGrid.hpp does not exist); COMMIT test file

### Todo 22: SpatialGrid class (GREEN)

Files: src/SpatialGrid.hpp (new)

```
struct SpatialGrid {
    void build(const glm::vec4* particles, int count,
               float influence_radius, glm::vec3 origin, glm::vec3 extent,
               int max_cells_per_axis);

    std::vector<glm::vec4> sorted_particles; // particles reordered by cell
    std::vector<uint32_t>  cell_starts;      // [c]..[c+1] = particle range in cell c
    int   num_cells_x = 0, num_cells_y = 0, num_cells_z = 0;
    float cell_size   = 0.0f;
    glm::vec3 cell_origin{};
};
```

- build() steps: compute cell for each particle, counting-sort by cell ID, fill
  cell_starts via prefix sum, copy particles in sorted order to sorted_particles
- Clamp num_cells per axis to max_cells_per_axis before allocating

GREEN: implement; run SpatialGridTests -- all pass; COMMIT

### Todo 23: Update density_field.comp to use spatial grid

Files: src/shaders/density_field.comp (modify)

Add SSBOs and uniforms:
```glsl
layout(std430, binding = 2) readonly buffer CellStarts    { uint cell_starts[];     };
layout(std430, binding = 3) readonly buffer SortedParticles { vec4 sorted_particles[]; };
uniform float cell_size;
uniform vec3  cell_origin;
uniform int   num_cells_x, num_cells_y, num_cells_z;
```

Replace the O(N) for loop with a 3x3x3 neighbor cell loop:
```glsl
ivec3 vcell = clamp(ivec3(floor((voxel_center - cell_origin) / cell_size)),
                    ivec3(0), ivec3(num_cells_x-1, num_cells_y-1, num_cells_z-1));
float density = 0.0;
for (int dx = -1; dx <= 1; ++dx) {
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dz = -1; dz <= 1; ++dz) {
      ivec3 nc = vcell + ivec3(dx, dy, dz);
      if (any(lessThan(nc, ivec3(0)))) continue;
      if (any(greaterThanEqual(nc, ivec3(num_cells_x, num_cells_y, num_cells_z)))) continue;
      int cid = nc.x + nc.y * num_cells_x + nc.z * num_cells_x * num_cells_y;
      for (uint i = cell_starts[cid]; i < cell_starts[cid + 1]; ++i) {
          vec3  delta   = sorted_particles[i].xyz - voxel_center;
          float dist_sq = dot(delta, delta);
          if (dist_sq > ir_sq) continue;
          float w = 1.0 - dist_sq * inv_ir_sq;
          density += w * w * w;
      }
    }
  }
}
```

IMPORTANT: after editing src/shaders/density_field.comp, manually copy to
build/Viewer-Assets/shaders/density_field.comp -- CMake only copies at configure time.

COMMIT

### Todo 24: ViewerApp spatial grid SSBO management

Files: src/viewer_app.hpp (members), src/viewer_app.cpp (build + upload + pass to render)

Add to ViewerApp:
```
SpatialGrid  spatial_grid_;
GLuint       cell_starts_ssbo_     = 0;
GLuint       sorted_particles_ssbo_ = 0;
float        last_spatial_grid_ir_ = -1.0f;
```

Add rebuildSpatialGrid(influence_radius):
1. Compute particle bounding box (or use grid_origin + grid_extent from MCRenderer)
2. Call spatial_grid_.build(particles, count, ir, origin, extent, grid_size)
3. Upload spatial_grid_.cell_starts to cell_starts_ssbo_ (glBufferData)
4. Upload spatial_grid_.sorted_particles to sorted_particles_ssbo_ (glBufferData)
5. Set last_spatial_grid_ir_ = influence_radius

Call rebuildSpatialGrid() at:
- (a) Particle load/frame advance (existing markDirty location in viewer_app.cpp)
- (b) influence_radius change: in the mc_params_changed handler, check if
  influence_radius != last_spatial_grid_ir_; if so, call rebuildSpatialGrid()

Pass cell_starts_ssbo_ and sorted_particles_ssbo_ to MCRenderer::render() as parameters.
MCRenderer binds them to slots 2 and 3 immediately before glDispatchCompute for density.
Do NOT bind them in ViewerApp -- MCRenderer owns the bind timing (Skeptic gap 3 fix).

Also set corresponding uniforms in the density shader:
cell_size, cell_origin, num_cells_x, num_cells_y, num_cells_z

COMMIT

### Todo 25: Integration verification via existing VR test

Files: none (run existing test)

Run: cd build && ./tests/ParticleViewerTests --gtest_filter="MarchingCubesVRTest*"

The existing MarchingCubesRenders_64ParticleGrid_MatchesBaseline test uses the 4x4x4
64-particle fixture. If it passes within 2/255 pixel tolerance against the committed
baseline, the spatial grid produces visually correct density output.

Acceptance: test PASSES (exit 0).

If the test fails: density is wrong. Diagnose before declaring the feature done.
Do NOT update the baseline -- a baseline update means the rendering changed.

COMMIT: only after the test passes.
