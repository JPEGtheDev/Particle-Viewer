# User Stories: GUI and Input Enhancement Features

**Created:** 2026-02-11  
**Updated:** 2026-02-11  
**Project:** Particle-Viewer  
**Status:** Refined with User Feedback

---

## Table of Contents

1. [Story 1: Gamepad/Xbox Controller Support](#story-1-gamepadxbox-controller-support)
2. [Story 2: Resolution-Independent Particle Scaling](#story-2-resolution-independent-particle-scaling)
3. [Story 3: Runtime Resolution Changing](#story-3-runtime-resolution-changing)
4. [Story 4: ImGui Menu System](#story-4-imgui-menu-system)
5. [Future Vision: Movie Editor with Keyframe Animation](#future-vision-movie-editor-with-keyframe-animation)
6. [Implementation Order Recommendation](#implementation-order-recommendation)

---

## Story 1: Gamepad/Xbox Controller Support

**Title:** Add Gamepad/Xbox Controller Support with Tank Control Mapping

**Type:** Feature  
**Size:** L (recommend breaking into subtasks - see breakdown below)  
**Epic/Component:** Input System  
**Priority:** Medium  
**Depends On:** None

---

### Effort Estimate

**Total Premium Requests:** 80-120  
**Recommended Model Tier:** Advanced  
**Reasoning:** Complex integration across input handling, state management, and documentation. Requires careful controller API abstraction, input mapping configuration, and comprehensive documentation with pictured diagrams.

---

### User Story

**As a** Particle-Viewer user  
**I want to** control the viewer using an Xbox/gamepad controller  
**So that** I can navigate simulations more intuitively and comfortably without keyboard/mouse

---

### Acceptance Criteria

#### Controller Input Mapping
- [ ] Left stick controls camera movement (same constraints as tank controls)
- [ ] Right stick controls camera rotation/viewing angle
- [ ] Left trigger (LT) rewinds simulation frames
- [ ] Right trigger (RT) fast-forwards simulation frames
- [ ] Left bumper (LB) steps backward one frame
- [ ] Right bumper (RB) steps forward one frame
- [ ] A button toggles play/pause
- [ ] Select/Back button opens file load dialog
- [ ] Y button toggles COM (Center of Mass) lock
- [ ] X button toggles point lock
- [ ] L3 (left stick button) moves point lock target inward (bracket functionality)
- [ ] R3 (right stick button) moves point lock target outward (bracket functionality)
- [ ] While locked, right stick up/down controls zoom in/out

#### Technical Requirements
- [ ] Supports standard Xbox One/360 controllers on target platforms
- [ ] Analog stick input respects dead zones (configurable, default ~0.15)
- [ ] Trigger input properly mapped to analog range [0.0, 1.0]
- [ ] Controller hotplug support (connect/disconnect during runtime)
- [ ] Falls back gracefully to keyboard/mouse if no controller detected
- [ ] Input state does not conflict with existing keyboard/mouse controls (both work simultaneously)

#### Documentation
- [ ] Pictured documentation showing controller layout with labeled buttons
- [ ] Markdown document in `docs/CONTROLLER_SUPPORT.md` with:
  - Controller button mapping diagram
  - Setup instructions per platform
  - Troubleshooting section for common issues
- [ ] README.md updated with controller support mention

#### Testing
- [ ] Unit tests for controller input mapping logic
- [ ] Integration tests verify controller state updates ViewerApp correctly
- [ ] Manual testing checklist document for all button combinations

---

### Technical Notes

**Dependencies:**
- **Library Options for Gamepad Input:**
  1. **GLFW 3.3+ (Recommended)** - Already used by project, has built-in gamepad support via `glfwGetGamepadState()`
     - ✅ Zero new dependencies
     - ✅ Cross-platform (Linux, Windows, macOS)
     - ✅ Standard xinput/gamepad mapping (supports all standard controllers)
     - ✅ Target Xbox 360 layout for documentation
     - ❌ No haptic feedback support (native rumble not available)
  
  2. **SDL2 GameController API** - Robust, widely-used gamepad library
     - ✅ Extensive controller database (supports many controller types)
     - ✅ Haptic feedback support
     - ✅ Better controller detection/hotplug
     - ❌ Adds new external dependency
     - ❌ Requires SDL2 installation on system
  
  3. **SFML Joystick Module** - C++ friendly, multi-platform
     - ✅ Clean C++ API
     - ✅ Cross-platform
     - ❌ Adds new external dependency
     - ❌ Less gamepad-specific (more generic joystick API)

**Recommendation:** Use **GLFW 3.3+ gamepad support** with standard xinput mapping. Documentation will feature Xbox 360 controller layout, but all standard controllers supported. Haptic feedback omitted as GLFW doesn't support it natively.

**Platform Priority:**
- **Primary:** Linux (Flatpak distribution target)
- **Secondary:** Windows (including WSL2 compatibility)
- GLFW gamepad support works on both platforms

**Button Mapping Strategy:**
- Hardcoded mapping as specified (not configurable initially)
- Future enhancement: Configuration file with keyboard mappings

**Constraints:**
- Must respect same movement constraints as tank controls (likely velocity/acceleration limits)
- Analog input needs smoothing/filtering to prevent jittery camera movement
- Dead zone configuration may need per-controller calibration

**Files to Create/Modify:**
- `src/input/gamepad_handler.hpp` (new) - Gamepad input abstraction
- `src/input/gamepad_handler.cpp` (new) - GLFW gamepad state polling
- `src/viewer_app.hpp` (modify) - Add gamepad input handling callbacks
- `src/viewer_app.cpp` (modify) - Wire gamepad input to application state
- `docs/CONTROLLER_SUPPORT.md` (new) - Pictured documentation
- `docs/controller-layout.png` (new) - Controller diagram image
- `tests/input/GamepadHandlerTests.cpp` (new) - Unit tests
- `README.md` (modify) - Add controller support section

**Architecture Notes:**
- Create `GamepadHandler` class to encapsulate controller polling/state
- Use strategy pattern for input mapping (allows future custom mappings)
- Controller state should update during input polling phase of main loop
- Consider input configuration file (`gamepad_config.ini`) for dead zones, sensitivity

---

### Definition of Done

- [ ] Code written and peer-reviewed
- [ ] Unit tests pass (min. 85% coverage on new code)
- [ ] All controller buttons mapped and functional
- [ ] Pictured documentation complete with controller diagram
- [ ] No new linter/compiler warnings
- [ ] Manual testing completed on at least one Xbox controller
- [ ] Integration tested on primary platform (Linux recommended)
- [ ] README and docs updated
- [ ] Ready to merge to main branch

---

### Subtask Breakdown (Recommended)

Given the size (L), this story should be broken into smaller independent stories:

#### Subtask 1.1: GLFW Gamepad Input Integration (M, 35-45 requests)
- Create `GamepadHandler` class
- Poll GLFW gamepad state in main loop
- Unit tests for gamepad detection and state reading
- **Deliverable:** Gamepad connection/disconnection detected, state accessible

#### Subtask 1.2: Camera Control Mapping (M, 30-40 requests)
- Map left stick → camera movement
- Map right stick → camera rotation
- Implement dead zone filtering
- **Deliverable:** Basic camera control with gamepad sticks

#### Subtask 1.3: Playback Control Mapping (S, 20-25 requests)
- Map triggers → rewind/fast-forward
- Map bumpers → frame step
- Map A button → play/pause
- **Deliverable:** Playback controls functional via gamepad

#### Subtask 1.4: Advanced Feature Mapping (M, 25-30 requests)
- Map Y/X buttons → COM/point lock toggles
- Map L3/R3 → bracket functionality (point lock adjust)
- Map right stick up/down → zoom while locked
- **Deliverable:** All advanced features controllable via gamepad

#### Subtask 1.5: Documentation and Testing (S, 20-25 requests)
- Create pictured controller layout diagram
- Write `docs/CONTROLLER_SUPPORT.md`
- Manual testing checklist
- Update README
- **Deliverable:** Complete documentation ready for users

---

### Comments

**User Requirements Confirmed:**
- **Controller Support:** All standard controllers (PlayStation, Xbox, etc.), using standard xinput mapping. Documentation will feature Xbox 360 controller layout.
- **Platform Priority:** Linux first (Flatpak target), Windows/WSL2 as secondary. GLFW's cross-platform support handles both.
- **Button Mapping:** Hardcoded initially. Future story will estimate effort for configuration file including keyboard mappings.
- **Haptic Feedback:** Not included - GLFW doesn't support native rumble.
- **Multi-Controller:** Not needed, removed from scope.

**Future Work:**
- Custom button remapping via config file (separate story - estimate effort for both gamepad and keyboard mapping configuration)
- Advanced controller features (if migrating to SDL2 later)

---

## Story 2: Resolution-Independent Particle Scaling

**Title:** Implement Resolution-Independent Particle Scaling

**Type:** Feature  
**Size:** M  
**Epic/Component:** Rendering  
**Priority:** High  
**Depends On:** None

---

### Effort Estimate

**Total Premium Requests:** 45-60  
**Recommended Model Tier:** Standard  
**Reasoning:** Moderate rendering complexity requiring shader modifications, scaling calculations, and visual regression test updates. Well-understood problem with clear mathematical solution.

---

### User Story

**As a** Particle-Viewer user  
**I want** particles to maintain consistent visual size proportions across different resolutions  
**So that** I see the same relative composition when scaling my resolution, not just more particles fitting on screen

---

### Acceptance Criteria

#### Rendering Behavior
- [ ] Particle size maintains proportional screen coverage across resolutions
- [ ] At 1920x1080, particles occupy target percentage of viewport
- [ ] At 1280x720, same particles occupy same target percentage of viewport
- [ ] At 2560x1440, same particles occupy same target percentage of viewport
- [ ] Scaling works correctly with 16:9 aspect ratios (standard target)
- [ ] Visual appearance remains consistent when window is resized

#### Visual Regression Tests
- [ ] All 3 existing visual regression tests updated with resolution-scaling
- [ ] Baseline images regenerated for standard 16:9 resolutions
- [ ] Tests verify particle screen-space percentage is consistent ±2% across resolutions
- [ ] Tests pass on CI with Xvfb/Mesa (headless)

#### Technical Implementation
- [ ] Scaling computation moved to shader side (not CPU-side per-particle)
- [ ] Sphere scale values no longer hardcoded per resolution in CPU code
- [ ] Use uniform variable for viewport dimensions in shader
- [ ] Particle size calculation based on viewport height (resolution-independent metric)

---

### Technical Notes

**Current Problem:**
- Sphere scale values are currently hand-tuned per resolution (see `SphereParams` in `viewer_app.hpp`)
- When resolution increases, more particles fit on screen, but each particle appears smaller
- User expects: "zoomed in" view (same particles, higher detail), not "more particles visible"

**Solution Approach:**
- Define "reference particle size" as percentage of viewport height (e.g., 2% of screen height)
- Compute particle size in shader based on `uniform vec2 viewportSize`
- Formula: `particleRadius = (viewportSize.y * targetPercentage) / projectionScaleFactor`
- Camera FOV and distance remain unchanged; only particle rendering size adjusts

**Files to Create/Modify:**
- `src/shaders/particle.vert` (modify) - Add viewport uniform, compute resolution-independent size
- `src/shaders/particle.frag` (modify) - Update if fragment shader uses size-dependent logic
- `src/viewer_app.cpp` (modify) - Pass viewport dimensions as shader uniform
- `src/viewer_app.hpp` (modify) - Remove hardcoded resolution-specific scale values from `SphereParams`
- `tests/visual-regression/` (modify) - Update all 3 visual tests with multi-resolution checks
- `tests/visual-regression/baselines/` (regenerate) - New baseline images for 720p, 1080p, 1440p

**Architecture Notes:**
- This moves toward resolution-independence, a key architectural goal (see `docs/CODING_STANDARDS.md`)
- Reduces magic numbers in codebase
- Makes future resolution changes trivial (no re-tuning needed)

**Mathematical Approach:**
```cpp
// In vertex shader
uniform vec2 u_viewportSize;
uniform float u_targetParticleScreenPercent; // e.g., 0.02 for 2%

float particleScreenHeight = u_viewportSize.y * u_targetParticleScreenPercent;
float particleRadius = particleScreenHeight * projectionScale; // Adjusted for perspective
```

---

### Definition of Done

- [ ] Code written and peer-reviewed
- [ ] Shader modifications tested and functional
- [ ] Resolution changes produce consistent particle sizes (visual verification)
- [ ] All 3 visual regression tests pass with updated baselines
- [ ] Unit tests pass (min. 85% coverage on new code)
- [ ] No new linter/compiler warnings
- [ ] CI passes (Xvfb + Mesa)
- [ ] Documentation updated if shader uniforms changed
- [ ] Ready to merge to main branch

---

### Comments

**User Requirements Confirmed:**
- **Particle Size Complexity:** Number of particles on screen varies greatly based on camera position. Historical challenge: need to adjust BOTH spacing AND particle size to achieve correct visual appearance.
- **Particle Scale Parameter:** May want configurable particle scale, but this depends on simulation characteristics and shell depth (simulation-specific technical considerations).
- **Visual Regression Strategy:** Ideally, NO images need regeneration for existing 1280x720 baseline. The cube test image should appear identical across all resolutions, just at higher quality. Verification approach: scale down higher resolution render to compare with 720p baseline for visual consistency.
- **Key Insight:** Resolution independence means "same composition, higher detail" not "more particles visible."

**Implementation Note:**
The challenge is maintaining consistent visual appearance while allowing particle size to be resolution-independent. May need to expose particle scale as a configurable parameter separate from resolution calculations.

**Visual Regression Test Impact:**
- Baseline images for 1280x720 should NOT require regeneration (verify by downscaling higher res images)
- Add new baselines at: 1920x1080, 2560x1440
- Test verification: "downscaled high-res image matches 720p baseline within tolerance"

**Future Work:**
- User-configurable particle size via GUI (slider: "Particle Scale")
- Per-particle-type scaling (e.g., stars vs planets in N-body simulation)

---

## Story 3: Runtime Resolution Changing

**Title:** Support Runtime Resolution Changes

**Type:** Feature  
**Size:** M  
**Epic/Component:** Window Management  
**Priority:** Medium  
**Depends On:** Story 2 (Resolution-Independent Particle Scaling) - soft dependency

---

### Effort Estimate

**Total Premium Requests:** 35-50  
**Recommended Model Tier:** Standard  
**Reasoning:** Moderate complexity involving OpenGL framebuffer resizing, GLFW window reconfiguration, and state preservation. Well-defined problem with established patterns in OpenGL applications.

---

### User Story

**As a** Particle-Viewer user  
**I want to** change the window resolution while the application is running  
**So that** I can adjust the display size without restarting the application

---

### Acceptance Criteria

#### Functional Requirements
- [ ] User can change resolution via GUI menu (dropdown or text input)
- [ ] Supports standard 16:9 resolutions:
  - 1280x720 (720p)
  - 1920x1080 (1080p)
  - 2560x1440 (1440p)
  - 3840x2160 (4K) - if system supports
- [ ] Fullscreen/windowed mode toggle via Alt+Enter hotkey
- [ ] Resolution choice persists across application restarts (saved to config file)
- [ ] Window resizes smoothly without flickering
- [ ] Rendering adapts immediately to new resolution
- [ ] Application state preserved across resolution changes:
  - Camera position and orientation maintained
  - Playback state (frame number, play/pause) maintained
  - Loaded simulation file remains loaded
  - Settings and preferences retained

#### Technical Requirements
- [ ] OpenGL viewport resized via `glViewport()`
- [ ] Framebuffer objects (FBOs) resized or recreated as needed
- [ ] Projection matrix updated with new aspect ratio
- [ ] GLFW window resized via `glfwSetWindowSize()`
- [ ] No memory leaks when resizing (valgrind clean)
- [ ] Handles edge cases:
  - Minimum window size enforced (e.g., 640x360)
  - Resolution changes during playback don't cause frame drops
  - Resolution changes during frame rendering don't corrupt framebuffer

#### Testing
- [ ] Unit tests verify viewport and projection updates
- [ ] Integration tests confirm state preservation across resolution changes
- [ ] Manual testing on multiple resolutions (720p → 1080p → 1440p → back to 1080p)
- [ ] CI testing validates headless mode still works (Xvfb)

---

### Technical Notes

**Current Behavior:**
- Resolution is set at application startup via config file or defaults
- Changing resolution requires restarting application
- No runtime resize support

**Implementation Approach:**

1. **Window Resize Handler:**
   - Add GLFW framebuffer size callback: `glfwSetFramebufferSizeCallback()`
   - Callback updates `ViewerApp::WindowConfig` dimensions
   - Trigger viewport/projection recalculation

2. **Viewport Update:**
   ```cpp
   void ViewerApp::onFramebufferResize(int width, int height) {
       windowConfig.width = width;
       windowConfig.height = height;
       glViewport(0, 0, width, height);
       updateProjectionMatrix(); // Recalculate aspect ratio
   }
   ```

3. **Framebuffer Resizing (if using FBOs):**
   - Check if application uses offscreen FBOs for rendering
   - Recreate FBO textures/renderbuffers with new dimensions
   - Detach old attachments, create new, reattach

4. **State Preservation:**
   - Camera state (position, rotation) unaffected by resolution
   - Playback state (frame index, play/pause) unaffected
   - Only update viewport-dependent values (projection matrix, aspect ratio)

**Files to Create/Modify:**
- `src/viewer_app.hpp` (modify) - Add `onFramebufferResize()` method
- `src/viewer_app.cpp` (modify) - Implement resize logic, register GLFW callback
- `src/camera.hpp` (modify) - Add `updateProjectionMatrix(float aspect)` if needed
- `tests/integration/ResolutionChangeTests.cpp` (new) - Integration tests for resolution changes
- `docs/USER_GUIDE.md` (modify) - Document resolution change feature

**Dependencies:**
- Soft dependency on Story 2 (Resolution-Independent Particle Scaling)
- If Story 2 not implemented, particles may appear disproportionate after resolution change
- Can be implemented independently, but user experience better with Story 2 completed first

**Constraints:**
- Minimum resolution: 640x360 (enforced to prevent unusable window)
- Maximum resolution: System-dependent (don't exceed monitor native resolution)
- Aspect ratio: Target 16:9, but should work with other ratios

---

### Definition of Done

- [ ] Code written and peer-reviewed
- [ ] User can change resolution via GUI (or temporary hotkey for testing)
- [ ] Resolution changes take effect immediately
- [ ] All application state preserved across changes
- [ ] Unit tests pass (min. 85% coverage on new code)
- [ ] Integration tests verify state preservation
- [ ] No memory leaks (valgrind clean)
- [ ] No new linter/compiler warnings
- [ ] CI passes (Xvfb + Mesa)
- [ ] Documentation updated
- [ ] Ready to merge to main branch

---

### Comments

**User Requirements Confirmed:**
- **Interface:** Resolution changes via GUI menu only (no temporary hotkey needed). Story 4 (ImGui Menu) should be implemented first.
- **Command-Line:** Application already supports setting resolution via command-line argument at startup.
- **Fullscreen Toggle:** Yes, implement Alt+Enter for fullscreen/windowed mode toggle.
- **Persistence:** Yes, resolution choice should persist across application restarts (saved to config file).

**Integration with Story 4 (GUI Menu):**
- GUI menu implementation is prerequisite
- Resolution selector will be dropdown in View menu
- Standard 16:9 presets: 1280x720, 1920x1080, 2560x1440, 3840x2160 (4K if supported)

**Future Work:**
- Fullscreen/windowed mode toggle
- Custom resolution input (not just presets)
- Multi-monitor support (display selection)

---

## Story 4: ImGui Menu System

**Title:** Implement ImGui-Based Menu System

**Type:** Feature  
**Size:** L (recommend breaking into subtasks - see breakdown below)  
**Epic/Component:** UI/GUI  
**Priority:** High  
**Depends On:** None

---

### Effort Estimate

**Total Premium Requests:** 70-95  
**Recommended Model Tier:** Standard  
**Reasoning:** Standard integration work requiring ImGui setup, OpenGL integration, menu design, and testing. Well-documented library with clear integration patterns for OpenGL applications.

---

### User Story

**As a** Particle-Viewer user  
**I want to** access application features via a graphical menu system  
**So that** I can easily toggle settings, load files, and quit without memorizing keyboard shortcuts

---

### Acceptance Criteria

#### Menu Implementation
- [ ] Top menu bar with "File", "View", "Help" categories (or similar logical grouping)
- [ ] **File Menu:**
  - [ ] "Load File..." - Opens file dialog to load simulation data
  - [ ] "Quit" - Closes application gracefully
- [ ] **View Menu:**
  - [ ] "Toggle Debug Overlay" - Shows/hides camera debug info
  - [ ] "Resolution" submenu (if Story 3 implemented) - Select resolution
  - [ ] Future: COM Lock, Point Lock toggles
- [ ] **Help Menu:**
  - [ ] "About" - Shows application version and info
  - [ ] "Controls" - Shows keyboard/mouse/gamepad reference

#### Technical Requirements
- [ ] ImGui (Dear ImGui) integrated with existing OpenGL context
- [ ] Menu renders on top of 3D viewport (overlay mode)
- [ ] Menu visible by default on application start
- [ ] Menu automatically hidden in framebuffer screenshots (separate render paths)
- [ ] Screenshot configuration system to control visible UI elements:
  - [ ] Toggle menu visibility in screenshots
  - [ ] Toggle debug overlay visibility in screenshots
  - [ ] Toggle simulation time/metadata visibility in screenshots
  - [ ] Configuration accessible via "Screenshot Settings..." menu item
- [ ] Input handling respects menu state:
  - Mouse clicks on menu don't propagate to camera controls
  - Keyboard shortcuts disabled when menu focused (e.g., typing in text field)
- [ ] Menu can be toggled on/off with hotkey (e.g., F1 or Escape)
- [ ] Menu state saved across application runs (open/closed preference)
- [ ] Supports headless mode (menu code doesn't break CI tests)

#### Visual Design
- [ ] Menu uses consistent theme (default dark ImGui theme acceptable)
- [ ] Menu is semi-transparent when not hovered (doesn't obscure viewport)
- [ ] Font size readable at 1080p resolution
- [ ] Menu responds to mouse hover and click events
- [ ] Menu keyboard navigation works (Tab, Arrow keys, Enter)

#### Testing
- [ ] Unit tests for menu state management
- [ ] Integration tests verify menu actions trigger correct application behavior
- [ ] Manual testing checklist for all menu items
- [ ] CI tests pass with ImGui integrated (headless mode)

---

### Technical Notes

**Why ImGui (Dear ImGui)?**
- ✅ Lightweight immediate-mode GUI - perfect for tools/visualizers
- ✅ Excellent OpenGL integration (example code widely available)
- ✅ Active development and community support
- ✅ MIT licensed (compatible with project)
- ✅ Works well with existing GLFW + OpenGL stack
- ✅ Used by many scientific/visualization tools
- ✅ Easy to extend (add new menu items trivially)

**ImGui Integration Steps:**

1. **Add ImGui Dependency:**
   - Download ImGui from: https://github.com/ocornut/imgui
   - Add as git submodule or copy files to `src/imgui/`
   - Update CMakeLists.txt to compile ImGui sources

2. **Initialize ImGui Context:**
   ```cpp
   // In ViewerApp::initialize()
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 130"); // Adjust for your GL version
   ImGui::StyleColorsDark(); // Set theme
   ```

3. **Render ImGui UI:**
   ```cpp
   // In ViewerApp::render()
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
   
   renderMenuBar(); // Custom method to draw menu
   
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   ```

4. **Menu Rendering Example:**
   ```cpp
   void ViewerApp::renderMenuBar() {
       if (ImGui::BeginMainMenuBar()) {
           if (ImGui::BeginMenu("File")) {
               if (ImGui::MenuItem("Load File...")) {
                   openFileDialog();
               }
               if (ImGui::MenuItem("Quit")) {
                   glfwSetWindowShouldClose(window, true);
               }
               ImGui::EndMenu();
           }
           if (ImGui::BeginMenu("View")) {
               if (ImGui::MenuItem("Toggle Debug", nullptr, debugOverlayEnabled)) {
                   debugOverlayEnabled = !debugOverlayEnabled;
               }
               ImGui::EndMenu();
           }
           ImGui::EndMainMenuBar();
       }
   }
   ```

5. **Input Handling:**
   - Check `ImGui::GetIO().WantCaptureMouse` before processing camera mouse input
   - Check `ImGui::GetIO().WantCaptureKeyboard` before processing camera keyboard input

**Files to Create/Modify:**
- `src/imgui/` (new directory) - ImGui library files
- `src/ui/imgui_integration.hpp` (new) - ImGui setup/teardown utilities
- `src/ui/imgui_integration.cpp` (new) - ImGui initialization code
- `src/viewer_app.hpp` (modify) - Add menu state, ImGui context
- `src/viewer_app.cpp` (modify) - Integrate ImGui rendering into main loop
- `CMakeLists.txt` (modify) - Add ImGui sources to build
- `tests/integration/ImGuiIntegrationTests.cpp` (new) - Test menu actions
- `docs/IMGUI_INTEGRATION.md` (new) - Document ImGui setup and menu extension
- `README.md` (modify) - Mention GUI menu support

**Dependencies:**
- ImGui requires GLFW and OpenGL (already available in project)
- ImGui backends: `imgui_impl_glfw.cpp`, `imgui_impl_opengl3.cpp` (included with ImGui)

**Constraints:**
- ImGui adds ~50KB to binary size (acceptable for feature benefits)
- ImGui requires per-frame API calls (immediate mode) - different from retained-mode GUIs
- Must respect ImGui's input capture flags to avoid conflicts with camera controls

---

### Definition of Done

- [ ] ImGui integrated into project build system
- [ ] Menu bar renders correctly in application
- [ ] All specified menu items functional (Load File, Quit, Toggle Debug)
- [ ] Input handling correctly respects menu focus (no camera control when menu active)
- [ ] Code written and peer-reviewed
- [ ] Unit tests pass (min. 85% coverage on new code)
- [ ] Integration tests verify menu actions work correctly
- [ ] No new linter/compiler warnings
- [ ] CI passes (ImGui works in headless mode or conditionally compiled out for tests)
- [ ] Documentation complete (`docs/IMGUI_INTEGRATION.md`)
- [ ] README updated
- [ ] Ready to merge to main branch

---

### Subtask Breakdown (Recommended)

Given the size (L), this story should be broken into smaller independent stories:

#### Subtask 4.1: ImGui Library Integration (M, 30-35 requests)
- Add ImGui as dependency (git submodule or copy files)
- Update CMakeLists.txt to build ImGui
- Initialize ImGui context in `ViewerApp`
- Render empty ImGui frame (proof of integration)
- **Deliverable:** ImGui renders in application, ready for UI elements

#### Subtask 4.2: Basic Menu Bar Implementation (S, 20-25 requests)
- Create top menu bar with "File", "View", "Help" sections
- Add "Load File..." and "Quit" to File menu
- Wire menu actions to existing ViewerApp methods
- **Deliverable:** Basic menu structure functional

#### Subtask 4.3: Debug Toggle and View Menu (S, 15-20 requests)
- Add "Toggle Debug Overlay" to View menu
- Wire toggle to existing debug overlay code
- **Deliverable:** Debug overlay controllable via GUI

#### Subtask 4.4: Input Handling and Focus Management (S, 20-25 requests)
- Implement `WantCaptureMouse` / `WantCaptureKeyboard` checks
- Prevent camera controls from activating when menu focused
- Add hotkey to show/hide menu (e.g., F1)
- **Deliverable:** Menu and camera controls coexist without conflicts

#### Subtask 4.5: Documentation and Testing (S, 15-20 requests)
- Write `docs/IMGUI_INTEGRATION.md`
- Integration tests for menu actions
- Update README
- **Deliverable:** Complete documentation and tests

---

### Comments

**User Requirements Confirmed:**
- **Menu Visibility:** Menu visible by default.
- **Screenshot Behavior:** Menu hidden in framebuffer screenshots. Need configuration system to control what elements are visible in exported screenshots (menu, debug overlay, simulation time, etc.).
- **Historical Context:** Old GUI branch showed simulation time inside screenshots. New system should support configurable screenshot overlays.
- **FPS Counter:** Yes, in debug mode.
- **Menu Items to Include:**
  - Playback controls (play/pause, speed slider) - "This is going to be an interesting one" (noted for future complexity)
  - Visual settings (background color, particle colors) - Yes
  - Export/screenshot options - Yes

**Screenshot Configuration Feature:**
This implies a need for:
- `ScreenshotConfig` struct defining what UI elements render in screenshots
- Separate render paths for on-screen display vs. framebuffer capture
- Menu item: "Screenshot Settings..." to configure visible overlays

**Future Work:**
See "Future Vision: Movie Editor with Keyframe Animation" section below for the long-term goal these features build toward.

**ImGui Confirmation:**
User confirmed: "Keep IMGUI" - Dear ImGui is the selected library.

**Alternative GUI Options (if you want to reconsider):**

| Library | Pros | Cons | Verdict |
|---------|------|------|---------|
| **ImGui** | Lightweight, immediate-mode, widely used | Requires per-frame calls | ✅ **Recommended** |
| **NanoGUI** | C++11, OpenGL, simple API | Smaller community, less documentation | ⚠️ Acceptable |
| **Qt** | Fully-featured, native look | Heavy dependency, overkill for tool | ❌ Not recommended |
| **Native GLFW Menus** | Zero dependencies | Platform-specific, very limited | ❌ Not recommended |
| **Custom OpenGL** | Full control | Time-consuming, reinvent wheel | ❌ Not recommended |

**ImGui is the clear winner for this use case.** ✅ **User Confirmed**

**Future Work:**
- Custom ImGui theme matching project branding
- Dockable windows (detach debug overlay into separate window)
- Settings panel with sliders (particle size, camera speed, etc.)
- Help overlay with keyboard shortcut reference
- Playback controls with speed slider (noted as "interesting" - complexity TBD)

---

## Future Vision: Movie Editor with Keyframe Animation

**Long-term Goal:** These four stories are building blocks toward a sophisticated movie editor for scientific visualization.

### The Vision

Create a keyframe-based camera animation system that enables:

1. **Low-Resolution Previewing:**
   - Load simulation on low-resolution monitor
   - Navigate and create keyframes interactively
   - Follow simulation as it progresses (COM Lock feature)
   - Smooth camera transitions between keyframes
   - Save camera path data to file

2. **High-Resolution Rendering:**
   - Load saved camera path
   - Replay at 8K or higher resolution
   - Export as video frames for compilation
   - Framebuffer captures without UI elements

3. **Interactive Editing:**
   - Pause simulation at any frame
   - Rotate camera smoothly to new angle
   - Record translation as new keyframe
   - Bezier/spline interpolation between keyframes

### Technical Note: Paused-State Camera Recording

A critical feature requirement: **Camera recording must continue while simulation is paused.**

**Use Case:**
- Pause simulation at a specific frame of interest
- Spin camera around to explore from multiple angles **while recording**
- Zoom in/out for detailed examination **while recording**
- Create voiceover-ready footage (pause + rotate creates "frozen time" effect)
- Multiple camera angles of same simulation frame for educational/presentation purposes

**Technical Implications:**
- Camera path recording operates independently of simulation playback state
- "Recording" state separate from "Playing/Paused" state
- Timeline can have multiple keyframes at same simulation frame index but different camera positions
- Export rendering must support:
  - Multiple camera angles at single simulation frame
  - Smooth camera interpolation while simulation frame remains static
  - Frame duplication in output (same particle positions, different camera views)

**Workflow Example:**
1. Play simulation to frame 1000
2. **Pause simulation** (particles frozen)
3. **Start camera recording**
4. Rotate 360° around particles over 5 seconds
5. **Stop camera recording**
6. Result: 5 seconds of video showing frozen frame 1000 from rotating perspective
7. (Optional) Add voiceover in separate software describing what's visible

This enables "bullet time" style scientific visualization where time-frozen particles can be examined from all angles with narration.

### Historical Context

This has been a decade-long goal that required:
- Tools (now available via modern AI assistants)
- Time (accelerated development cycle)
- Skillset (enhanced via AI collaboration)

**Recent Progress:** "What I have done in the last 4 days is more than I would have been able to accomplish working full time in 6 months."

### How Current Stories Enable This

- **Story 1 (Gamepad):** Intuitive navigation during keyframe creation
- **Story 2 (Resolution Scaling):** Consistent appearance from preview to final render
- **Story 3 (Runtime Resolution):** Switch from preview to export resolution seamlessly
- **Story 4 (ImGui Menu):** UI foundation for timeline, keyframe editor, camera path visualization

### Future Stories Needed

1. **Keyframe System:**
   - Camera position/orientation capture
   - Timeline UI with keyframe markers
   - Interpolation algorithms (linear, Bezier, spline)
   - Camera path visualization in 3D viewport

2. **Camera Path Recording:**
   - Record manual camera movements
   - **Record camera movements while simulation is paused** (frozen frame inspection)
   - Save to file format (JSON/binary)
   - Load and replay paths
   - Edit/trim/splice paths
   - Independent recording state from playback state (can record camera while sim paused)

3. **High-Resolution Export:**
   - Batch render at target resolution
   - Frame-by-frame export (PNG/PPM sequence)
   - Progress tracking for long renders
   - Resume from interruption

4. **Timeline Editor:**
   - Visual timeline with frame markers
   - Drag-and-drop keyframe editing
   - Preview scrubbing
   - Multi-track support (camera + metadata overlays)

---

## User Feedback Summary

All questions have been answered inline. Key decisions:

**Story 1 - Gamepad:**
- Support all standard controllers (xinput), document with Xbox 360 layout
- Linux primary (Flatpak), Windows/WSL2 secondary
- Hardcoded mapping initially, config file as future story
- No haptic feedback (GLFW limitation)
- No multi-controller support

**Story 2 - Resolution Scaling:**
- Particle size complexity acknowledged (spacing + size adjustments)
- May need configurable particle scale parameter
- Preserve existing 1280x720 baselines (verify via downscaling)
- Goal: same composition, higher detail

**Story 3 - Runtime Resolution:**
- GUI menu only (no hotkey)
- Command-line argument already exists
- Add fullscreen toggle (Alt+Enter)
- Persist resolution choice to config

**Story 4 - ImGui Menu:**
- Visible by default
- Hidden in framebuffer screenshots
- Screenshot configuration system needed
- FPS counter in debug mode
- Include playback controls, visual settings, export options
- ImGui confirmed as library choice

**Implementation Strategy:**
- Use subtask checklists (not separate issues)
- Feed each story to advanced model (Opus) for implementation

---

## Implementation Order Recommendation

Based on dependencies and value delivery, I recommend this implementation order:

### Phase 1: Foundation (High Priority)
**Story 4: ImGui Menu System** (L, 70-95 requests)
- **Why first:** Provides UI foundation for other features
- **Value:** Immediately improves UX (no more memorizing shortcuts)
- **Risk:** Low - ImGui integration well-documented
- **Blocks:** Story 3 (if resolution change via GUI desired)

### Phase 2: Visual Quality (High Priority)
**Story 2: Resolution-Independent Particle Scaling** (M, 45-60 requests)
- **Why second:** Core visual quality improvement
- **Value:** Better experience across all resolutions
- **Risk:** Low - mathematical solution clear
- **Blocks:** Nothing, but improves Story 3 experience

### Phase 3: Usability Enhancement (Medium Priority)
**Story 3: Runtime Resolution Changing** (M, 35-50 requests)
- **Why third:** Integrates well with GUI from Story 4
- **Value:** Convenience feature, removes restart requirement
- **Risk:** Low - standard window management
- **Depends:** Soft dependency on Story 2 for best experience

### Phase 4: Advanced Input (Medium Priority)
**Story 1: Gamepad Support** (L, 80-120 requests)
- **Why fourth:** Nice-to-have, doesn't block other features
- **Value:** Improved comfort for long viewing sessions
- **Risk:** Medium - controller API varies by platform
- **Depends:** None, but GUI from Story 4 could show controller status

---

## Summary

**Status: Refined and Ready for Implementation**

This document contains 4 comprehensive user stories following INVEST principles and Particle-Viewer conventions, with all user requirements confirmed:

1. **Story 1: Gamepad Support** (L, 80-120 requests, Advanced)
   - All standard controllers via xinput, Xbox 360 layout for docs
   - Linux primary, Windows/WSL2 secondary
   - Hardcoded mapping initially
   - GLFW 3.3+ (zero new dependencies)

2. **Story 2: Resolution-Independent Particle Scaling** (M, 45-60 requests, Standard)
   - Same composition across resolutions, higher detail
   - Preserve existing 1280x720 baselines
   - Configurable particle scale parameter
   - Shader-based solution

3. **Story 3: Runtime Resolution Changing** (M, 35-50 requests, Standard)
   - GUI menu only (requires Story 4 first)
   - Fullscreen toggle (Alt+Enter)
   - Persistence to config file
   - State preservation across changes

4. **Story 4: ImGui Menu System** (L, 70-95 requests, Standard)
   - Dear ImGui confirmed
   - Visible by default, hidden in screenshots
   - Screenshot configuration system
   - FPS counter, playback controls, visual settings, export options

**Future Vision:**
These stories build toward a keyframe-based camera animation system for creating high-resolution scientific visualization videos. See "Future Vision: Movie Editor with Keyframe Animation" section for details.

**Implementation Approach:**
- Subtask checklists (not separate issues)
- Feed each story to advanced model (Opus) for implementation
- Follow recommended phase order: GUI → Resolution Scaling → Runtime Resolution → Gamepad

**Next Steps:**
1. ✅ User requirements confirmed
2. ✅ Technical approach validated
3. ✅ Dependencies and constraints documented
4. **Ready:** Begin implementation starting with Story 4 (ImGui Menu)