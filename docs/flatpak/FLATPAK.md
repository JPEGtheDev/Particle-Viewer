# Flatpak Distribution Guide

This document provides comprehensive information about building, installing, and distributing Particle Viewer as a Flatpak package.

## What is Flatpak?

Flatpak is a universal package format for Linux that provides:
- **Sandboxed execution** for security and stability
- **Dependency isolation** - all dependencies are bundled
- **Cross-distribution compatibility** - works on any Linux distribution
- **Easy installation** - single-file bundles (.flatpak)

## For Users

### Installation

#### From GitHub Releases

1. Download the `.flatpak` file from the [latest release](https://github.com/JPEGtheDev/Particle-Viewer/releases)
2. Install the Flatpak:
   ```bash
   flatpak install --user particle-viewer-<version>.flatpak
   ```
3. Run the application:
   ```bash
   flatpak run org.particleviewer.ParticleViewer
   ```

#### Prerequisites

Ensure Flatpak is installed on your system:

**Ubuntu/Debian:**
```bash
sudo apt install flatpak
```

**Fedora:**
```bash
sudo dnf install flatpak
```

**Arch Linux:**
```bash
sudo pacman -S flatpak
```

### Running the Application

After installation, you can run Particle Viewer in several ways:

1. **Command line:**
   ```bash
   flatpak run org.particleviewer.ParticleViewer
   ```

2. **Desktop launcher:** Look for "Particle Viewer" in your application menu

### File Access

The Flatpak version has access to:
- Your home directory (`~/`)
- XDG directories (Documents, Downloads, etc.)

You can load particle simulation files from any of these locations through the file picker.

### Uninstalling

```bash
flatpak uninstall org.particleviewer.ParticleViewer
```

## For Developers

### Local Testing & Validation

**NEW: Test your changes before pushing!** 🎯

Before pushing changes to the Flatpak manifest, use our local validation tools to catch errors early:

```bash
# Quick validation (30 seconds) - checks YAML, URLs, checksums
./scripts/validate-flatpak-manifest.sh

# Full build test (5-10 minutes) - simulates CI/CD locally
./scripts/test-flatpak-build-local.sh
```

**Benefits:**
- ✅ Catch errors before they hit CI/CD
- ✅ No more "fix CI" commits
- ✅ Faster iteration cycle
- ✅ Confidence your changes will work

See [`scripts/FLATPAK_TESTING.md`](../../scripts/FLATPAK_TESTING.md) for detailed usage guide.

### Building Locally

#### Prerequisites

Install the required Flatpak tools:

```bash
# Install flatpak and flatpak-builder
sudo apt install flatpak flatpak-builder

# Add Flathub repository (for runtime dependencies)
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install the Freedesktop runtime and SDK
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

#### Build Steps

1. **Using the build script (recommended):**
   ```bash
   ./scripts/build-flatpak.sh
   ```

2. **Manual build:**
   ```bash
   # Build the Flatpak
   flatpak-builder --force-clean --repo=repo flatpak-build flatpak/org.particleviewer.ParticleViewer.yaml
   
   # Create a bundle for distribution
   flatpak build-bundle repo particle-viewer.flatpak org.particleviewer.ParticleViewer
   ```

3. **Clean rebuild:**
   ```bash
   ./scripts/build-flatpak.sh --clean
   ```

#### Testing Locally

Install and test your local build:

```bash
# Install
flatpak install --user particle-viewer.flatpak

# Run
flatpak run org.particleviewer.ParticleViewer

# View logs
flatpak run --log-session-bus org.particleviewer.ParticleViewer
```

### Manifest Configuration

The Flatpak manifest (`flatpak/org.particleviewer.ParticleViewer.yaml`) defines:

- **Runtime:** org.freedesktop.Platform 24.08
- **SDK:** org.freedesktop.Sdk 24.08
- **Permissions:**
  - Graphics: X11, Wayland, all devices (`--device=all` required for NVIDIA GPU detection inside the sandbox)
  - File system: home directory, documents, downloads
  - File picker portal support

### Dependencies

The Flatpak bundles the following dependencies:

1. **GLM** (1.0.1) - Header-only math library
2. **SDL3** (release-3.2.6) - Window and OpenGL context management (built as a standalone Flatpak module before the app so that the Freedesktop SDK pkg-config paths are available and `SDL_X11`/`SDL_WAYLAND` are detected correctly)
3. **OpenGL** - Provided by the runtime (Mesa `GL.default` extension; hardware acceleration requires a matching NVIDIA GL extension — see Troubleshooting)
4. **tinyFileDialogs** - Embedded in source
5. **GLAD** - Embedded in source
6. **stb libraries** - Embedded in source
7. **Dear ImGui** - Downloaded via CMake FetchContent at build time

## CI/CD Integration

### Pull Request Builds

When you open a pull request that modifies relevant files, a beta Flatpak is automatically built:

1. The workflow triggers on PR creation/updates
2. Flatpak is built using GitHub Actions
3. The build artifact is uploaded with a PR-specific name
4. A comment is posted on the PR with download instructions

This allows testing Flatpak packaging changes before merging.

### Release Builds

On every release (when version changes are detected):

1. The release workflow automatically builds the Flatpak
2. The `.flatpak` bundle is attached to the GitHub release
3. Users can download and install directly from the release page

### Workflow Files

- **`.github/workflows/flatpak-pr.yml`** - PR build workflow
- **`.github/workflows/release.yml`** - Release workflow with Flatpak integration

## Troubleshooting

### Build Issues

**Problem:** "Runtime not found"
```bash
# Install the runtime
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

**Problem:** "Permission denied" errors
```bash
# Make build script executable
chmod +x scripts/build-flatpak.sh
```

### Runtime Issues

**Problem:** Application won't start
```bash
# Check logs
flatpak run --log-session-bus org.particleviewer.ParticleViewer

# Try with verbose logging
flatpak run --verbose org.particleviewer.ParticleViewer
```

**Problem:** `SDL_CreateWindow failed: Invalid window driver data` on NVIDIA

This happens when the installed Flatpak NVIDIA GL extension does not exactly match your installed driver. For example, driver `580.126.18` requires extension `nvidia-580-126-18`, but only `nvidia-580-126-16` may be available on flathub.

The application detects this automatically and falls back to Mesa software rendering (llvmpipe) with reduced performance. A warning message is printed at startup.

To restore hardware-accelerated rendering, install the matching extension:
```bash
# Find your driver version
nvidia-smi --query-gpu=driver_version --format=csv,noheader
# e.g. output: 580.126.18 → install nvidia-580-126-18

# Install the matching GL extension (24.08 branch for this runtime)
sudo flatpak install flathub org.freedesktop.Platform.GL.nvidia-<YOUR_VER>//1.4
# Example:
sudo flatpak install flathub org.freedesktop.Platform.GL.nvidia-580-126-18//1.4
```

If the exact version isn't on flathub yet (extensions lag behind driver releases by a few days), the software rendering fallback keeps the app usable in the meantime.

> **Note:** Three env vars must be set together for Mesa software rendering to work on an NVIDIA X11 session: `LIBGL_ALWAYS_SOFTWARE=1`, `GALLIUM_DRIVER=llvmpipe`, and `__GLX_VENDOR_LIBRARY_NAME=mesa`. Setting only `LIBGL_ALWAYS_SOFTWARE=1` is insufficient — GLVND still queries the X server extension (which advertises NVIDIA) and tries to load the absent `libGLX_nvidia.so`. The application sets all three automatically.

**Problem:** Can't access certain files
- Check that files are in accessible locations (home directory or XDG directories)
- Use the file picker dialog for sandboxed file access

### File Permissions

The Flatpak has the following filesystem permissions:
- `--filesystem=home` - Full home directory access
- `--filesystem=xdg-documents` - Documents folder
- `--filesystem=xdg-download` - Downloads folder

To grant additional permissions:
```bash
# Example: Allow access to a specific directory
flatpak override --user --filesystem=/path/to/directory org.particleviewer.ParticleViewer
```

## Advanced Topics

### Custom Installations

For system-wide installation (requires root):
```bash
sudo flatpak install particle-viewer.flatpak
```

### Updating Permissions

Use Flatseal (GUI) or the command line:
```bash
# Install Flatseal
flatpak install flathub com.github.tchx84.Flatseal

# Or use command line
flatpak override --user org.particleviewer.ParticleViewer --show
```

### Development Environment

To modify files inside the Flatpak sandbox:
```bash
flatpak run --command=bash org.particleviewer.ParticleViewer
```

## Resources

- [Flatpak Documentation](https://docs.flatpak.org/)
- [Flatpak Builder](https://docs.flatpak.org/en/latest/flatpak-builder.html)
- [Freedesktop SDK](https://gitlab.com/freedesktop-sdk/freedesktop-sdk)
- [Flathub](https://flathub.org/)

## Future Improvements

- [ ] Submit to Flathub for wider distribution
- [x] Add application icon (completed - collision visualization SVG)
- [ ] Implement desktop file actions
- [ ] Add screenshots to AppStream metadata
- [ ] Consider implementing native file portal support
