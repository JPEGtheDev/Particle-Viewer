---
title: "Window and Resolution Management"
description: "How to configure window resolution, fullscreen mode, and persistence settings in Particle-Viewer."
domain: architecture
subdomain: sdl3
tags: [architecture, sdl3, window, opengl]
related:
  - "IMGUI_INTEGRATION.md"
---

# Window and Resolution Management

## Changing Resolution

### Via Menu

1. Press **F1** to show the menu (if hidden)
2. Navigate to **View → Resolution**
3. Select your desired resolution:
   - **720p (1280x720)** - Standard HD
   - **1080p (1920x1080)** - Full HD
   - **1440p (2560x1440)** - 2K/QHD
   - **4K (3840x2160)** - Ultra HD

**Note:** The application clamps resolutions larger than your monitor while maintaining aspect ratio. The menu shows "(clamped to WxH)" next to options that exceed your monitor size.

### Via Command Line

You can specify an initial resolution when launching:

```bash
./Viewer --resolution 1920x1080
# or shorthand:
./Viewer --res 1080p
```

### Manual Resizing

You can also manually resize the window by dragging the edges. The viewport and projection automatically adjust to maintain aspect ratio.

## Fullscreen Mode

### Toggle Fullscreen

Press **Alt+Enter** at any time to toggle between windowed and fullscreen mode.

- **Windowed → Fullscreen**: Switches to your monitor's native resolution
- **Fullscreen → Windowed**: Restores your previous windowed size

### Behavior

- Fullscreen uses your monitor's native resolution and refresh rate
- The application remembers your windowed size when entering fullscreen
- Your fullscreen preference is saved to configuration

## Configuration Persistence

Window settings (resolution and fullscreen state) are automatically saved when you:
- Change resolution via the menu
- Toggle fullscreen mode
- Manually resize the window

### Configuration File Location

Settings are stored in:
- **Linux/macOS**: `~/.config/particle-viewer/window.cfg`
- **Windows**: `%APPDATA%\particle-viewer\window.cfg`

### Configuration File Format

The configuration uses a simple INI-style format:

```ini
# Particle-Viewer Window Configuration
width=1920
height=1080
fullscreen=0
```

**Fields:**
- `width` - Window width in pixels
- `height` - Window height in pixels
- `fullscreen` - `1` for fullscreen, `0` for windowed

### Resetting Configuration

Delete the configuration file to reset to defaults (1280x720 windowed).

## Tips

### Display Limitations

If you try to set a resolution larger than your monitor, the application will:
1. Maintain the aspect ratio of your chosen resolution
2. Scale it down to fit your monitor
3. Show you the clamped resolution in the menu

**Example**: Selecting 4K (3840x2160) on a 1920x1080 monitor will clamp to 1920x1080.

### Performance Considerations

Higher resolutions require more GPU power. For low framerates, try a lower resolution.

### Debug Mode

Press **F3** to toggle debug mode, which shows:
- Current resolution
- Camera position and orientation
- Frame rate (FPS)

This can help verify resolution settings are applied correctly.

## Troubleshooting

### Window Too Small/Large

If the window size is uncomfortable:
1. Use the **View → Resolution** menu to select a preset
2. Or manually resize by dragging window edges
3. Your preference will be saved automatically

### Fullscreen Exits Immediately

If fullscreen mode exits immediately after toggling:
- Check if you have multiple monitors
- Ensure your monitor is properly detected
- Try manually resizing the window before entering fullscreen

### Resolution Not Saved

If your resolution isn't persisting between sessions:
- Check file permissions on the config directory
- Ensure the config directory is writable:
  - Linux: `~/.config/particle-viewer/`
  - Windows: `%APPDATA%\particle-viewer\`
- The application will log "Failed to save window configuration" if saving fails

### Visual Distortion

If particles or the scene appears stretched:
- Try toggling fullscreen and back to reset
- If the issue persists, restart the application

## Related

- [IMGUI_INTEGRATION.md](IMGUI_INTEGRATION.md) — Dear ImGui integration, menu system, and SDL3 event order.
- [CONTROLLER_MAPPING.md](CONTROLLER_MAPPING.md) — Gamepad input mappings and keyboard equivalents.
