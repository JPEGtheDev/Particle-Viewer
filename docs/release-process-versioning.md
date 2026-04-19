---
title: "Semantic Versioning and Conventional Commits"
description: "How commit types map to MAJOR/MINOR/PATCH version bumps in the Particle-Viewer release pipeline."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, release, versioning, conventional-commits, git]
related:
  - "RELEASE_PROCESS.md"
  - "CONVENTIONAL_COMMITS.md"
---

# Semantic Versioning and Conventional Commits

## Semantic Versioning

The project follows [Semantic Versioning 2.0.0](https://semver.org/):

| Segment | Meaning |
|---------|---------|
| **MAJOR** | Breaking changes or incompatible API changes |
| **MINOR** | New features, backward-compatible |
| **PATCH** | Bug fixes, backward-compatible |

At build time, the version is taken from the `PROJECT_VERSION` build variable, falling back to git tags, then `0.0.0`.

## Commit Types → Version Bumps

| Commit Type | Version Bump |
|------------|--------------|
| `feat:` | **MINOR** |
| `fix:` | **PATCH** |
| `feat!:` or `BREAKING CHANGE:` footer | **MAJOR** |
| `docs:`, `style:`, `refactor:`, `test:`, `chore:`, `perf:` | **PATCH** |

## Examples

**Patch release (0.1.0 → 0.1.1):**
```bash
git commit -m "fix: resolve crash when loading empty particle files"
```

**Minor release (0.1.0 → 0.2.0):**
```bash
git commit -m "feat: add support for particle velocity visualization"
```

**Major release (0.1.0 → 1.0.0):**
```bash
git commit -m "feat!: redesign shader pipeline with breaking changes"
```

Or with breaking change footer:
```bash
git commit -m "feat: redesign shader pipeline

BREAKING CHANGE: Shader configuration API has changed"
```

## Version Storage

**Single source of truth: git tags** (e.g., `v0.1.0`).

- **Git tags**: created automatically by the release workflow
- **CMakeLists.txt**: accepts version as variable or reads from git tags as fallback
- **GitHub Releases**: contain generated changelog for each release

## Related

- [Release Process](RELEASE_PROCESS.md) — parent ToC stub and full pipeline overview
- [Conventional Commits](CONVENTIONAL_COMMITS.md) — quick reference for commit format
- [release-process-automation.md](release-process-automation.md) — how the workflow runs
