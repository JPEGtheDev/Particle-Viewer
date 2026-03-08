# Controller Mapping

Particle-Viewer supports standard gamepads (Xbox 360, Xbox One, and compatible controllers) via the SDL3 gamepad API.  The mapping below uses Xbox button labels; equivalent PlayStation / generic buttons are identified by position.

## Requirements

- A standard gamepad recognised by the OS as an xinput-compatible device.
- No additional drivers or configuration are required.
- Only one controller is supported at a time.  If multiple controllers are connected, the first one detected is used.

## Button Layout (Xbox Controller)

```
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
| **X button** | Cycle point lock state: Off → Point visible → Orbit locked |
| **Y button** | Toggle COM (centre-of-mass) lock — only active in orbit mode |

Point lock states match the **P** key on the keyboard:
- **Off** — free camera, no sphere displayed
- **Point visible** (sphere red) — sphere displayed, camera moves freely
- **Orbit locked** (sphere green) — camera orbits around the sphere

### Camera Speed

| Input | Action |
|-------|--------|
| **B button** (hold) | Speed boost — 20× movement speed while held (mirrors Shift key) |

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

All gamepad actions have keyboard equivalents.  See the in-app menu (**F1**) for the full keyboard reference.

| Gamepad | Keyboard |
|---------|----------|
| A | Space |
| B (hold) | Shift (speed boost) |
| X | P |
| Y | O |
| Select/Back | T |
| RB / LB | → / ← |
| RT / LT | E / Q |
| Left stick | W / A / S / D |
| Right stick | I / K / J / L |

## Disconnection Handling

If the controller is disconnected while the application is running, input falls back gracefully to keyboard/mouse.  Reconnecting the same controller (or plugging in a new one) resumes gamepad control automatically.

## Limitations (v1)

- Single controller only.
- Button mapping is not user-configurable in this release.  Custom remapping will be addressed in a future story.
- No haptic/rumble feedback.
