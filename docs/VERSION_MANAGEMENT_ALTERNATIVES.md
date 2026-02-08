# Version Management Alternatives for Particle-Viewer

## Current Approach: Dedicated VERSION File

The project currently uses a dedicated `VERSION` file as the single source of truth:

```
VERSION file (0.1.0)
    ↓
CMakeLists.txt reads it → project(Particle-Viewer VERSION ${PROJECT_VERSION})
    ↓
Release workflow updates it → commits back to repo
    ↓
Git tag created (v0.1.0)
```

### Current Pros
- ✅ Simple to understand
- ✅ Easy to read programmatically
- ✅ Works across all build systems
- ✅ Human-readable

### Current Cons
- ❌ Requires committing VERSION file back after release
- ❌ Can cause merge conflicts
- ❌ Creates "version bump" commits
- ❌ Potential circular dependency with CI/CD

---

## Alternative 1: Git Tags as Single Source of Truth ⭐ **Recommended**

Use git tags exclusively, derive version from `git describe` at build time.

### How It Works

```bash
# Get version from most recent tag
VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
```

**CMakeLists.txt:**
```cmake
# Get version from git
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Fallback if no tags exist
if(NOT GIT_VERSION)
    set(GIT_VERSION "0.0.0-dev")
endif()

# Parse version (remove 'v' prefix if present)
string(REGEX REPLACE "^v" "" PROJECT_VERSION ${GIT_VERSION})

project(Particle-Viewer VERSION ${PROJECT_VERSION})
```

**Release Workflow:**
```yaml
- name: Get current version from tags
  id: current_version
  run: |
    CURRENT_VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
    # Remove 'v' prefix if present
    CURRENT_VERSION=${CURRENT_VERSION#v}
    echo "version=$CURRENT_VERSION" >> $GITHUB_OUTPUT
    echo "Current version: $CURRENT_VERSION"

# ... calculate new version ...

# Create tag (no VERSION file to update!)
- name: Create Git tag
  run: |
    TAG="v${{ steps.new_version.outputs.version }}"
    git tag -a "$TAG" -m "Release $TAG"
    git push origin "$TAG"

# No need to commit VERSION file back!
```

### Pros
- ✅ **No VERSION file to maintain** - eliminates file altogether
- ✅ **No commit-back required** - workflow just creates tags
- ✅ **No merge conflicts** on VERSION file
- ✅ **Cleaner git history** - no "bump version" commits
- ✅ **Git is authoritative** - version lives where it belongs
- ✅ **Works with branch protection** - no pushes to master needed

### Cons
- ❌ Requires git at build time (usually available)
- ❌ Slightly more complex CMake
- ❌ Must handle "no tags yet" case

### Best For
- Projects with automated CI/CD
- Teams that prefer minimal file changes
- Avoiding branch protection issues

---

## Alternative 2: Git Describe with Build Metadata

Use `git describe` to generate rich version strings with commit info.

### How It Works

```bash
# Get detailed version info
VERSION=$(git describe --tags --long --always --dirty)
# Example output: v0.1.0-5-g3e2a1b4-dirty
# Means: 5 commits after v0.1.0, commit hash g3e2a1b4, with uncommitted changes
```

**CMakeLists.txt:**
```cmake
execute_process(
    COMMAND git describe --tags --long --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Parse into components
if(GIT_DESCRIBE MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
else()
    set(VERSION_MAJOR 0)
    set(VERSION_MINOR 0)
    set(VERSION_PATCH 0)
endif()

project(Particle-Viewer VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Store full describe output for --version flag
set(VERSION_FULL ${GIT_DESCRIBE})
configure_file(version.h.in version.h)
```

**version.h.in:**
```cpp
#define VERSION_MAJOR @VERSION_MAJOR@
#define VERSION_MINOR @VERSION_MINOR@
#define VERSION_PATCH @VERSION_PATCH@
#define VERSION_STRING "@VERSION_FULL@"
```

### Pros
- ✅ Rich version information (commit count, hash, dirty status)
- ✅ Unique version for every commit
- ✅ Great for development builds
- ✅ No VERSION file needed

### Cons
- ❌ Version string can be long/ugly
- ❌ Requires git at build time
- ❌ More complex to parse

### Best For
- Development builds
- Debugging issues in the field
- Knowing exactly what commit was built

---

## Alternative 3: CMakeLists.txt as Version Source

