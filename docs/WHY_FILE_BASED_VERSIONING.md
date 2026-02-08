# Why File-Based Versioning Was Originally Chosen (PR #40)

## Historical Context

When the automated release workflow was first implemented in PR #40, a file-based versioning system using a `VERSION` file was chosen as the single source of truth for version information.

## Rationale for File-Based Approach

### 1. **Simplicity and Clarity**
- A plain text `VERSION` file is extremely simple to understand
- Easy to read programmatically: `cat VERSION` or `file(READ ...)`
- No parsing complexity or edge cases
- Version is visible in the file system, easy to inspect

### 2. **Independence from Git**
- Works without git being installed or available
- Source archives (tarballs, zip files) distributed without git history can still be built
- No dependency on git commands at build time
- Users building from source without cloning the repo can see the version

### 3. **Cross-Platform Compatibility**
- Works identically on all platforms (Windows, macOS, Linux)
- No platform-specific git behavior to handle
- No concerns about git line endings or other git-specific issues

### 4. **Standard Practice**
- Many projects use a dedicated version file (VERSION, version.txt, etc.)
- Familiar pattern for contributors
- Well-established convention in open-source projects

### 5. **Zero Manual Intervention Goal**
- The PR #40 goal was "zero-manual release pipeline"
- At the time, having a file that the workflow updates was seen as part of the automation
- The version number is computed, stored, and tagged all automatically

## The Critical Flaw: Branch Protection Conflict

However, the file-based approach had a **significant drawback** that became apparent:

### The Problem
The workflow needed to:
1. Calculate new version based on commits
2. **Update the VERSION file with the new version**
3. **Commit the VERSION file back to master**
4. Create a git tag
5. Create GitHub release

**Step 3 (committing back to master)** conflicts with branch protection rules that require:
- All changes go through pull requests
- Status checks must pass before merging

### The Conflict
- The automated release workflow tried to push directly to master
- Branch protection rules rejected this push
- Error: "GH006: Protected branch update failed for refs/heads/master"
- Error: "Changes must be made through a pull request"

### Attempted Workaround
An initial attempt was made to work around this by having the workflow create a pull request for the version bump. However, this defeated the purpose of automation by requiring manual PR review and merge.

## Why Git Tag-Based Versioning Is Better

The git tag-based approach solves the branch protection issue:

### How It Works
1. Calculate new version based on commits
2. Create git tag with version (e.g., `v0.2.0`)
3. Push the tag (tags are not subject to branch protection)
4. Create GitHub release
5. **No commit to master needed!**

### Benefits
- ✅ **No commit-back required** - workflow only creates tags
- ✅ **Works with branch protection** - tags bypass protection rules
- ✅ **Cleaner git history** - no "bump version" commits
- ✅ **Git tags are the natural place for versions**
- ✅ **CMakeLists.txt derives version at build time** - still automatic

### Trade-off
- ⚠️ **Requires git at build time** - `execute_process(COMMAND git describe ...)`
- ⚠️ **Source archives need tags** - or fallback to 0.0.0

However, this trade-off is acceptable because:
- Git is nearly always available in development environments
- CI/CD systems always have git
- The fallback version (0.0.0) ensures builds don't fail
- The benefits far outweigh this minor limitation

## Conclusion

File-based versioning with a VERSION file was a reasonable choice initially, especially for:
- Simplicity
- Cross-platform compatibility
- Building without git

However, the **critical flaw of requiring commits back to a protected branch** made it impractical in modern workflows with branch protection enabled.

Git tag-based versioning is the better solution because:
- Tags are the natural place for version information
- No branch protection conflicts
- Cleaner workflow
- Still fully automated

The migration from file-based to tag-based versioning maintains the "zero-manual" goal while eliminating the branch protection conflicts.
