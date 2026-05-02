---
title: "ImGui Integration"
description: "Patterns for integrating Dear ImGui into Particle-Viewer — FetchContent setup, SDL3 event order, menu system, and debug overlay positioning."
domain: architecture
subdomain: imgui
tags: [architecture, imgui, dear-imgui, c++]
related:
  - "WINDOW_MANAGEMENT.md"
---

# ImGui Integration

Dear ImGui is used for the main menu bar and debug overlay. This document
covers how ImGui is integrated into Particle-Viewer — specifically the
patterns agents should follow when touching `src/ui/` or any code that
interacts with the ImGui layer.

## Architecture

- Dear ImGui is downloaded via CMake `FetchContent` (not vendored). Since
  ImGui has no `CMakeLists.txt`, use `FetchContent_Declare()` with
  `SOURCE_SUBDIR` pointing to a non-existent path plus
  `FetchContent_MakeAvailable()`, then manually add source files to the
  target. This avoids the `FetchContent_Populate()` deprecation warning.

- **FetchContent ordering with CMake 3.31+**: When a dependency has its own
  `CMakeLists.txt` (e.g., SDL3, GoogleTest), its `FetchContent_MakeAvailable()`
  can corrupt internal FetchContent state for subsequent calls in the same
  cmake run. Fix: make all other FetchContent deps available **before** the
  problematic one. In practice, process `imgui` and `googletest` before
  `SDL3`. Declare all deps together at the top, then `MakeAvailable` in safe
  order.

- ImGui renders to the **default framebuffer** (after the FBO blit), so it
  naturally does not appear in FBO-based screenshots or frame recordings.

- **SDL3 event order**: Call `ImGui_ImplSDL3_ProcessEvent(&event)` for
  **every** SDL event **before** application-specific handling in the event
  loop. This ensures ImGui captures input first (e.g., to block key events
  when a text field is focused via `ImGui::GetIO().WantCaptureKeyboard`).

## Menu System (`src/ui/`)

- `MenuState` tracks visibility and debug mode; `MenuActions` communicates
  triggered actions back to `ViewerApp`.
- **F1** toggles menu visibility, **F3** toggles debug mode at runtime.
- `menu_state_.debug_mode` is the runtime source of truth (initialized from
  the `--debug-camera` CLI flag).

## Debug Overlay Positioning

- The debug overlay is an ImGui window positioned via
  `ImGui::SetNextWindowPos()` with `ImGui::GetFrameHeight()` to dynamically
  offset below the menu bar.
- Uses `ImGuiWindowFlags_AlwaysAutoResize` for automatic sizing and anchors
  to the top-right corner.
- When adding new overlays, account for the menu bar height to avoid
  z-order collisions.

## Related

- [WINDOW_MANAGEMENT.md](WINDOW_MANAGEMENT.md) — SDL3 window and resolution management, fullscreen, and config persistence.
