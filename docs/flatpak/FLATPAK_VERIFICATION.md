---
title: "Flatpak Implementation Verification Guide"
description: "Steps to verify Flatpak PR and release workflows, local builds, and runtime functionality for Particle Viewer."
domain: build
subdomain: flatpak
tags: [build, flatpak, verification, testing]
related:
  - "FLATPAK.md"
  - "flatpak-build.md"
  - "flatpak-gotchas.md"
---

# Flatpak Implementation Verification Guide

## PR Workflow Testing

### When to Test

The PR workflow triggers automatically when a pull request is opened or updated against `src/`, `CMakeLists.txt`, or Flatpak files.

### Expected Behavior

1. **Workflow Triggers** — `Build Flatpak (PR)` starts in the Actions tab
2. **Build Success** — workflow completes with a green checkmark
3. **Artifact Upload** — artifact named `particle-viewer-flatpak-pr-<NUMBER>` is available
4. **PR Comment** — bot posts download and installation instructions

### Manual Verification

```bash
# Download artifact, extract zip, then:
flatpak install --user particle-viewer-pr-<NUMBER>.flatpak
flatpak run org.particleviewer.ParticleViewer
# Verify: app launches, file loading works, 3D rendering works, no dependency errors
```

## Release Workflow Testing

### When to Test

Runs on push to master when version changes are detected, or via manual workflow dispatch.

### Expected Behavior

1. **Version Detection** — workflow detects version bump from commit messages
2. **Release Created** — GitHub release tagged `v<VERSION>`
3. **Flatpak Build Job** — separate job builds Flatpak in container
4. **Asset Upload** — Flatpak bundle attached to the release

### Manual Verification

Check the [Releases page](https://github.com/JPEGtheDev/Particle-Viewer/releases) for a `.flatpak` asset, then:

```bash
flatpak install --user particle-viewer-<VERSION>.flatpak
flatpak run org.particleviewer.ParticleViewer
```

## Local Build Testing

### Prerequisites

```bash
sudo apt install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

### Build and Test

```bash
cd /path/to/Particle-Viewer
./scripts/build-flatpak.sh
# Expected: build completes, particle-viewer.flatpak created in root
flatpak install --user particle-viewer.flatpak
flatpak run org.particleviewer.ParticleViewer
```

### Clean Build

```bash
./scripts/build-flatpak.sh --clean
```

## PR Review Checklist

- [ ] YAML workflows are syntactically valid
- [ ] Flatpak manifest includes all dependencies
- [ ] Desktop entry and AppStream metadata are valid
- [ ] Build script is executable
- [ ] `.gitignore` excludes Flatpak build artifacts
- [ ] PR workflow triggers on correct paths
- [ ] Release workflow separates jobs correctly
- [ ] Documentation is complete (`FLATPAK.md`, `README.md`)

## Common Issues

| Issue | Fix |
|-------|-----|
| Flatpak Builder not found | `sudo apt install flatpak-builder` |
| Runtime not found | `flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08` |
| Permission denied on build script | `chmod +x scripts/build-flatpak.sh` |
| Workflow fails on missing dependencies | Check manifest URLs, SHA256 hashes, and runtime version (24.08) |
| Application won't launch | `flatpak run --verbose org.particleviewer.ParticleViewer` |

## Next Steps

After verification: merge the PR, test the PR workflow on a subsequent PR, trigger a release to validate the release workflow, and consider submitting to Flathub.

## Related

- [FLATPAK.md](FLATPAK.md) — Distribution overview and navigation
- [flatpak-build.md](flatpak-build.md) — Local build guide and manifest reference
- [flatpak-gotchas.md](flatpak-gotchas.md) — Runtime troubleshooting and CI/CD details
