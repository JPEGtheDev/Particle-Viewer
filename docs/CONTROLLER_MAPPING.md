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
         ┌──────────────────────────────────────────┐
         │  LB                                    RB │
         │  LT ──► rewind ◄──             ──► forward ◄── RT │
         │                                          │
         │  ┌──────┐   SELECT  START   ┌──────┐    │
         │  │ L    │     [◄ ]  [ ►]    │  R   │    │
         │  │stick │                   │ stick│    │
         │  └──────┘  D-PAD            └──────┘    │
         │               ↑             Y(▲)         │
         │             ← + →       X(◄)   B(►)     │
         │               ↓             A(▼)         │
         └──────────────────────────────────────────┘
```

## Input Mappings

### Camera Movement

| Input | Action |
|-------|--------|
| **Left stick** — up/down | Move camera forward / backward |
| **Left stick** — left/right | Strafe camera left / right |
| **Right stick** — left/right | Rotate camera yaw (look left / right) |
| **Right stick** — up/down | Rotate camera pitch (look up / down) |

> In **orbit mode** (activated with **X**), the left stick orbits the focal sphere instead of moving the camera.  The right stick Y-axis zooms in/out (adjusts sphere distance) rather than pitching the camera.

### Point Lock / Sphere Controls

| Input | Action |
|-------|--------|
| **B button** | Cycle point lock state: Off → Point visible → Orbit locked |
| **Y button** | Toggle COM (centre-of-mass) lock — only active in orbit mode |
| **L3** (left stick click) | Move rotation sphere closer (zoom in) |
| **R3** (right stick click) | Move rotation sphere further (zoom out) |

Point lock states match the **P** key on the keyboard:
- **Off** — free camera, no sphere displayed
- **Point visible** (sphere red) — sphere displayed, camera moves freely
- **Orbit locked** (sphere green) — camera orbits around the sphere

### Camera Speed

| Input | Action |
|-------|--------|
| **X button** (hold) | Speed boost — 20× movement speed while held (mirrors Shift key) |

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
| **Select / Back** | Open file load dialog |

## Keyboard Equivalents

| Gamepad | Keyboard |
|---------|----------|
| A | Space |
| B | P |
| X (hold) | Shift (speed boost) |
| Y | O |
| Select/Back | T |
| RB / LB | → / ← |
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

- [WINDOW_MANAGEMENT.md](WINDOW_MANAGEMENT.md) — SDL3 window and resolution management, fullscreen, and config persistence.
- [IMGUI_INTEGRATION.md](IMGUI_INTEGRATION.md) — ImGui menu system and event handling order.
