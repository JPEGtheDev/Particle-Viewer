# Automated Release Process

This document describes the fully automated semantic versioning and release pipeline for the Particle-Viewer project.

## Overview

The project uses **automated semantic versioning** with a zero-manual release pipeline. All version increments, changelog updates, tagging, and releases are handled automatically by GitHub Actions.

## Semantic Versioning

The project follows [Semantic Versioning 2.0.0](https://semver.org/):
- **MAJOR** version: Breaking changes or incompatible API changes
- **MINOR** version: New features in a backward-compatible manner
- **PATCH** version: Backward-compatible bug fixes

At build time, the current version is taken from the PROJECT_VERSION build variable (for example via -DPROJECT_VERSION=...), falling back to git tags when that is not set, and to 0.0.0 if neither is available.

## Conventional Commits

The release automation uses [Conventional Commits](https://www.conventionalcommits.org/) to automatically determine version bumps:

### Commit Message Format
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Commit Types and Version Bumps

| Commit Type | Version Bump | Example |
|------------|--------------|---------|
| `feat:` | **MINOR** | `feat: add particle color customization` |
| `fix:` | **PATCH** | `fix: resolve memory leak in renderer` |
| `feat!:` or `BREAKING CHANGE:` | **MAJOR** | `feat!: redesign configuration API` |
| Other types | **PATCH** (default) | `docs: update README` |

### Examples

**Patch Release (0.1.0 → 0.1.1):**
```bash
git commit -m "fix: resolve crash when loading empty particle files"
```

**Minor Release (0.1.0 → 0.2.0):**
```bash
git commit -m "feat: add support for particle velocity visualization"
```

**Major Release (0.1.0 → 1.0.0):**
```bash
git commit -m "feat!: redesign shader pipeline with breaking changes"
```

Or with footer:
```bash
git commit -m "feat: redesign shader pipeline

BREAKING CHANGE: Shader configuration API has changed"
```

## Release Triggers

The automated release workflow can be triggered in two ways:

### 1. Automatic on Push to Master (Recommended)
When commits are pushed to the `master` branch, the workflow automatically:
1. **Checks for source code changes** - Skips release if only non-source files changed
2. Analyzes commit messages since the last release
3. Determines the appropriate version bump
4. Generates release notes from commit history for the GitHub Release (does not modify VERSION or CHANGELOG.md directly)
5. Creates a git tag and GitHub release

**Source File Patterns (trigger releases):**
- `src/` - All C++ source code, headers, and shaders
- `CMakeLists.txt` - Build configuration
- `cmake/` - CMake modules
- `flatpak/*.yaml`, `flatpak/*.yml`, `flatpak/*.desktop`, `flatpak/*.svg`, `flatpak/*.metainfo.xml` - Flatpak packaging

**Non-Source Files (skip releases):**
- `tests/` - Unit tests and test infrastructure
- `docs/` - Documentation
- `.github/` - CI/CD workflows and configurations
- `README.md`, `CHANGELOG.md` - Project documentation
- `.clang-format`, `.clang-tidy` - Code quality tools
- `scripts/` - Development and build scripts

### 2. Manual Workflow Dispatch
For manual control, you can trigger a release from GitHub Actions:
1. Go to **Actions** → **Automated Release**
2. Click **Run workflow**
3. Optionally specify the bump type (major/minor/patch) to override auto-detection
4. Click **Run workflow** to start

## What Happens During a Release

When the release workflow runs:

1. **Source Change Detection**
   - Compares files changed since last release tag
   - Checks against source file patterns (src/, CMakeLists.txt, cmake/, flatpak manifests)
   - **Skips release** if only tests, docs, or CI configs changed
   - **Proceeds with release** if any source files were modified

2. **Version Detection**
   - Reads current version from most recent git tag
   - Analyzes commits since last tag using conventional commit format
   - Determines version bump type (major/minor/patch)

3. **Version Calculation**
   - Calculates new version number based on bump type
   - Validates version format (MAJOR.MINOR.PATCH)

4. **Changelog Generation**
   - Parses all commits since last release
   - Groups changes by type (Added, Fixed, Breaking Changes, Changed)
   - Generates formatted changelog entry with date

5. **Git Operations**
   - Creates annotated git tag with new version (e.g., `v0.2.0`)
   - **Pushes tag to remote** (required before GitHub Release creation)

6. **GitHub Release**
   - Creates GitHub release referencing the pushed tag
   - Includes generated changelog as description
   - Tags the release with version number
   - Publishes release to GitHub Releases page

7. **Version in Build**
   - CMakeLists.txt can accept version as variable from CI or read from git tags locally

### Important: Tag Push Order

The workflow explicitly pushes the git tag **before** creating the GitHub Release. This is required because:
- `gh release create` expects the tag to already exist in the remote repository
- Pushing the tag first ensures proper synchronization
- This approach provides better error handling and transparency

## Zero-Manual Requirements

To maintain the zero-manual release process:

✅ **DO:**
- Use conventional commit messages for all commits
- Push commits to master to trigger automatic releases
- Let the workflow determine version bumps automatically
- Review releases in GitHub Releases page after automation

❌ **DON'T:**
- Manually create git tags
- Manually edit version in CMakeLists.txt

## Version Storage

**Single Source of Truth:** Git tags

- **Git tags**: Created automatically by release workflow (e.g., `v0.1.0`)
- **CMakeLists.txt**: Can accept version as variable or read from git tags as fallback
- **GitHub Releases**: Contain generated changelog for each release

## Workflow Configuration

The release workflow is defined in `.github/workflows/release.yml` with:

- **Concurrency control**: Prevents simultaneous releases
- **Permissions**: `contents: write` for creating releases
- **Skip CI**: Version bump commits include `[skip ci]` to prevent recursive workflows
- **Full history**: Fetches complete git history for changelog generation

## Troubleshooting

### Release didn't trigger automatically
- Verify commits are pushed to `master` branch
- **Check if only non-source files changed** (tests, docs, CI) - release will be skipped
- Review GitHub Actions logs for skip messages
- Review GitHub Actions logs for errors

### Wrong version bump detected
- Ensure commit messages follow conventional commit format
- Use manual workflow dispatch with explicit bump type
- Check that commit prefixes are lowercase (`feat:` not `Feat:`)

### Workflow fails with permission error
- Verify GitHub Actions has `contents: write` permission
- Check that GITHUB_TOKEN has not expired

### Duplicate releases created
- Workflow has concurrency control to prevent this
- Check if multiple workflows were triggered simultaneously

## Best Practices

1. **Write meaningful commit messages**: They become your release notes
2. **Use conventional commits consistently**: Ensures correct version bumps
3. **Test before merging to master**: Releases are automatic on master
4. **Use scopes in commits**: `feat(renderer): add new feature` for better organization

## Release History

All releases are tracked in two places:
1. **Git tags**: Versioned snapshots in git history
2. **GitHub Releases**: Published releases with generated release notes

## Example Workflow

Typical development cycle:

```bash
# Make changes with conventional commits
git commit -m "feat: add particle size control"
git commit -m "fix: correct camera rotation bug"
git commit -m "docs: update API documentation"

# Push to master (triggers automatic release)
git push origin master

# Workflow automatically:
# 1. Detects source file changes (src/main.cpp changed)
# 2. Analyzes commits: 1 feature + 1 fix → minor bump (0.1.0 → 0.2.0)
# 3. Generates release notes from commits
# 4. Creates tag v0.2.0
# 5. Publishes GitHub release with notes
```

### When Release is Skipped

If only non-source files are changed:

```bash
# Make test and documentation changes
git commit -m "test: add unit tests for camera rotation"
git commit -m "docs: improve README examples"

# Push to master
git push origin master

# Workflow automatically:
# 1. Detects only tests/ and docs/ files changed
# 2. Skips release creation (no source code changes)
# 3. Shows info message: "Only non-source files changed"
```

## Future Enhancements

Potential improvements to the release pipeline:
- Build and attach compiled binaries to releases
- Automated release notes with contributor recognition
- Integration with package registries
- Automated dependency vulnerability scanning
- Release branch support for maintenance versions

## References

- [Semantic Versioning 2.0.0](https://semver.org/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Keep a Changelog](https://keepachangelog.com/)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
