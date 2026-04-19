---
title: "Release Automation Pipeline"
description: "How GitHub Actions detects source changes, calculates versions, and creates releases for Particle-Viewer."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, release, automation, github-actions, workflow]
related:
  - "RELEASE_PROCESS.md"
  - "release-process-versioning.md"
---

# Release Automation Pipeline

## Release Triggers

### Automatic (Push to Master)

Triggered automatically when commits are pushed to `master`.

**Source files that trigger releases:**
- `src/` — C++ source, headers, shaders
- `CMakeLists.txt` — build configuration
- `cmake/` — CMake modules
- `flatpak/` — packaging files (`.yaml`, `.yml`, `.desktop`, `.svg`, `.metainfo.xml`)

**Non-source files that skip releases:**
- `tests/`, `docs/`, `.github/`, `README.md`, `CHANGELOG.md`, `.clang-format`, `.clang-tidy`, `scripts/`

### Manual Dispatch

1. Go to **Actions** → **Automated Release**
2. Click **Run workflow**
3. Optionally specify bump type (major/minor/patch) to override auto-detection
4. Click **Run workflow**

## What Happens During a Release

1. **Source Change Detection** — skips release if only tests, docs, or CI configs changed
2. **Version Detection** — reads current version from most recent git tag; analyzes commits since last tag
3. **Version Calculation** — computes new MAJOR.MINOR.PATCH
4. **Changelog Generation** — groups commits by type (Added/Fixed/Breaking/Changed)
5. **Git Tag** — creates annotated tag (e.g., `v0.2.0`) and **pushes it first**
6. **GitHub Release** — creates release referencing the pushed tag; includes changelog

> The tag MUST be pushed before the GitHub Release is created. `gh release create` requires the tag to already exist in the remote.

## Example

```bash
git commit -m "feat: add particle size control"
git commit -m "fix: correct camera rotation bug"
git push origin master

# Workflow:
# 1. Detects src/main.cpp changed → proceeds
# 2. 1 feat + 1 fix → minor bump (0.1.0 → 0.2.0)
# 3. Generates notes, creates tag v0.2.0, publishes release
```

## Related

- [Release Process](RELEASE_PROCESS.md) — parent ToC stub
- [release-process-versioning.md](release-process-versioning.md) — how commits map to version bumps
- [release-process-operations.md](release-process-operations.md) — workflow config and troubleshooting
