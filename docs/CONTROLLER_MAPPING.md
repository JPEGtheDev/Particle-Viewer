#Controller Mapping Guide

This guide documents the gamepad/controller button mappings for Particle-Viewer.

## Supported Controllers

Particle-Viewer uses the GLFW 3.3+ gamepad API, which provides standardized input mapping for:
- Xbox 360 Controller
- Xbox One Controller  
- Xbox Series X/S Controller
- Steam Deck (built-in controller)
- Generic gamepads with xinput mapping

The controller is **automatically detected** at startup by scanning all available joystick slots. The first detected gamepad will be used.

The documentation uses Xbox 360 button labels as the standard reference.


## Control Mapping

### Movement Controls

| Input | Action | Notes |
|-------|--------|-------|
| **Left Stick X-Axis** | Strafe Left/Right | Tank controls - move perpendicular to view direction |
| **Left Stick Y-Axis** | Move Forward/Backward | Push up to move forward, down to move back |
| **Right Stick X-Axis** | Rotate Camera (Yaw) | Pan camera left/right |
| **Right Stick Y-Axis** | Rotate Camera (Pitch) | Tilt camera up/down |

**Note:** When point lock is active, Right Stick Y-Axis controls zoom instead of pitch.

### Frame Playback Controls

| Input | Action | Notes |
|-------|--------|-------|
| **A Button** | Toggle Play/Pause | Equivalent to Space key |
| **Left Trigger** | Rewind Frames | Continuous - hold to rewind faster |
| **Right Trigger** | Fast-Forward Frames | Continuous - hold to advance faster |
| **Left Bumper (LB)** | Previous Frame | Single frame backward |
| **Right Bumper (RB)** | Next Frame | Single frame forward |

### Camera Lock Controls

| Input | Action | Notes |
|-------|--------|-------|
| **X Button** | Toggle Point Lock | Cycles through rotation states (unlocked → bracket → locked) |
| **Y Button** | Toggle COM Lock | Only active when point lock is enabled |
| **L3 (Left Stick Click)** | Move Sphere Closer | When sphere is visible |
| **R3 (Right Stick Click)** | Move Sphere Farther | When sphere is visible |

### System Controls

| Input | Action | Notes |
|-------|--------|-------|
| **Back/Select Button** | Open File Dialog | Load simulation data |
| **Start Button** | _(Reserved)_ | Not currently mapped |
| **B Button** | _(Reserved)_ | Not currently mapped |

## Advanced Features

### Analog Sensitivity

- **Deadzone**: 15% - Prevents stick drift from affecting camera/movement
- **Rotation Speed**: 2.5° per frame at full stick deflection
- **Movement Speed**: Matches keyboard movement speed (scales with delta time)

### Zoom Control (Point Lock Active)

When point lock is enabled (X button):
- **Right Stick Y-Axis** switches from camera pitch to zoom control
- Push up to zoom in (decrease sphere distance)
- Push down to zoom out (increase sphere distance)
- X-axis rotation still works normally

### Trigger Behavior

Triggers are analog (0.0 to 1.0):
- Threshold: 20% - Pressing beyond 20% activates the function
- Seek speed scales with trigger pressure (1-5 frames per update)

## Keyboard/Mouse Compatibility

Controller input does **not** disable keyboard or mouse control. Both input methods work simultaneously:
- Use controller for smooth analog movement and rotation
- Use keyboard for precise single-key actions
- Switch between input methods at any time

## Controller Connection

### Connection Detection

The application automatically detects gamepad connection:
1. Connect controller before or after starting the application
2. No configuration needed - works out of the box
3. Status is checked every frame (no polling delay)

### Disconnection Handling

If the controller disconnects during use:
- Controller input stops immediately
- Keyboard/mouse input continues to work normally
- Reconnect controller to resume gamepad control

## Limitations

Current implementation has the following limitations:

