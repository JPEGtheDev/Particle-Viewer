# Automated Release Process

This document describes the fully automated semantic versioning and release pipeline for the Particle-Viewer project.

## Overview

The project uses **automated semantic versioning** with a zero-manual release pipeline. All version increments, changelog updates, tagging, and releases are handled automatically by GitHub Actions.

## Semantic Versioning

The project follows [Semantic Versioning 2.0.0](https://semver.org/):
- **MAJOR** version: Breaking changes or incompatible API changes
- **MINOR** version: New features in a backward-compatible manner
- **PATCH** version: Backward-compatible bug fixes

Current version is stored in the `VERSION` file at the repository root.

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
1. Analyzes commit messages since the last release
2. Determines the appropriate version bump
3. Updates VERSION and CHANGELOG.md
4. Creates a git tag and GitHub release

### 2. Manual Workflow Dispatch
For manual control, you can trigger a release from GitHub Actions:
1. Go to **Actions** → **Automated Release**
2. Click **Run workflow**
3. Optionally specify the bump type (major/minor/patch) to override auto-detection
4. Click **Run workflow** to start

## What Happens During a Release

When the release workflow runs:

1. **Version Detection**
   - Reads current version from `VERSION` file
   - Analyzes commits since last tag using conventional commit format
   - Determines version bump type (major/minor/patch)

2. **Version Calculation**
   - Calculates new version number based on bump type
   - Validates version format (MAJOR.MINOR.PATCH)

3. **Changelog Generation**
   - Parses all commits since last release
   - Groups changes by type (Added, Fixed, Breaking Changes, Changed)
   - Generates formatted changelog entry with date

4. **File Updates & Pull Request**
   - Updates `VERSION` file with new version
   - Inserts new changelog entry into `CHANGELOG.md`
   - Creates a release branch with these changes
   - Opens a pull request for review and merging
   - CMakeLists.txt automatically reads from VERSION file

5. **Git Operations**
   - Creates annotated git tag (e.g., `v0.2.0`)
   - Pushes tag to repository

6. **GitHub Release**
   - Creates GitHub release with changelog as description
   - Tags the release with version number
   - Publishes release to GitHub Releases page

7. **Version Bump PR**
   - A pull request is created with VERSION and CHANGELOG updates
   - PR can be reviewed and merged according to branch protection rules
   - PR includes `[skip ci]` tag to prevent recursive workflows

## Zero-Manual Requirements

To maintain the zero-manual release process:

✅ **DO:**
- Use conventional commit messages for all commits
- Push commits to master to trigger automatic releases
- Let the workflow determine version bumps automatically
- Review releases in GitHub Releases page after automation

❌ **DON'T:**
- Manually edit VERSION file
- Manually create git tags
- Manually edit version in CMakeLists.txt (it reads from VERSION)
- Manually update CHANGELOG.md (except for [Unreleased] section)

## Version Storage

**Single Source of Truth:** The `VERSION` file at repository root

- **VERSION file**: Plain text file containing semantic version (e.g., `0.1.0`)
- **CMakeLists.txt**: Automatically reads version from VERSION file
- **Git tags**: Created automatically by release workflow (e.g., `v0.1.0`)
- **CHANGELOG.md**: Updated automatically with each release

## Workflow Configuration

The release workflow is defined in `.github/workflows/release.yml` with:

- **Concurrency control**: Prevents simultaneous releases
- **Permissions**: `contents: write` for creating releases and tags, `pull-requests: write` for creating PRs
- **Branch Protection Compatibility**: Creates PRs for version bumps instead of pushing directly to master
- **Skip CI**: Version bump commits include `[skip ci]` to prevent recursive workflows
- **Full history**: Fetches complete git history for changelog generation

### Working with Branch Protection

The workflow is designed to work with branch protection rules on the master branch:

1. **Release Tag & GitHub Release**: Created immediately when workflow runs
2. **Version File Updates**: Created as a pull request that respects branch protection
3. **Merge Flow**: After the PR is merged, VERSION and CHANGELOG are updated in master

This ensures releases are published immediately while maintaining code review requirements for version file updates.

## Troubleshooting

### Release didn't trigger automatically
- Verify commits are pushed to `master` branch
- Check that VERSION or CHANGELOG.md weren't the only changed files
- Review GitHub Actions logs for errors

### Wrong version bump detected
- Ensure commit messages follow conventional commit format
- Use manual workflow dispatch with explicit bump type
- Check that commit prefixes are lowercase (`feat:` not `Feat:`)

### Workflow fails with permission error
- Verify GitHub Actions has `contents: write` and `pull-requests: write` permissions
- Check that GITHUB_TOKEN has not expired
- If pushing to master fails, ensure branch protection allows PR-based updates

### Branch is protected
- This is expected! The workflow creates a PR for version updates instead of pushing directly
- Review and merge the automatically created PR to update VERSION and CHANGELOG files
- The release itself is published immediately via git tag and GitHub Release

### Duplicate releases created
- Workflow has concurrency control to prevent this
- Check if multiple workflows were triggered simultaneously

## Best Practices

1. **Write meaningful commit messages**: They become your changelog
2. **Use conventional commits consistently**: Ensures correct version bumps
3. **Review the [Unreleased] section**: Add important notes before release
4. **Test before merging to master**: Releases are automatic on master
5. **Use scopes in commits**: `feat(renderer): add new feature` for better organization

## Release History

All releases are tracked in three places:
1. **CHANGELOG.md**: Human-readable change history
2. **Git tags**: Versioned snapshots in git history
3. **GitHub Releases**: Published releases with notes

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
# 1. Detects 1 feature + 1 fix → minor bump (0.1.0 → 0.2.0)
# 2. Creates tag v0.2.0
# 3. Publishes GitHub release
# 4. Creates PR with VERSION and CHANGELOG updates
# 5. Merge the PR to update version files in master
```

**Note**: The GitHub release is created immediately, while the VERSION and CHANGELOG updates are merged via PR to respect branch protection rules.

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
