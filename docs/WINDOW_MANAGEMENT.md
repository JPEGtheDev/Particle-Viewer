# Window and Resolution Management

Particle-Viewer provides flexible window and resolution management to adapt to your display setup.

## Changing Resolution

### Via Menu

1. Press **F1** to show the menu (if hidden)
2. Navigate to **View → Resolution**
3. Select your desired resolution:
   - **720p (1280x720)** - Standard HD
   - **1080p (1920x1080)** - Full HD
   - **1440p (2560x1440)** - 2K/QHD
   - **4K (3840x2160)** - Ultra HD

**Note:** If your monitor is smaller than the selected resolution, the application will automatically clamp the resolution to fit your monitor while maintaining aspect ratio. The menu will show "(clamped to WxH)" next to options that exceed your monitor size.

### Via Command Line

You can specify an initial resolution when launching:

```bash
./Viewer --resolution 1920x1080
# or shorthand:
./Viewer --res 1080p
```

### Manual Resizing

You can also manually resize the window by dragging the window edges. The viewport and projection will automatically adjust to maintain correct aspect ratio.

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
- `width` - Window width in pixels (windowed mode)
- `height` - Window height in pixels (windowed mode)
- `fullscreen` - `1` for fullscreen, `0` for windowed

### Resetting Configuration

To reset to defaults, simply delete the configuration file. The application will use default settings (1280x720 windowed) on next launch.

## Tips

### Display Limitations

If you try to set a resolution larger than your monitor, the application will:
1. Maintain the aspect ratio of your chosen resolution
2. Scale it down to fit your monitor
3. Show you the clamped resolution in the menu

**Example**: Selecting 4K (3840x2160) on a 1920x1080 monitor will clamp to 1920x1080.

### Performance Considerations

Higher resolutions require more GPU power:
- **720p**: Smooth on most systems
- **1080p**: Recommended for most modern systems
- **1440p**: Requires a mid-range or better GPU
- **4K**: Requires a high-end GPU for smooth framerates

If you experience low framerates, try a lower resolution.

### Debug Mode

Press **F3** to toggle debug mode, which shows:
- Current resolution
- Camera position and orientation
- Frame rate (FPS)

This can help verify your resolution settings are applied correctly.

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
- The projection matrix updates automatically on resize
- Try toggling fullscreen and back to reset
- If the issue persists, restart the application
