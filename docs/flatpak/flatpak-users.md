---
title: "Flatpak User Guide"
description: "Installing, running, and managing Particle Viewer as a Flatpak package on Linux."
domain: build
subdomain: flatpak
tags: [build, flatpak, packaging, linux]
related:
  - "FLATPAK.md"
  - "flatpak-build.md"
  - "flatpak-gotchas.md"
---

# Flatpak User Guide

## Prerequisites

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

## Installation

### From GitHub Releases

1. Download the `.flatpak` file from the [latest release](https://github.com/JPEGtheDev/Particle-Viewer/releases)
2. Install:
   ```bash
   flatpak install --user particle-viewer-<version>.flatpak
   ```
3. Run:
   ```bash
   flatpak run org.particleviewer.ParticleViewer
   ```

### System-Wide Installation

Requires root:
```bash
sudo flatpak install particle-viewer.flatpak
```

## Running the Application

**Command line:**
```bash
flatpak run org.particleviewer.ParticleViewer
```

Or use the **Particle Viewer** desktop launcher in your application menu.

## File Access

The Flatpak has access to:
- Home directory (`~/`)
- XDG directories (Documents, Downloads, etc.)

Load particle simulation files from these locations via the file picker.

## Managing Permissions

Use Flatseal (GUI) or the command line:

```bash
# View current permissions
flatpak override --user org.particleviewer.ParticleViewer --show

# Grant access to a specific directory
flatpak override --user --filesystem=/path/to/directory org.particleviewer.ParticleViewer

# Install Flatseal (permission manager GUI)
flatpak install flathub com.github.tchx84.Flatseal
```

## Uninstalling

```bash
flatpak uninstall org.particleviewer.ParticleViewer
```

## Development Shell

To open a shell inside the Flatpak sandbox:
```bash
flatpak run --command=bash org.particleviewer.ParticleViewer
```

## Resources

- [Flatpak Documentation](https://docs.flatpak.org/)
- [Flathub](https://flathub.org/)

## Related

- [FLATPAK.md](FLATPAK.md) — Distribution overview and navigation
- [flatpak-build.md](flatpak-build.md) — Building the Flatpak from source
- [flatpak-gotchas.md](flatpak-gotchas.md) — Troubleshooting runtime and permission issues
