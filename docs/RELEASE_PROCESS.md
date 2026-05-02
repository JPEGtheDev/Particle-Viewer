---
title: "Release Process"
description: "Automated semantic versioning and release pipeline for Particle-Viewer using conventional commits and GitHub Actions."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, release, workflow, versioning, conventional-commits]
related:
  - "CONVENTIONAL_COMMITS.md"
  - "release-process-versioning.md"
  - "release-process-automation.md"
  - "release-process-operations.md"
---

# Automated Release Process

This project uses a **zero-manual release pipeline**. All version increments, changelog updates, tagging, and releases are handled automatically by GitHub Actions when commits are pushed to `master`.

## Contents

| Concept | File | Coverage |
|---------|------|----------|
| Versioning | [release-process-versioning.md](release-process-versioning.md) | Semantic versioning, commit→version-bump mapping, version storage |
| Automation | [release-process-automation.md](release-process-automation.md) | Trigger conditions, release steps, source file patterns |
| Operations | [release-process-operations.md](release-process-operations.md) | Zero-manual requirements, workflow config, troubleshooting |

## How It Works

1. Push commits to `master` using **conventional commit format**
2. GitHub Actions detects source file changes
3. Analyzes commits to determine version bump (major/minor/patch)
4. Creates git tag and publishes GitHub Release with generated changelog

## Quick Reference

| Commit Type | Version Bump | Example |
|------------|--------------|---------|
| `feat:` | Minor | `feat: add particle color picker` |
| `fix:` | Patch | `fix: resolve startup crash` |
| `feat!:` | Major | `feat!: redesign shader API` |
| `docs:`, `test:`, `chore:` | Patch | — |

Releases are **skipped** when only non-source files change (`tests/`, `docs/`, `.github/`).

## Related

- [Conventional Commits](CONVENTIONAL_COMMITS.md) — commit format quick reference
- [release-process-versioning.md](release-process-versioning.md) — how commits map to version bumps
- [release-process-automation.md](release-process-automation.md) — full release pipeline steps
