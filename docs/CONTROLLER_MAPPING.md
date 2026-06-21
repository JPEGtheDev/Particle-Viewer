---
title: "Controller Mapping"
description: "Gamepad button layout and input mappings for Particle-Viewer, covering camera, playback, and file controls with keyboard equivalents."
domain: architecture
subdomain: sdl3
tags: [architecture, sdl3, controller, input]
related:
  - "WINDOW_MANAGEMENT.md"
---

# Controller Mapping

## Requirements

- Standard xinput-compatible gamepad; no extra drivers needed.
- Only one controller supported. If multiple are connected, the first detected is used.

## Button Layout (Xbox Controller)

```text
         +------------------------------------------+
         |  LB                                    RB |
         |  LT --> rewind <--             --> forward <-- RT |
         |                                          |
         |  +------+   SELECT  START   +------+    |
         |  | L    |     [< ]  [ >]    |  R   |    |
         |  |stick |                   | stick|    |
         |  +------+  D-PAD            +------+    |
         |               ^             Y(^)         |
         |             <- + ->       X(<)   B(>)     |
         |               v             A(v)         |
         +------------------------------------------+
```

## Input Mappings

> **Input modes:** All mappings below apply in **View Mode** (the default). Opening the controller panel (Start or Esc) switches to **Menu Mode**, where **gamepad** camera, playback, and file inputs are suspended. See [Controller Panel / Menu Navigation](#controller-panel--menu-navigation) for Menu Mode bindings.

### Camera Movement

| Input | Action |
|-------|--------|
| **Left stick** -- up/down | Move camera forward / backward |
| **Left stick** -- left/right | Strafe camera left / right |
| **Right stick** -- left/right | Rotate camera yaw (look left / right) |
| **Right stick** -- up/down | Rotate camera pitch (look up / down) |

> In **orbit mode** (activated with **X**), the left stick orbits the focal sphere instead of moving the camera.  The right stick Y-axis zooms in/out (adjusts sphere distance) rather than pitching the camera.

### Point Lock / Sphere Controls

| Input | Action |
|-------|--------|
| **B button** | Cycle point lock state: Off -> Point visible -> Orbit locked |
| **Y button** | Toggle COM (centre-of-mass) lock -- only active in orbit mode |
| **L3** (left stick click) | Move rotation sphere closer (zoom in) |
| **R3** (right stick click) | Move rotation sphere further (zoom out) |

Point lock states match the **P** key on the keyboard:
- **Off** -- free camera, no sphere displayed
- **Point visible** (sphere red) -- sphere displayed, camera moves freely
- **Orbit locked** (sphere green) -- camera orbits around the sphere

### Camera Speed

| Input | Action |
|-------|--------|
| **X button** (hold) | Speed boost -- 20x movement speed while held (mirrors Shift key) |

### Frame Playback

| Input | Action |
|-------|--------|
| **A button** | Toggle play / pause |
| **Right bumper (RB)** | Advance one frame |
| **Left bumper (LB)** | Rewind one frame |
| **Right trigger (RT)** | Fast-forward (3 frames per tick while held) |
| **Left trigger (LT)** | Rewind (3 frames per tick while held) |

### File / Application

| Input | Action |
|-------|--------|
| **Select / Back** | Open file load dialog *(View Mode only; suspended in Menu Mode)* |
| **Start** | Open / close controller panel |

### Controller Panel / Menu Navigation

Opening the controller panel switches to **Menu Mode**. All **gamepad** camera, playback, and file inputs are suspended until the panel is closed.

| Input | Action |
|-------|--------|
| **Start** | Open / close controller panel *(any mode)* |
| **Esc** *(keyboard)* | Open / close controller panel *(any mode)* |
| **B button** | Close panel, return to View Mode *(Menu Mode only)* |
| **D-pad ^ / v** | Navigate panel items |
| **A button** | Select / activate highlighted item |

The controller panel exposes: Fullscreen, Auto-COM, Debug Mode, Quit, Load File, Recording Folder, and Close.

> **Quit** is available only via the controller panel or the File menu. Esc no longer quits the application -- it opens the controller panel.

## Keyboard Equivalents

| Gamepad | Keyboard |
|---------|----------|
| A | Space |
| B | P |
| X (hold) | Shift (speed boost) |
| Y | O |
| Select/Back | T |
| Start | Esc (open / close controller panel) |
| RB / LB | -> / <- |
| RT / LT | E / Q |
| L3 / R3 | `[` / `]` |
| Left stick | W / A / S / D |
| Right stick | I / K / J / L |

## Disconnection Handling

If the controller disconnects, input falls back to keyboard/mouse. Reconnecting resumes gamepad control automatically.

## Limitations (v1)

- Single controller only.
- Button mapping is not user-configurable in this release.
- No haptic/rumble feedback.

## Related

- [WINDOW_MANAGEMENT.md](WINDOW_MANAGEMENT.md) -- SDL3 window and resolution management, fullscreen, and config persistence.
- [IMGUI_INTEGRATION.md](IMGUI_INTEGRATION.md) -- ImGui menu system and event handling order.
