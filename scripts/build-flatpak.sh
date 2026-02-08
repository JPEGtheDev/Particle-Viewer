#!/usr/bin/env bash
# Build script for Particle Viewer Flatpak
# This script builds the Flatpak package for local testing and CI/CD

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building Particle Viewer Flatpak...${NC}"

# Check if flatpak-builder is installed
if ! command -v flatpak-builder &> /dev/null; then
    echo -e "${RED}Error: flatpak-builder is not installed${NC}"
    echo "Install with: sudo apt-get install flatpak-builder"
    exit 1
fi

# Clean previous build artifacts (optional)
if [ "$1" == "--clean" ]; then
    echo -e "${YELLOW}Cleaning previous build artifacts...${NC}"
    rm -rf .flatpak-builder flatpak-build repo *.flatpak
fi

# Build the Flatpak
echo -e "${GREEN}Running flatpak-builder...${NC}"
flatpak-builder --force-clean --repo=repo flatpak-build org.particleviewer.ParticleViewer.yaml

# Create a single-file bundle for distribution
echo -e "${GREEN}Creating Flatpak bundle...${NC}"
flatpak build-bundle repo particle-viewer.flatpak org.particleviewer.ParticleViewer

echo -e "${GREEN}Build complete!${NC}"
echo -e "Flatpak bundle created: ${YELLOW}particle-viewer.flatpak${NC}"
echo ""
echo "To install locally:"
echo "  flatpak install --user particle-viewer.flatpak"
echo ""
echo "To run:"
echo "  flatpak run org.particleviewer.ParticleViewer"
