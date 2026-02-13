# 8BitDo Controller - Quick Start

Your 8BitDo wireless controller should now work with Particle-Viewer!

## Step 1: Set Controller to X-Input Mode

**Power on your 8BitDo controller while holding: Start + X**

This puts it in X-Input mode, which:
- ✅ Best compatibility with Linux
- ✅ Standard Xbox button layout
- ✅ Works with Raw Joystick Mode

## Step 2: Launch Particle-Viewer

Start the application normally. You should see in the console:

```
=== Controller Detection Debug ===
  Slot 0: 8BitDo Wireless Controller | Gamepad: NO

  *** No gamepad mapping found for: 8BitDo Wireless Controller
  *** Using RAW JOYSTICK MODE (experimental)
  *** Button/axis mapping: Assuming Xbox-style layout

Controller selected at slot 0 (Raw Joystick Mode)
=================================
```

## Step 3: Test Controls

Try these to verify it's working:

- **Left Stick** → Move camera forward/back/strafe
- **Right Stick** → Rotate camera (pan/tilt)
- **A Button** → Toggle play/pause
- **Triggers** → Fast-forward/rewind frames

## If It's Not Working

### 1. Check Console Output
Look for error messages. Common issues:
- "No controller detected" → Check Bluetooth pairing
- "Gamepad: NO" but no "RAW JOYSTICK MODE" → Try restarting app

### 2. Try Different Firmware Mode
If X-Input doesn't work, try:
- **D-Input mode:** Start + B
- **Switch mode:** Start + Y

### 3. Test with jstest
Verify controller works outside the app:
```bash
sudo apt install joystick  # If not installed
jstest /dev/input/js0
```

Move sticks and press buttons. You should see values change.

### 4. Report Issue
If still not working, please share:
- Console output (copy the "Controller Detection Debug" section)
- Output of `jstest /dev/input/js0`
- Which 8BitDo model you're using
- Which firmware mode you're using

## Button Mapping Reference

| Button | Action |
|--------|--------|
| **Left Stick** | Camera movement |
| **Right Stick** | Camera rotation |
| **A** | Play/Pause |
| **B** | (unused) |
| **X** | Point Lock |
| **Y** | COM Lock |
| **LB** | Previous Frame |
| **RB** | Next Frame |
| **LT** | Rewind Frames |
| **RT** | Fast-Forward Frames |
| **Back/Select** | Load File |
| **Start** | (unused) |
| **L3** (click left stick) | Sphere Closer |
| **R3** (click right stick) | Sphere Farther |

## More Help

- **Detailed guide:** See `docs/8BITDO_CONTROLLER_SETUP.md`
- **General controller docs:** See `docs/CONTROLLER_MAPPING.md`
- **Steam Deck issues:** See `docs/STEAM_DECK_FIX.md`
