# Flatpak Implementation Verification Guide

This document provides steps to verify the Flatpak implementation for Particle Viewer.

## PR Workflow Testing

### When to Test
The PR workflow triggers automatically when:
- A pull request is opened or updated
- Changes are made to relevant files (src/, CMakeLists.txt, Flatpak files, etc.)

### Expected Behavior
1. **Workflow Triggers**: Check the "Actions" tab to see if `Build Flatpak (PR)` workflow starts
2. **Build Success**: Workflow should complete successfully (green checkmark)
3. **Artifact Upload**: A Flatpak artifact should be uploaded with name `particle-viewer-flatpak-pr-<NUMBER>`
4. **PR Comment**: A bot comment should appear with download and installation instructions

### Manual Verification
To test the PR build:
```bash
# Download the artifact from GitHub Actions
# Extract the zip file
# Install the Flatpak
flatpak install --user particle-viewer-pr-<NUMBER>.flatpak

# Run the application
flatpak run org.particleviewer.ParticleViewer

# Verify functionality
# - Application launches
# - Can open/load files
# - 3D rendering works
# - No dependency errors
```

## Release Workflow Testing

### When to Test
The release workflow runs:
- On push to master branch (if version changes are detected)
- On manual workflow dispatch with version bump selection

### Expected Behavior
1. **Version Detection**: Workflow detects version bump from commit messages
2. **Release Created**: GitHub release is created with tag `v<VERSION>`
3. **Flatpak Build Job**: Separate job builds Flatpak in container
4. **Asset Upload**: Flatpak bundle is uploaded to the release

### Manual Verification
After a release:
1. Check [Releases page](https://github.com/JPEGtheDev/Particle-Viewer/releases)
2. Verify the latest release has a `.flatpak` asset attached
3. Download and test the release Flatpak:
   ```bash
   flatpak install --user particle-viewer-<VERSION>.flatpak
   flatpak run org.particleviewer.ParticleViewer
   ```

## Local Build Testing

### Prerequisites
```bash
# Install required packages
sudo apt install flatpak flatpak-builder

# Add Flathub repository
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install runtime and SDK
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

### Build and Test
```bash
# Navigate to repository root
cd /path/to/Particle-Viewer

# Run build script
./scripts/build-flatpak.sh

# Expected output:
# - Build completes without errors
# - particle-viewer.flatpak is created in root directory

# Install and test
flatpak install --user particle-viewer.flatpak
flatpak run org.particleviewer.ParticleViewer
```

### Clean Build
```bash
# Clean and rebuild
./scripts/build-flatpak.sh --clean
```

## Checklist for PR Review

- [ ] All YAML workflows are syntactically valid
- [ ] Flatpak manifest includes all dependencies
- [ ] Desktop entry file is properly formatted
- [ ] AppStream metadata is valid
- [ ] Build script is executable and functional
- [ ] .gitignore excludes Flatpak build artifacts
- [ ] PR workflow triggers on correct paths
- [ ] Release workflow separates jobs correctly
- [ ] Documentation is complete (FLATPAK.md, README.md)

## Common Issues and Solutions

### Issue: Flatpak Builder Not Found
**Solution:**
```bash
sudo apt install flatpak-builder
```

### Issue: Runtime Not Found
**Solution:**
```bash
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
```

### Issue: Permission Denied on Build Script
**Solution:**
```bash
chmod +x scripts/build-flatpak.sh
```

### Issue: Workflow Fails on Missing Dependencies
**Solution:**
- Check the Flatpak manifest for correct dependency URLs and SHA256 hashes
- Verify runtime version (24.08) is available
- Check that container image is correct in workflow

### Issue: Application Won't Launch
**Solution:**
```bash
# Check logs
flatpak run --verbose org.particleviewer.ParticleViewer

# Verify installation
flatpak list | grep ParticleViewer
```

## Success Criteria

✅ **PR Workflow**
- Builds Flatpak successfully on PR creation
- Uploads artifact for testing
- Posts helpful comment with instructions

✅ **Release Workflow**
- Creates release with correct version
- Builds Flatpak in separate job
- Attaches Flatpak to release

✅ **Local Build**
- Build script works without errors
- Generated Flatpak can be installed
- Application runs correctly in sandbox

✅ **Functionality**
- Application launches from Flatpak
- File loading works (with correct permissions)
- All features work in sandboxed environment
- No missing dependencies

✅ **Documentation**
- FLATPAK.md is comprehensive
- README.md mentions Flatpak distribution
- Build instructions are clear

## Next Steps

After verification:
1. Merge PR to enable workflows
2. Test PR workflow on a subsequent PR
3. Trigger a release to test release workflow
4. Verify the application icon displays correctly in your desktop environment
5. Consider submitting to Flathub (optional)