Hardcode version directly in CMakeLists.txt.

### How It Works

**CMakeLists.txt:**
```cmake
project(Particle-Viewer VERSION 0.1.0)
# That's it - version is right here
```

**Release Workflow:**
```yaml
- name: Read current version from CMakeLists.txt
  id: current_version
  run: |
    VERSION=$(grep "project.*VERSION" CMakeLists.txt | sed -E 's/.*VERSION ([0-9.]+).*/\1/')
    echo "version=$VERSION" >> $GITHUB_OUTPUT

- name: Update version in CMakeLists.txt
  run: |
    sed -i "s/VERSION [0-9.]\+/VERSION ${{ steps.new_version.outputs.version }}/" CMakeLists.txt
    
- name: Commit version bump
  run: |
    git add CMakeLists.txt CHANGELOG.md
    git commit -m "chore(release): bump version to ${{ steps.new_version.outputs.version }}"
    git push origin master
```

### Pros
- ✅ One less file to maintain
- ✅ Version is where it's used
- ✅ Standard CMake practice

### Cons
- ❌ Still requires commit-back
- ❌ Parsing CMakeLists.txt is fragile
- ❌ Mixing concerns (build config + version)

### Best For
- Simple projects
- When you're already editing CMakeLists.txt often

---

## Alternative 4: Generated Version Header

Generate version at build time, no source control of version.

### How It Works

**CMakeLists.txt:**
```cmake
# Get version from environment or git
if(DEFINED ENV{CI_VERSION})
    set(PROJECT_VERSION $ENV{CI_VERSION})
else()
    execute_process(
        COMMAND git describe --tags --abbrev=0
        OUTPUT_VARIABLE PROJECT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT PROJECT_VERSION)
        set(PROJECT_VERSION "0.0.0-dev")
    endif()
endif()

project(Particle-Viewer VERSION ${PROJECT_VERSION})

# Generate header
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
    @ONLY
)

target_include_directories(Viewer PRIVATE ${CMAKE_BINARY_DIR}/generated)
```

**cmake/version.h.in:**
```cpp
#ifndef VERSION_H
#define VERSION_H

#define PROJECT_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define PROJECT_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define PROJECT_VERSION_PATCH @PROJECT_VERSION_PATCH@
#define PROJECT_VERSION_STRING "@PROJECT_VERSION@"

#endif // VERSION_H
```

**Usage in code:**
```cpp
#include "version.h"
std::cout << "Particle-Viewer v" << PROJECT_VERSION_STRING << std::endl;
```

### Pros
- ✅ Version available in C++ code
- ✅ Flexible source (env var, git, file)
- ✅ Generated file not in source control

### Cons
- ❌ Requires build to see version
- ❌ More CMake complexity

### Best For
- Displaying version in application UI
- Logging version for debugging

---

## Alternative 5: Environment Variable Driven

CI/CD sets version, no file needed in repo.

### How It Works

**Release Workflow:**
```yaml
- name: Calculate version
  id: version
  run: |
    # Get from tags
    OLD_VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
    # ... calculate new version ...
    echo "VERSION=$NEW_VERSION" >> $GITHUB_ENV
    echo "version=$NEW_VERSION" >> $GITHUB_OUTPUT

- name: Build with version
  run: |
    cmake -DPROJECT_VERSION=${{ steps.version.outputs.version }} .
    cmake --build .
    
- name: Create tag
  run: git tag v${{ steps.version.outputs.version }}
```

**CMakeLists.txt:**
```cmake
# Accept version from command line or derive from git
if(NOT DEFINED PROJECT_VERSION)
    execute_process(
        COMMAND git describe --tags --abbrev=0
        OUTPUT_VARIABLE PROJECT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT PROJECT_VERSION)
    set(PROJECT_VERSION "0.0.0")
endif()

project(Particle-Viewer VERSION ${PROJECT_VERSION})
```

### Pros
- ✅ No version file in repo
- ✅ CI has full control
- ✅ Can override easily

### Cons
- ❌ Version not visible in repo
- ❌ Local builds need special handling

### Best For
- Pure CI/CD environments
- Multiple build configurations

---

## Alternative 6: Hybrid: Git Tags + Optional VERSION File

Use git tags as primary source, fall back to VERSION file if git not available.

### How It Works

