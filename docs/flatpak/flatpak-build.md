---
title: "Flatpak Build Guide"
description: "Building Particle Viewer as a Flatpak locally, including manifest configuration and bundled dependencies."
domain: build
subdomain: flatpak
tags: [build, flatpak, packaging, linux]
related:
  - "FLATPAK.md"
  - "flatpak-gotchas.md"
  - "FLATPAK_VERIFICATION.md"
---

# Flatpak Build Guide

## Local Validation Before Pushing

Use local validation tools to catch errors before CI:

```bash
# Quick validation (30 seconds) — checks YAML, URLs, checksums
./scripts/validate-flatpak-manifest.sh

# Full build test (5–10 minutes) — simulates CI/CD locally
./scripts/test-flatpak-build-local.sh
```

See [`scripts/FLATPAK_TESTING.md`](../../scripts/FLATPAK_TESTING.md) for details.

## Prerequisites

```bash
# Install flatpak and flatpak-builder
sudo apt install flatpak flatpak-builder

# Add Flathub repository
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install the Freedesktop runtime and SDK
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

## Build Steps

**Using the build script (recommended):**
```bash
./scripts/build-flatpak.sh
```

**Manual build:**
```bash
# Build the Flatpak
flatpak-builder --force-clean --repo=repo flatpak-build flatpak/org.particleviewer.ParticleViewer.yaml

# Create a bundle for distribution
flatpak build-bundle repo particle-viewer.flatpak org.particleviewer.ParticleViewer
```

**Clean rebuild:**
```bash
./scripts/build-flatpak.sh --clean
```

## Testing the Local Build

```bash
flatpak install --user particle-viewer.flatpak
flatpak run org.particleviewer.ParticleViewer
flatpak run --log-session-bus org.particleviewer.ParticleViewer  # view logs
```

## Manifest Configuration

The manifest (`flatpak/org.particleviewer.ParticleViewer.yaml`) defines:

- **Runtime:** `org.freedesktop.Platform 24.08`
- **SDK:** `org.freedesktop.Sdk 24.08`
- **Permissions:** X11, Wayland, `--device=all` (required for NVIDIA GPU detection), home/documents/downloads filesystem access, file picker portal support

## Bundled Dependencies

| Dependency | Version | Notes |
|------------|---------|-------|
| GLM | 1.0.1 | Header-only math library |
| SDL3 | release-3.2.6 | Built as standalone module; pkg-config paths and X11/Wayland detection depend on build order |
| OpenGL | runtime | Mesa `GL.default`; hardware acceleration requires a matching NVIDIA extension |
| tinyFileDialogs | embedded | — |
| GLAD | embedded | — |
| stb | embedded | — |
| Dear ImGui | fetched | Downloaded via CMake FetchContent at build time |

## Related

- [FLATPAK.md](FLATPAK.md) — Distribution overview and navigation
- [flatpak-gotchas.md](flatpak-gotchas.md) — Troubleshooting build and runtime issues
- [FLATPAK_VERIFICATION.md](FLATPAK_VERIFICATION.md) — Verification checklist for CI and releases
