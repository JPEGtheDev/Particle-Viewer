# User Stories: GUI and Input Enhancement Features

**Created:** 2026-02-11  
**Project:** Particle-Viewer  
**Status:** Draft - Awaiting Feedback

---

## Table of Contents

1. [Story 1: Gamepad/Xbox Controller Support](#story-1-gamepadxbox-controller-support)
2. [Story 2: Resolution-Independent Particle Scaling](#story-2-resolution-independent-particle-scaling)
3. [Story 3: Runtime Resolution Changing](#story-3-runtime-resolution-changing)
4. [Story 4: ImGui Menu System](#story-4-imgui-menu-system)
5. [Questions for Refinement](#questions-for-refinement)
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
     - ✅ Standard Xbox controller mapping
     - ❌ Limited to basic gamepad features (no haptics, LED control)
  
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

**Recommendation:** Use **GLFW 3.3+ gamepad support** to avoid new dependencies. If more advanced features (haptics, extensive controller DB) are needed later, consider SDL2.

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

**Question for User:**
- Should this support PlayStation controllers (DualShock/DualSense) as well, or Xbox only?
- Target platform priority: Linux first, then Windows? Or both simultaneously?
- Do you want the controller mapping to be configurable (remappable buttons), or hardcoded as specified?

**Future Work:**
- Custom button remapping via config file
- Haptic feedback (rumble on frame boundaries, collisions in simulation)
- Multi-controller support (unlikely needed for viewer)

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

**Question for User:**
- What percentage of the viewport should a "standard" particle occupy at reference resolution? 
  - Suggestion: 1-3% of viewport height for typical particles
  - Allows user to configure via settings file later
- Should this affect ALL particles uniformly, or should different particle types have different scaling factors?

**Visual Regression Test Impact:**
- Baseline images will need regeneration after this change
- Recommend capturing baselines at: 1280x720, 1920x1080, 2560x1440
- Test should verify: "particle at screen center occupies X% ± 2% of viewport height"

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

**Question for User:**
- Should resolution changes be available via:
  - GUI menu only? (preferred, wait for Story 4)
  - Temporary hotkey (e.g., Ctrl+R) for now?
  - Command-line argument to set resolution before startup?
- Should the application support fullscreen mode toggle (Alt+Enter)?
- Should resolution changes persist across application restarts (saved to config)?

**Integration with Story 4 (GUI Menu):**
- If GUI menu implemented first, resolution selector can be dropdown in menu
- If this implemented first, add temporary keyboard shortcut, migrate to GUI later

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

**Question for User:**
- Do you want the menu visible by default, or hidden (press F1 to show)?
- Should the menu include FPS counter or performance stats?
- Future menu items to plan for:
  - Playback controls (play/pause, speed slider)?
  - Visual settings (background color, particle colors)?
  - Export/screenshot options?

**Alternative GUI Options (if you want to reconsider):**

| Library | Pros | Cons | Verdict |
|---------|------|------|---------|
| **ImGui** | Lightweight, immediate-mode, widely used | Requires per-frame calls | ✅ **Recommended** |
| **NanoGUI** | C++11, OpenGL, simple API | Smaller community, less documentation | ⚠️ Acceptable |
| **Qt** | Fully-featured, native look | Heavy dependency, overkill for tool | ❌ Not recommended |
| **Native GLFW Menus** | Zero dependencies | Platform-specific, very limited | ❌ Not recommended |
| **Custom OpenGL** | Full control | Time-consuming, reinvent wheel | ❌ Not recommended |

**ImGui is the clear winner for this use case.**

**Future Work:**
- Custom ImGui theme matching project branding
- Dockable windows (detach debug overlay into separate window)
- Settings panel with sliders (particle size, camera speed, etc.)
- Help overlay with keyboard shortcut reference

---

## Questions for Refinement

Please answer these questions so I can refine the stories:

### Gamepad Support (Story 1)
1. **Platform priority:** Should gamepad support work on Linux first, Windows first, or both simultaneously?
2. **Controller types:** Xbox only, or should we also support PlayStation controllers (DualShock/DualSense)?
3. **Button remapping:** Should the button mapping be configurable by users, or hardcoded as specified?
4. **Timing:** Is this feature high priority, or can it come after the GUI menu?

### Resolution Scaling (Story 2)
5. **Particle size:** What percentage of the viewport should a standard particle occupy at 1080p? (Suggestion: 1-3% of viewport height)
6. **Particle types:** Should all particles scale uniformly, or should different particle types (if they exist) have different scaling factors?

### Runtime Resolution (Story 3)
7. **Interface:** Should resolution changes be available via:
   - GUI menu only (requires Story 4 first)
   - Temporary hotkey (e.g., Ctrl+R) for now, migrate to GUI later
   - Both simultaneously
8. **Persistence:** Should the resolution choice persist across application restarts (saved to config file)?
9. **Fullscreen:** Should we also add fullscreen/windowed mode toggle (Alt+Enter)?

### GUI Menu (Story 4)
10. **Default state:** Should the menu be visible by default, or hidden (press F1 to toggle)?
11. **Additional features:** Should the menu include:
    - FPS counter / performance stats?
    - Playback controls (play/pause, speed slider)?
    - Visual settings panel (colors, rendering options)?
12. **Library confirmation:** Are you happy with ImGui recommendation, or would you like me to elaborate on alternatives?

### General Questions
13. **Implementation order:** Which story should be implemented first? My recommendation:
    - **Phase 1:** Story 4 (GUI Menu) - Foundation for other features
    - **Phase 2:** Story 2 (Resolution Scaling) - Improves core UX
    - **Phase 3:** Story 3 (Runtime Resolution) - Integrates with GUI
    - **Phase 4:** Story 1 (Gamepad) - Advanced feature, can come later
14. **Breaking down large stories:** Stories 1 and 4 are L-sized. Should I create separate GitHub issues for each subtask, or keep them as single issues with subtask checklists?

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

I've created 4 comprehensive user stories following INVEST principles and Particle-Viewer conventions:

1. **Gamepad Support** - Full Xbox controller integration (L-sized, break into subtasks)
2. **Resolution Scaling** - Consistent particle sizes across resolutions (M-sized)
3. **Runtime Resolution** - Change resolution without restart (M-sized)
4. **ImGui Menu** - Professional menu system (L-sized, break into subtasks)

Each story includes:
- Effort estimates (premium requests, model tier recommendation)
- Detailed acceptance criteria
- Technical notes with implementation guidance
- Definition of Done checklist
- Specific questions for you to answer
- Recommended subtask breakdown for L-sized stories

**Next Steps:**
1. Please review and answer the questions in the "Questions for Refinement" section
2. Confirm or adjust the implementation order recommendation
3. Decide if you want L-sized stories broken into separate GitHub issues
4. I can then refine the stories based on your feedback and create GitHub issue templates if desired

---

**Questions or feedback? Let me know what you'd like to adjust!**
