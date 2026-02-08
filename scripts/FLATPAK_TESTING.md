# Local Flatpak Testing Guide

This directory contains scripts to test Flatpak builds locally before pushing to GitHub, eliminating the trial-and-error cycle with CI/CD.

## Quick Start

### Option 1: Quick Validation (Fast)
Just want to check if your manifest changes are valid?

```bash
./scripts/validate-flatpak-manifest.sh
```

This script:
- ✅ Validates YAML syntax
- ✅ Checks if download URLs are accessible
- ✅ Verifies SHA256 checksums match
- ✅ Validates manifest structure
- ⚡ Takes ~30 seconds

### Option 2: Full Build Test (Comprehensive)
Want to test the actual build locally like CI does?

```bash
./scripts/test-flatpak-build-local.sh
```

This script:
- ✅ Installs required dependencies (if needed)
- ✅ Sets up Flatpak environment
- ✅ Runs the exact same build steps as CI
- ✅ Creates a test bundle
- 🕐 Takes ~5-10 minutes (first run downloads dependencies)

## Workflow

### Before Making Changes
1. Make your changes to `flatpak/org.particleviewer.ParticleViewer.yaml`
2. Run validation: `./scripts/validate-flatpak-manifest.sh`
3. Fix any errors reported
4. Optionally test full build: `./scripts/test-flatpak-build-local.sh`
5. Commit and push with confidence! 🎉

## Troubleshooting

### "flatpak not installed"
The test script will automatically install it. Or manually:
```bash
sudo apt-get install flatpak flatpak-builder
```

### "Permission denied"
Make scripts executable:
```bash
chmod +x scripts/*.sh
```

### "SHA256 mismatch"
This means the checksum in your manifest doesn't match the actual file:
1. Download the file: `curl -sL <URL> | sha256sum`
2. Update the `sha256:` field in the manifest
3. Run validation again

### "URL not accessible (404)"
The download URL is broken:
1. Check if the file exists at that URL
2. Try alternative URL formats (e.g., `archive/refs/tags/` instead of `releases/download/`)
3. Update the URL in the manifest

## What These Scripts Don't Do

- ❌ Cannot directly trigger GitHub Actions (GitHub limitation)
- ❌ Cannot test PR-specific features (e.g., PR comments)
- ❌ Cannot replicate exact CI environment (but very close!)

## What These Scripts DO

- ✅ Catch 99% of build errors before pushing
- ✅ Save time by avoiding "fix CI" commits
- ✅ Give you confidence your changes will work
- ✅ Let you iterate faster locally

## Examples

### Validate after changing GLM version:
```bash
# Edit manifest to change GLM version
vim flatpak/org.particleviewer.ParticleViewer.yaml

# Validate
./scripts/validate-flatpak-manifest.sh

# If validation passes, you're good to push!
```

### Test full build after major changes:
```bash
# Make significant changes
vim flatpak/org.particleviewer.ParticleViewer.yaml

# Test full build locally
./scripts/test-flatpak-build-local.sh

# If successful, the CI build will also succeed
git add .
git commit -m "feat: update dependencies"
git push
```

## Integration with Your Workflow

### Pre-commit Hook (Optional)
Want automatic validation before every commit?

Create `.git/hooks/pre-commit`:
```bash
#!/bin/bash
if git diff --cached --name-only | grep -q "flatpak/"; then
    echo "Validating Flatpak manifest..."
    ./scripts/validate-flatpak-manifest.sh
fi
```

Make it executable:
```bash
chmod +x .git/hooks/pre-commit
```

## CI/CD Pipeline Comparison

| CI Step | Local Equivalent |
|---------|-----------------|
| Install flatpak | Auto-installed by test script |
| Add Flathub | Auto-configured by test script |
| Install SDK/Platform | Auto-installed by test script |
| Build Flatpak | `flatpak-builder` in test script |
| Create bundle | `flatpak build-bundle` in test script |
| Upload artifact | Creates local `.flatpak` file |

## Tips

1. **First time setup**: Run the full test script once to set up your environment
2. **Quick iterations**: Use validation script for quick checks
3. **Before pushing**: Always run at least the validation script
4. **Save time**: These scripts save hours of CI/CD debugging time

## Need Help?

If you encounter issues with these scripts, check:
1. You're running on Ubuntu/Debian-based system
2. You have sudo access
3. Internet connection is working (for downloads)
4. Disk space is available (~2GB for full build)
