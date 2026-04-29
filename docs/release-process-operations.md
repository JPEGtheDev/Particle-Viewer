---
title: "Release Operations and Troubleshooting"
description: "Zero-manual release requirements, workflow configuration, troubleshooting guide, and best practices for Particle-Viewer releases."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, release, operations, troubleshooting, workflow]
related:
  - "RELEASE_PROCESS.md"
  - "release-process-automation.md"
---

# Release Operations and Troubleshooting

## Zero-Manual Requirements

**MUST DO:**
- Use conventional commit messages for all commits
- Push to master to trigger automatic releases
- Let the workflow determine version bumps automatically
- Review releases in GitHub Releases after automation

**MUST NOT DO:**
- Manually create git tags
- Manually edit the version in `CMakeLists.txt`

## Workflow Configuration

The release workflow is defined in `.github/workflows/release.yml`:

- **Concurrency control** — prevents simultaneous releases
- **Permissions** — `contents: write` for creating releases
- **Skip CI** — version bump commits include `[skip ci]` to prevent recursive workflows
- **Full history** — fetches complete git history for changelog generation

## Troubleshooting

### Release did not trigger automatically

- Verify commits are pushed to `master`
- Check if only non-source files changed (tests, docs, CI) — release is skipped by design
- Review GitHub Actions logs for skip or error messages

### Wrong version bump detected

- Verify commit messages follow conventional commit format exactly
- Confirm prefixes are lowercase (`feat:` not `Feat:`)
- Use manual workflow dispatch with an explicit bump type

### Workflow fails with permission error

- Verify GitHub Actions has `contents: write` permission
- Check that `GITHUB_TOKEN` has not expired

### Duplicate releases created

- Concurrency control is in place; check if multiple workflows were triggered simultaneously

## Best Practices

1. Write meaningful commit messages — they become release notes
2. Use conventional commits consistently
3. Test before merging to master — releases trigger automatically
4. Use scopes for better organization: `feat(renderer): add feature`

## Release History

All releases are tracked in:
- **Git tags**: versioned snapshots in git history
- **GitHub Releases**: published releases with generated release notes

## Related

- [Release Process](RELEASE_PROCESS.md) — parent ToC stub
- [release-process-automation.md](release-process-automation.md) — trigger logic and release steps
- [Conventional Commits](CONVENTIONAL_COMMITS.md) — commit format reference
