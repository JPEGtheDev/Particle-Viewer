---
title: "Flatpak CI/CD and Troubleshooting"
description: "CI/CD integration details and troubleshooting for build and runtime issues in the Particle Viewer Flatpak."
domain: build
subdomain: flatpak
tags: [build, flatpak, packaging, linux]
related:
  - "FLATPAK.md"
  - "flatpak-build.md"
  - "FLATPAK_VERIFICATION.md"
---

# Flatpak CI/CD and Troubleshooting

## CI/CD Integration

### Pull Request Builds

Opening a PR that modifies relevant source files triggers an automatic Flatpak build:

1. Workflow triggers on PR creation/update
2. Flatpak is built in GitHub Actions
3. Artifact uploaded as `particle-viewer-flatpak-pr-<NUMBER>`
4. A bot comment posts download and install instructions

Workflow file: `.github/workflows/flatpak-pr.yml`

### Release Builds

On every release (when a version change is detected):

1. Release workflow builds the Flatpak automatically
2. `.flatpak` bundle is attached to the GitHub release
3. Users can download and install from the release page

Workflow file: `.github/workflows/release.yml`

## Troubleshooting

### Build Issues

**"Runtime not found"**
```bash
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

**"Permission denied" on build script**
```bash
chmod +x scripts/build-flatpak.sh
```

### Runtime Issues

**Application won't start**
```bash
flatpak run --log-session-bus org.particleviewer.ParticleViewer
flatpak run --verbose org.particleviewer.ParticleViewer
```

**`SDL_CreateWindow failed: Invalid window driver data` on NVIDIA**

This occurs when the installed Flatpak NVIDIA GL extension does not match your driver version exactly. The application detects this and falls back to Mesa software rendering (llvmpipe) with a startup warning.

To restore hardware acceleration:
```bash
# Find your driver version
nvidia-smi --query-gpu=driver_version --format=csv,noheader

# Install the matching GL extension (24.08 branch)
sudo flatpak install flathub org.freedesktop.Platform.GL.nvidia-<YOUR_VER>//1.4
# Example:
sudo flatpak install flathub org.freedesktop.Platform.GL.nvidia-580-126-18//1.4
```

If the exact version isn't on Flathub yet, the software fallback keeps the app usable.

> **Note:** Three env vars are required together for Mesa software rendering on an NVIDIA X11 session: `LIBGL_ALWAYS_SOFTWARE=1`, `GALLIUM_DRIVER=llvmpipe`, and `__GLX_VENDOR_LIBRARY_NAME=mesa`. Setting only `LIBGL_ALWAYS_SOFTWARE=1` is insufficient — GLVND still queries the X server extension and tries to load `libGLX_nvidia.so`. The application sets all three automatically.

**Can't access certain files**
- Ensure files are in home or XDG directories
- Use the file picker dialog for sandboxed file access

### File Permissions

The Flatpak grants: `--filesystem=home`, `--filesystem=xdg-documents`, `--filesystem=xdg-download`

To grant additional access:
```bash
flatpak override --user --filesystem=/path/to/directory org.particleviewer.ParticleViewer
```

## Related

- [FLATPAK.md](FLATPAK.md) — Distribution overview and navigation
- [flatpak-build.md](flatpak-build.md) — Building locally and manifest configuration
- [FLATPAK_VERIFICATION.md](FLATPAK_VERIFICATION.md) — Verification checklist for CI and releases
