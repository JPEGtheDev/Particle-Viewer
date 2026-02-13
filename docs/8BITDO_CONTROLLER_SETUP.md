# 8BitDo Wireless Controller Setup

## Overview

8BitDo Bluetooth controllers are now supported via **Raw Joystick Mode**. This is a fallback mode that reads controller input directly when GLFW doesn't have a proper gamepad mapping for your controller model.

## Quick Setup

1. **Pair your 8BitDo controller** via Bluetooth
2. **Launch Particle-Viewer**
3. **Check console output** - you should see:
   ```
   *** No gamepad mapping found for: 8BitDo Wireless Controller
   *** Using RAW JOYSTICK MODE (experimental)
   ```

That's it! The controller should work immediately.

## What is Raw Joystick Mode?

8BitDo controllers often don't appear in GLFW's gamepad database, so they're detected as generic "joysticks" rather than "gamepads". Raw Joystick Mode:

- Uses GLFW's raw joystick API instead of gamepad API
- Maps axes and buttons directly using Xbox-style layout assumptions
- Works for most 8BitDo models (SN30 Pro, Pro 2, Ultimate, etc.)

## Button Mapping

Raw Joystick Mode assumes standard Xbox-style button layout:

### Sticks & Triggers
| Axis Index | Function |
|------------|----------|
| 0 | Left Stick X (strafe) |
| 1 | Left Stick Y (forward/back) |
| 2 | Right Stick X (pan camera) |
| 3 | Right Stick Y (tilt camera) |
| 4 | Left Trigger (rewind frames) |
| 5 | Right Trigger (fast-forward frames) |

### Buttons
| Button Index | Function |
|--------------|----------|
| 0 | A - Play/Pause |
| 1 | B - (unused) |
| 2 | X - Point Lock |
| 3 | Y - COM Lock |
| 4 | LB - Previous Frame |
| 5 | RB - Next Frame |
| 6 | Back/Select - Load File |
| 7 | Start - (unused) |
| 8 | L3 (Left Stick Click) - Sphere Closer |
| 9 | R3 (Right Stick Click) - Sphere Farther |
| 10 | D-Pad Up |
| 11 | D-Pad Down |
| 12 | D-Pad Left |
| 13 | D-Pad Right |

## 8BitDo Firmware Modes

8BitDo controllers support multiple firmware modes. For best compatibility:

### Recommended Mode: X-Input Mode

Hold **Start + X** while powering on to enter X-Input mode. This mode:
- ✅ Best compatibility with Linux
- ✅ Most likely to have GLFW gamepad mapping
- ✅ Standard Xbox button layout

### Alternative Modes

- **D-Input Mode** (Start + B): May work with Raw Joystick Mode
- **Switch Mode** (Start + Y): May have different button mapping
- **Mac Mode** (Start + A): Not recommended for Linux

## Troubleshooting

### Controller Detected But Buttons Don't Work

**Symptom:** Console shows controller detected, but buttons have no effect

**Solutions:**
1. **Check button mapping** - Your 8BitDo model may have different button indices
2. **Try different firmware mode** - X-Input mode is most compatible
3. **Check console for errors** - Look for warnings about axis/button count

### Wrong Button Layout

**Symptom:** Buttons do unexpected actions

**Possible causes:**
- Different 8BitDo model with different layout
- Using Switch/Mac firmware mode instead of X-Input
- Controller in compatibility mode

**Solution:** Switch to X-Input mode (Start + X at power-on)

### Sticks Not Working

**Symptom:** Camera doesn't move when using sticks

**Check:**
1. Verify axes are being read (check console for axis count)
2. Ensure deadzone isn't too high (default is 15%)
3. Try moving sticks fully - partial movement may be within deadzone

### Controller Not Detected At All

**Symptom:** No controller shown in console output

**Solutions:**
1. **Check Bluetooth pairing** - Ensure controller is connected to Linux
2. **Verify with `jstest`** - Run `jstest /dev/input/js0` (install `joystick` package)
3. **Check permissions** - Flatpak may need `--device=all` permission
4. **Try wired connection** - Connect via USB cable to rule out Bluetooth issues

## Advanced: Adding GLFW Gamepad Mapping

If Raw Joystick Mode works but you want proper gamepad recognition, you can add a custom mapping:

### 1. Find Your Controller's Mapping String

Run this to test buttons/axes:
```bash
jstest /dev/input/js0
```

### 2. Create Mapping String

Format: `GUID,Name,button:value,axis:value,...`

Example for 8BitDo SN30 Pro:
```
05000000c82d00001038000000010000,8BitDo SN30 Pro,a:b0,b:b1,x:b2,y:b3,back:b6,start:b7,leftshoulder:b4,rightshoulder:b5,leftstick:b8,rightstick:b9,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,dpup:b10,dpdown:b11,dpleft:b12,dpright:b13,
```

### 3. Add to GLFW Database

Create or edit `~/.local/share/glfw/gamecontrollerdb.txt` and add your mapping line.

### 4. Restart Application

Controller should now be detected as gamepad instead of joystick.

## Compatibility

### Tested Models
- ✅ 8BitDo SN30 Pro (Raw Joystick Mode)
- ✅ 8BitDo Pro 2 (Raw Joystick Mode)
- ⚠️ Other models should work but may need button mapping adjustments

### Linux Distributions
- ✅ Ubuntu/Debian (kernel 5.4+)
- ✅ Fedora/RHEL
- ✅ Arch Linux
- ✅ Steam Deck (disable Steam Input first)

## Getting Help

If your 8BitDo controller still doesn't work:

1. **Share console output** - Copy the "Controller Detection Debug" section
2. **Run jstest** - Share output of `jstest /dev/input/js0`
3. **Report model** - Specify which 8BitDo model you're using
4. **Report firmware mode** - Which mode (X-Input, D-Input, Switch, Mac)

## Related Documentation

- [CONTROLLER_MAPPING.md](CONTROLLER_MAPPING.md) - Full button mapping reference
- [STEAM_DECK_FIX.md](STEAM_DECK_FIX.md) - Steam Deck specific issues
- [STEAM_DECK_QUICK_FIX.md](../STEAM_DECK_QUICK_FIX.md) - Quick troubleshooting

## Technical Notes

### Why 8BitDo Controllers Need Raw Joystick Mode

- GLFW maintains a gamepad database (SDL_GameControllerDB format)
- 8BitDo controllers have many models with varying USB IDs
- Not all 8BitDo controllers are in the database
- Raw Joystick Mode bypasses the mapping requirement
- Trade-off: Assumes Xbox button layout (may not match all models)

### Future Improvements

- [ ] Auto-detect 8BitDo-specific button layouts
- [ ] Per-model button mapping profiles
- [ ] Runtime button remapping GUI
- [ ] Custom mapping file support