**CMakeLists.txt:**
```cmake
# Try git first
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Fall back to VERSION file
if(NOT GIT_VERSION AND EXISTS "${CMAKE_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_SOURCE_DIR}/VERSION" FILE_VERSION)
    string(STRIP "${FILE_VERSION}" FILE_VERSION)
    set(PROJECT_VERSION ${FILE_VERSION})
else()
    string(REGEX REPLACE "^v" "" PROJECT_VERSION ${GIT_VERSION})
endif()

# Ultimate fallback
if(NOT PROJECT_VERSION)
    set(PROJECT_VERSION "0.0.0")
endif()

project(Particle-Viewer VERSION ${PROJECT_VERSION})
```

### Pros
- ✅ Works with or without git
- ✅ Can distribute source without git history
- ✅ Graceful degradation

### Cons
- ❌ Two sources to maintain
- ❌ Can get out of sync

### Best For
- Projects distributed as source archives
- Maximum compatibility

---

## Comparison Matrix

| Approach | No File | No Commit | Works Offline | Simple | Rich Info | Recommended |
|----------|---------|-----------|---------------|--------|-----------|-------------|
| **Current: VERSION file** | ❌ | ❌ | ✅ | ✅ | ❌ | ⭐⭐⭐ |
| **Git tags only** | ✅ | ✅ | ❌ | ✅ | ❌ | ⭐⭐⭐⭐⭐ |
| **Git describe** | ✅ | ✅ | ❌ | ⭐ | ✅ | ⭐⭐⭐⭐ |
| **CMakeLists.txt** | ⭐ | ❌ | ✅ | ✅ | ❌ | ⭐⭐ |
| **Generated header** | ✅ | ✅ | ❌ | ⭐ | ✅ | ⭐⭐⭐⭐ |
| **Environment var** | ✅ | ✅ | ❌ | ⭐⭐ | ❌ | ⭐⭐⭐ |
| **Hybrid** | ⭐ | ✅ | ✅ | ⭐ | ❌ | ⭐⭐⭐⭐ |

---

## Recommendation for Particle-Viewer

### Best Approach: **Git Tags Only** (Alternative 1)

**Why:**
1. Eliminates the VERSION file entirely
2. No commit-back to master needed (solves branch protection issue!)
3. Cleaner git history
4. Git tags are the natural place for versions
5. Simpler workflow

### Implementation Steps

1. **Update CMakeLists.txt:**
   ```cmake
   execute_process(
       COMMAND git describe --tags --abbrev=0
       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
       OUTPUT_VARIABLE GIT_VERSION
       OUTPUT_STRIP_TRAILING_WHITESPACE
       ERROR_QUIET
   )
   string(REGEX REPLACE "^v" "" PROJECT_VERSION ${GIT_VERSION})
   if(NOT PROJECT_VERSION)
       set(PROJECT_VERSION "0.0.0")
   endif()
   project(Particle-Viewer VERSION ${PROJECT_VERSION})
   ```

2. **Update Release Workflow:**
   - Remove "Update VERSION file" step
   - Remove "Commit version bump" step
   - Read version from tags instead of file
   - Only push tags, not commits

3. **Update Documentation:**
   - Remove references to VERSION file
   - Document that version comes from git tags
   - Update contributor guide

4. **Migration:**
   - Tag current state: `git tag v0.1.0`
   - Delete VERSION file
   - Test build works
   - Update CI/CD

### Fallback: **Hybrid Approach** (Alternative 6)

If you need to support building without git (e.g., from source archives):
- Keep VERSION file as fallback
- But make git tags authoritative
- Workflow only creates tags, doesn't update VERSION
- VERSION file manually maintained for rare source distribution

---

## Testing Checklist

Before switching approaches:

- [ ] Test building with version from new source
- [ ] Test building without git (if applicable)
- [ ] Test CI/CD workflow creates proper tags
- [ ] Test release notes generation
- [ ] Test CHANGELOG.md updates
- [ ] Test that old tags still work
- [ ] Test local builds
- [ ] Update all documentation

---

## Example Migration PR

See example implementation in branch `feature/git-tags-version` for:
- Updated CMakeLists.txt
- Updated release workflow
- Removal of VERSION file
- Updated documentation
- Migration guide

---

## References

- Git describe documentation: https://git-scm.com/docs/git-describe
- CMake project() command: https://cmake.org/cmake/help/latest/command/project.html
- Semantic Versioning: https://semver.org/
- Conventional Commits: https://www.conventionalcommits.org/
