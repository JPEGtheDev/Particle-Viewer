# Steam Deck Controller - Quick Fix Guide

## The Problem
Controller not working on Steam Deck? **Steam Input is blocking GLFW!**

## The Solution (2 Steps)

### Step 1: Disable Steam Input
1. **Right-click** Particle-Viewer in your Steam library
2. Select **Properties**
3. Click the **Controller** tab
4. Find **"Override for [Particle-Viewer]"**
5. Change it to **"Disable Steam Input"**
6. Close the properties window

### Step 2: Restart and Test
1. Close Particle-Viewer if running
2. Launch it again from Steam
3. Check the terminal/console output
4. You should see:
   ```
   === Controller Detection Debug ===
     Slot 0: Steam Deck Controller | Gamepad: YES | Name: ... [SELECTED]
   Controller selected at slot 0
   =================================
   ```

## What to Look For

### ✅ Working (Steam Input Disabled)
```
Slot 0: Steam Deck Controller | Gamepad: YES | Name: Steam Virtual Gamepad [SELECTED]
```

### ❌ Not Working (Steam Input Enabled)
```
Slot 0: Steam Deck Controller | Gamepad: NO
WARNING: No gamepad detected at startup
WARNING: glfwGetGamepadState failed for slot 0
         Steam Input may be interfering - try disabling Steam Input for this app
```

## Why This Happens

Steam Input creates a virtual controller layer that intercepts all controller events. This is great for Steam games with built-in controller remapping, but it prevents GLFW from seeing the "real" gamepad. GLFW can see a joystick, but not a proper gamepad with button mappings.

## Still Not Working?

If you disabled Steam Input and still see issues:

1. **Check console output** - Share it with the developer
2. **Try Desktop Mode** - Launch outside Steam to bypass Steam entirely
3. **Verify GLFW version** - Needs GLFW 3.3+ with gamepad support
4. **Check permissions** - Flatpak may need `--device=all` permission

## Button Mappings (Once Working)

- **Left Stick**: Camera movement (forward/back/strafe)
- **Right Stick**: Camera rotation
- **A**: Play/pause
- **X**: Point lock
- **Y**: COM lock  
- **Back/Select**: Load file
- **L/R Triggers**: Frame seek
- **L/R Bumpers**: Single frame advance
- **L3/R3** (click sticks): Adjust sphere distance

---

For detailed troubleshooting, see: `docs/STEAM_DECK_FIX.md`
