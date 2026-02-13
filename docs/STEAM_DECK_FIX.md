# Steam Deck Controller Fix

## Problem
The controller was not working on Steam Deck when running the application as a non-Steam app in Game Mode.

## Root Cause
The controller input system was hardcoded to only check joystick slot 1 (`GLFW_JOYSTICK_2` in GLFW naming, which is index 1), but Steam Deck's built-in controller appears at slot 0 (`GLFW_JOYSTICK_1`, which is index 0).

### GLFW Naming Convention
GLFW's joystick naming is confusing:
- `GLFW_JOYSTICK_1` = index 0 (first joystick)
- `GLFW_JOYSTICK_2` = index 1 (second joystick)
- etc.

## Solution
Modified the `ControllerInput` constructor to scan all joystick slots (0-15) and use the first detected gamepad.

### Code Changes
**Before:**
```cpp
ControllerInput() : joystick_id_(GLFW_JOYSTICK_2), was_connected_(false) {}
```

**After:**
```cpp
ControllerInput() : joystick_id_(GLFW_JOYSTICK_1), was_connected_(false) {
    // Scan all joystick slots to find first available gamepad
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
            joystick_id_ = i;
            std::cout << "Controller detected at slot " << i << ": " 
                      << glfwGetGamepadName(i) << std::endl;
            break;
        }
    }
}
```

## Benefits
1. **Steam Deck Support**: Controller now works on Steam Deck (slot 0)
2. **Improved Compatibility**: Works with controllers in any slot
3. **Better Diagnostics**: Console message shows which slot and controller name
4. **Future-Proof**: Will find controllers regardless of slot assignment

## Testing on Steam Deck

### Game Mode (Non-Steam App)
1. Add Particle-Viewer as a non-Steam app
2. Launch from Game Mode
3. Check that console shows: "Controller detected at slot 0: [controller name]"
4. Test all button mappings (A/B/X/Y, triggers, bumpers, sticks)

### Desktop Mode
1. Launch Particle-Viewer normally
2. Controller should still work (slot 0 detection)

## Troubleshooting

### Controller Still Not Detected
If you see no detection message:
1. Ensure GLFW has gamepad mappings for your controller
2. Check `/usr/share/glfw/gamecontrollerdb.txt` for mapping entries
3. Verify controller works in other Steam games

### Wrong Button Mappings
If buttons do wrong actions:
1. This usually means GLFW doesn't have correct mapping for your controller
2. Steam Input may be interfering (try disabling Steam Input for this app)
3. Check GLFW gamepad database for your specific controller model

## Related Files
- `src/input/controller_input.hpp` - Controller detection and input handling
- `docs/CONTROLLER_MAPPING.md` - Full button mapping documentation
- `src/viewer_app.cpp` - Controller input processing

## Future Enhancements
- Add support for custom gamepad mappings via config file
- Support multiple controllers simultaneously
- Add GUI for controller configuration
- Display controller status in UI
