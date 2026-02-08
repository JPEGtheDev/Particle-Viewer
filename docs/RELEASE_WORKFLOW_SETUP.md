# Release Workflow Setup Guide

This document explains how to configure the repository to enable the automated release workflow to function properly with branch protection rules.

## Problem

The Automated Release workflow attempts to commit VERSION and CHANGELOG updates directly to the `master` branch. However, if branch protection rules are enabled that require pull requests, the workflow will fail with:

```
remote: error: GH006: Protected branch update failed for refs/heads/master.
remote: - Changes must be made through a pull request.
```

## Solution

To maintain both **full automation** and **branch protection**, you need to configure branch protection to allow the GitHub Actions bot to bypass the rules for automated releases.

### Option 1: Allow GitHub Actions Bot to Bypass (Recommended)

This maintains full automation while keeping branch protection for human contributors.

1. Go to **Settings** → **Branches** → **Branch protection rules** for `master`
2. Scroll down to **"Allow specified actors to bypass required pull requests"** section
3. Add `github-actions[bot]` or `github-actions` to the bypass list
4. Alternatively, under **"Do not allow bypassing the above settings"**, uncheck this option
5. Save changes

This allows the automated release workflow to push directly to master while requiring PRs for all other changes.

### Option 2: Use a Personal Access Token (Alternative)

If you prefer not to modify branch protection rules:

1. Create a Personal Access Token (PAT) with `repo` scope
   - Go to **Settings** → **Developer settings** → **Personal access tokens** → **Tokens (classic)**
   - Generate new token with `repo` permissions
   - Copy the token

2. Add the token as a repository secret:
   - Go to repository **Settings** → **Secrets and variables** → **Actions**
   - Create new secret named `RELEASE_PAT`
   - Paste the token value

3. Update the workflow to use the PAT:
   ```yaml
   - name: Checkout repository
     uses: actions/checkout@v4
     with:
       fetch-depth: 0
       token: ${{ secrets.RELEASE_PAT }}  # Use PAT instead of GITHUB_TOKEN
   ```

4. Also configure git to use the PAT for pushes:
   ```yaml
   - name: Configure Git
     run: |
       git config user.name "github-actions[bot]"
       git config user.email "github-actions[bot]@users.noreply.github.com"
       git remote set-url origin https://x-access-token:${{ secrets.RELEASE_PAT }}@github.com/${{ github.repository }}
   ```

**Note:** Option 2 requires manual token management and rotation, making it less ideal for long-term maintenance.

### Option 3: Disable Branch Protection for Automated Commits

If your repository has minimal required status checks:

1. Go to **Settings** → **Branches** → **Branch protection rules** for `master`
2. Under **"Require status checks to pass before merging"**, ensure the release workflow is not listed as a required check
3. Add `[skip ci]` to the commit message (already included in workflow)
4. This prevents circular dependencies where the release workflow triggers itself

## Verification

After implementing Option 1 (recommended):

1. Make a commit to master with conventional commit format:
   ```bash
   git commit -m "feat: test automated release"
   git push origin master
   ```

2. Monitor the **Actions** tab to see the release workflow run
3. Verify that:
   - VERSION and CHANGELOG are updated
   - Git tag is created (e.g., `v0.2.0`)
   - GitHub Release is published
   - No errors about protected branches

## Why Not Use Pull Requests?

Creating a pull request for version bumps defeats the purpose of automation:
- Requires manual review and merging
- Delays release publication
- Adds unnecessary overhead for simple version file updates
- Breaks the "zero-manual release pipeline" design

The version file updates are automatically generated from commit history and should be trusted as part of the automated process.

## Recommended Configuration

For optimal security and automation:

1. ✅ Enable branch protection on `master`
2. ✅ Require pull requests for code changes
3. ✅ Require status checks (build, tests)
4. ✅ Allow `github-actions[bot]` to bypass for automated releases
5. ✅ Use conventional commits for semantic versioning
6. ✅ Review releases in GitHub Releases page after automation

This configuration maintains code quality standards while enabling full release automation.

## Troubleshooting

### Workflow still fails with "protected branch" error
- Verify `github-actions[bot]` is in the bypass list
- Check that you saved the branch protection rule changes
- Ensure the workflow is using `GITHUB_TOKEN` (default) or a PAT with sufficient permissions

### Tags created but GitHub Release fails
- Ensure `GITHUB_TOKEN` has `contents: write` permission
- Check for API rate limiting
- Verify the tag was pushed successfully before release creation

## Additional Resources

- [GitHub Branch Protection Documentation](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/defining-the-mergeability-of-pull-requests/about-protected-branches)
- [GitHub Actions Permissions](https://docs.github.com/en/actions/security-guides/automatic-token-authentication#permissions-for-the-github_token)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)
