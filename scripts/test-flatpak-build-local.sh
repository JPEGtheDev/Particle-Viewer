#!/usr/bin/env bash
# Local Flatpak build test script
# Simulates the CI/CD workflow steps locally

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

MANIFEST="flatpak/org.particleviewer.ParticleViewer.yaml"

echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       Testing Flatpak Build Locally (CI Simulation)           ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if we're running as root (not recommended)
if [ "$EUID" -eq 0 ]; then
    echo -e "${RED}✗ Don't run this script as root${NC}"
    echo -e "${YELLOW}  Run without sudo, script will ask for password when needed${NC}"
    exit 1
fi

# Step 1: Check dependencies
echo -e "${BLUE}[Step 1/6] Checking dependencies...${NC}"

if ! command -v flatpak &> /dev/null; then
    echo -e "${YELLOW}⚠ flatpak not installed${NC}"
    echo -e "${YELLOW}Installing flatpak...${NC}"
    sudo apt-get update -qq
    sudo apt-get install -y flatpak
fi
echo -e "${GREEN}✓ flatpak is installed${NC}"

if ! command -v flatpak-builder &> /dev/null; then
    echo -e "${YELLOW}⚠ flatpak-builder not installed${NC}"
    echo -e "${YELLOW}Installing flatpak-builder...${NC}"
    sudo apt-get install -y flatpak-builder
fi
echo -e "${GREEN}✓ flatpak-builder is installed${NC}"

# Step 2: Add Flathub repository
echo ""
echo -e "${BLUE}[Step 2/6] Configuring Flathub repository...${NC}"
if ! flatpak remote-list | grep -q "flathub"; then
    echo -e "${YELLOW}Adding Flathub repository...${NC}"
    sudo flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
fi
echo -e "${GREEN}✓ Flathub repository configured${NC}"

# Step 3: Install runtime and SDK
echo ""
echo -e "${BLUE}[Step 3/6] Installing Freedesktop SDK and Platform...${NC}"
RUNTIME_VERSION="24.08"

if ! flatpak list --runtime | grep -q "org.freedesktop.Platform.*$RUNTIME_VERSION"; then
    echo -e "${YELLOW}Installing org.freedesktop.Platform//$RUNTIME_VERSION...${NC}"
    sudo flatpak install -y flathub org.freedesktop.Platform//$RUNTIME_VERSION
else
    echo -e "${GREEN}✓ org.freedesktop.Platform//$RUNTIME_VERSION already installed${NC}"
fi

if ! flatpak list --runtime | grep -q "org.freedesktop.Sdk.*$RUNTIME_VERSION"; then
    echo -e "${YELLOW}Installing org.freedesktop.Sdk//$RUNTIME_VERSION...${NC}"
    sudo flatpak install -y flathub org.freedesktop.Sdk//$RUNTIME_VERSION
else
    echo -e "${GREEN}✓ org.freedesktop.Sdk//$RUNTIME_VERSION already installed${NC}"
fi

# Step 4: Clean previous build artifacts
echo ""
echo -e "${BLUE}[Step 4/6] Cleaning previous build artifacts...${NC}"
rm -rf .flatpak-builder build-dir repo *.flatpak 2>/dev/null || true
echo -e "${GREEN}✓ Build directory cleaned${NC}"

# Step 5: Build Flatpak
echo ""
echo -e "${BLUE}[Step 5/6] Building Flatpak (this may take a while)...${NC}"
echo -e "${YELLOW}Note: This will download and build dependencies${NC}"
echo ""

if flatpak-builder --force-clean --repo=repo build-dir "$MANIFEST"; then
    echo ""
    echo -e "${GREEN}✓ Flatpak build successful!${NC}"
else
    echo ""
    echo -e "${RED}✗ Flatpak build failed${NC}"
    echo -e "${YELLOW}Check the error messages above${NC}"
    exit 1
fi

# Step 6: Create bundle
echo ""
echo -e "${BLUE}[Step 6/6] Creating Flatpak bundle...${NC}"
BUNDLE_NAME="particle-viewer-test.flatpak"

if flatpak build-bundle repo "$BUNDLE_NAME" org.particleviewer.ParticleViewer; then
    echo -e "${GREEN}✓ Bundle created: $BUNDLE_NAME${NC}"
    
    # Show bundle info
    SIZE=$(du -h "$BUNDLE_NAME" | cut -f1)
    echo ""
    echo -e "${GREEN}Bundle Information:${NC}"
    echo -e "  Size: $SIZE"
    echo -e "  Path: $(pwd)/$BUNDLE_NAME"
else
    echo -e "${RED}✗ Failed to create bundle${NC}"
    exit 1
fi

# Summary
echo ""
echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║  ✓ Local Build Test Complete - Success!                       ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${GREEN}Your changes are ready to push! The CI workflow should succeed.${NC}"
echo ""
echo -e "${YELLOW}To test the built Flatpak:${NC}"
echo -e "  1. Install: flatpak install --user $BUNDLE_NAME"
echo -e "  2. Run:     flatpak run org.particleviewer.ParticleViewer"
echo ""
echo -e "${YELLOW}To clean up build artifacts:${NC}"
echo -e "  rm -rf .flatpak-builder build-dir repo *.flatpak"
echo ""