1. **No Haptic Feedback**: GLFW 3.3 does not provide vibration/rumble support
2. **Single Controller Only**: Only the first connected controller (GLFW_JOYSTICK_1) is used
3. **No Custom Remapping**: Button mappings are hardcoded (future enhancement)
4. **No D-Pad Mapping**: D-pad buttons are not currently assigned functions

## Troubleshooting

### Controller Not Detected

**Symptom**: Controller is connected but not responding

**Solutions**:
1. Check if controller is recognized by your OS:
   - Linux: Run `jstest /dev/input/js0` (install `joystick` package)
   - Windows: Check "Set up USB game controllers" in Control Panel
2. Ensure controller has xinput driver support
3. Try disconnecting and reconnecting the controller
4. Restart the application

### Unexpected Button Behavior

**Symptom**: Buttons do not match documented layout

**Solution**: Ensure you're using an xinput-compatible controller. Non-standard controllers may have different mappings.

### Stick Drift

**Symptom**: Camera/movement happens without touching sticks

**Solution**: The 15% deadzone should prevent most drift. If it persists:
1. Calibrate your controller using OS tools
2. Clean the controller (dust can affect sensors)
3. Replace worn analog sticks

## Platform-Specific Notes

### Steam Deck

- Built-in controller is automatically detected and supported
- Works in both Desktop Mode and Game Mode
- When adding as a non-Steam app, ensure the application has controller permissions
- Controller appears at joystick slot 0 (automatically detected)
- All standard Xbox button mappings apply (A/B/X/Y, triggers, bumpers, sticks)

### Linux

- Most Xbox controllers work via the `xpad` kernel driver
- Generic controllers may require `xboxdrv` for xinput emulation
- Flatpak users: Controller access requires the `--device=all` permission

### Windows/WSL2

- Xbox controllers work natively with Windows driver
- WSL2: USB passthrough required for controller access
- Non-Xbox controllers may need Xbox 360 controller emulator (x360ce)

## Testing

To verify controller is working:

1. **Connect Controller**: Plug in Xbox 360/One controller
2. **Launch Application**: Start Particle-Viewer
3. **Test Movement**: Push left stick - camera should move
4. **Test Rotation**: Push right stick - camera should rotate
5. **Test Buttons**: Press A button - playback should toggle

### Automated Testing Limitations

**GLFW does not provide a mock or fake gamepad API** for automated testing. This means:

- Unit tests can only verify state structures, deadzone logic, and connection detection
- Full button mapping and input response requires manual testing with physical hardware
- No xinput emulator or virtual controller support in GLFW 3.3+

**Potential Workarounds for Future Versions:**

1. **Platform-specific input injection** (complex, platform-dependent):
   - Linux: Write to `/dev/uinput` to create virtual controller
   - Windows: Use virtual Xbox controller drivers (x360ce, ViGEm)
   - Requires elevated privileges and platform-specific code

2. **Abstraction layer** (architectural change):
   - Create `IControllerInput` interface
   - Implement `GLFWControllerInput` (production) and `MockControllerInput` (testing)
   - Would enable full integration testing without hardware

3. **Record/replay system** (future enhancement):
   - Record real controller input during manual testing
   - Replay recorded inputs for regression testing
   - Requires input capture/playback infrastructure

For v1, manual testing with physical controllers remains the most practical approach.

## Future Enhancements

Planned improvements for future releases:

- [ ] User-customizable button remapping (via config file)
- [ ] D-pad mapping for additional shortcuts
- [ ] Multi-controller support
- [ ] Controller configuration GUI menu
- [ ] Sensitivity adjustment sliders
- [ ] Profile support (save/load button layouts)

## Related Documentation

- [TESTING_STANDARDS.md](TESTING_STANDARDS.md) - Controller testing guidelines
- [CODING_STANDARDS.md](CODING_STANDARDS.md) - Code style for input handling
- [README.md](../README.md) - General usage and setup

## Credits

Controller support implemented using:
- GLFW 3.3+ Gamepad API
- Standard xinput button mapping
- Tank control scheme for intuitive navigation
