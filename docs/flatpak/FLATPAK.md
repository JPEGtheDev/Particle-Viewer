---
title: "Flatpak Distribution Guide"
description: "Overview and navigation hub for Flatpak packaging, distribution, and troubleshooting for Particle Viewer."
domain: build
subdomain: flatpak
tags: [build, flatpak, packaging, linux]
related:
  - "flatpak-users.md"
  - "flatpak-build.md"
  - "flatpak-gotchas.md"
  - "FLATPAK_VERIFICATION.md"
---

# Flatpak Distribution Guide

Particle Viewer is distributed as a Flatpak package for cross-distribution Linux compatibility. This document is a navigation hub; detailed topics live in the sub-files below.

## Contents

| File | Topic |
|------|-------|
| [flatpak-users.md](flatpak-users.md) | Installing, running, and managing Particle Viewer as a Flatpak |
| [flatpak-build.md](flatpak-build.md) | Building the Flatpak locally, manifest configuration, and dependencies |
| [flatpak-gotchas.md](flatpak-gotchas.md) | CI/CD integration and troubleshooting runtime and build issues |
| [FLATPAK_VERIFICATION.md](FLATPAK_VERIFICATION.md) | Verification steps for PR and release workflows |

## What is Flatpak?

Flatpak is a universal package format for Linux that provides:
- **Sandboxed execution** for security and stability
- **Dependency isolation** — all dependencies are bundled
- **Cross-distribution compatibility** — works on any Linux distribution
- **Easy installation** — single-file bundles (`.flatpak`)

## Future Improvements

- [ ] Submit to Flathub for wider distribution
- [x] Add application icon (completed — collision visualization SVG)
- [ ] Implement desktop file actions
- [ ] Add screenshots to AppStream metadata
- [ ] Consider implementing native file portal support

## Related

- [flatpak-users.md](flatpak-users.md) — User installation and file access guide
- [flatpak-build.md](flatpak-build.md) — Developer build guide with manifest and dependency details
- [flatpak-gotchas.md](flatpak-gotchas.md) — CI/CD and troubleshooting reference
- [FLATPAK_VERIFICATION.md](FLATPAK_VERIFICATION.md) — Verification checklist for PR and release workflows
