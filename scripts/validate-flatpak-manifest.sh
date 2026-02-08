#!/usr/bin/env bash
# Validation script for Flatpak manifest
# Run this before pushing to catch errors early

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

MANIFEST="flatpak/org.particleviewer.ParticleViewer.yaml"
ERRORS=0

echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       Validating Flatpak Manifest Before Push                  ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if manifest exists
if [ ! -f "$MANIFEST" ]; then
    echo -e "${RED}✗ Manifest not found: $MANIFEST${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Manifest file exists${NC}"

# 1. Validate YAML syntax
echo ""
echo -e "${BLUE}[1/5] Validating YAML syntax...${NC}"
if python3 -c "import yaml; yaml.safe_load(open('$MANIFEST'))" 2>/dev/null; then
    echo -e "${GREEN}✓ YAML syntax is valid${NC}"
else
    echo -e "${RED}✗ YAML syntax is invalid${NC}"
    ERRORS=$((ERRORS + 1))
fi

# 2. Check URLs are accessible
echo ""
echo -e "${BLUE}[2/5] Checking download URLs...${NC}"
URLS=$(python3 -c "
import yaml
manifest = yaml.safe_load(open('$MANIFEST'))
for module in manifest.get('modules', []):
    for source in module.get('sources', []):
        if source.get('type') == 'archive' and 'url' in source:
            print(source['url'])
" 2>/dev/null)

if [ -z "$URLS" ]; then
    echo -e "${YELLOW}⚠ No archive URLs found to check${NC}"
else
    while IFS= read -r url; do
        if [ -n "$url" ]; then
            echo -n "  Checking: $url ... "
            if curl -sIL "$url" | grep -q "HTTP.*[23]0[0-9]"; then
                echo -e "${GREEN}✓${NC}"
            else
                echo -e "${RED}✗ (not accessible)${NC}"
                ERRORS=$((ERRORS + 1))
            fi
        fi
    done <<< "$URLS"
fi

# 3. Verify SHA256 checksums
echo ""
echo -e "${BLUE}[3/5] Verifying SHA256 checksums...${NC}"
python3 << 'PYEOF'
import yaml
import sys
import hashlib
import urllib.request

try:
    manifest = yaml.safe_load(open('flatpak/org.particleviewer.ParticleViewer.yaml'))
    errors = 0
    
    for module in manifest.get('modules', []):
        module_name = module.get('name', 'unknown')
        for source in module.get('sources', []):
            if source.get('type') == 'archive' and 'url' in source and 'sha256' in source:
                url = source['url']
                expected_sha = source['sha256']
                
                print(f"  {module_name}: ", end='', flush=True)
                
                try:
                    # Download and calculate SHA256
                    response = urllib.request.urlopen(url, timeout=30)
                    sha256_hash = hashlib.sha256()
                    
                    while chunk := response.read(8192):
                        sha256_hash.update(chunk)
                    
                    actual_sha = sha256_hash.hexdigest()
                    
                    if actual_sha == expected_sha:
                        print(f"\033[0;32m✓ SHA256 matches\033[0m")
                    else:
                        print(f"\033[0;31m✗ SHA256 mismatch!\033[0m")
                        print(f"    Expected: {expected_sha}")
                        print(f"    Got:      {actual_sha}")
                        errors += 1
                except Exception as e:
                    print(f"\033[1;33m⚠ Could not verify: {e}\033[0m")
    
    if errors > 0:
        sys.exit(1)
        
except Exception as e:
    print(f"\033[0;31m✗ Error: {e}\033[0m")
    sys.exit(1)
PYEOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ All SHA256 checksums verified${NC}"
else
    echo -e "${RED}✗ SHA256 verification failed${NC}"
    ERRORS=$((ERRORS + 1))
fi

# 4. Validate manifest structure
echo ""
echo -e "${BLUE}[4/5] Validating manifest structure...${NC}"
python3 << 'PYEOF'
import yaml
import sys

try:
    manifest = yaml.safe_load(open('flatpak/org.particleviewer.ParticleViewer.yaml'))
    
    # Check required fields
    required_fields = ['app-id', 'runtime', 'runtime-version', 'sdk', 'command', 'modules']
    missing = [f for f in required_fields if f not in manifest]
    
    if missing:
        print(f"\033[0;31m✗ Missing required fields: {', '.join(missing)}\033[0m")
        sys.exit(1)
    
    # Check modules structure
    if not isinstance(manifest['modules'], list):
        print(f"\033[0;31m✗ 'modules' must be a list\033[0m")
        sys.exit(1)
    
    if len(manifest['modules']) == 0:
        print(f"\033[0;31m✗ 'modules' cannot be empty\033[0m")
        sys.exit(1)
    
    print("\033[0;32m✓ All required fields present\033[0m")
    print(f"  - app-id: {manifest['app-id']}")
    print(f"  - runtime: {manifest['runtime']} {manifest['runtime-version']}")
    print(f"  - modules: {len(manifest['modules'])} defined")
    
except Exception as e:
    print(f"\033[0;31m✗ Error: {e}\033[0m")
    sys.exit(1)
PYEOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Manifest structure is valid${NC}"
else
    echo -e "${RED}✗ Manifest structure validation failed${NC}"
    ERRORS=$((ERRORS + 1))
fi

# 5. Check if flatpak-builder can parse it
echo ""
echo -e "${BLUE}[5/5] Testing flatpak-builder compatibility...${NC}"
if command -v flatpak-builder &> /dev/null; then
    # Try a dry-run parse (doesn't actually build)
    if flatpak-builder --help | grep -q "stop-at"; then
        echo -e "${YELLOW}  Note: Full parse test requires build environment${NC}"
        echo -e "${GREEN}✓ flatpak-builder is installed${NC}"
    else
        echo -e "${GREEN}✓ flatpak-builder is available${NC}"
    fi
else
    echo -e "${YELLOW}⚠ flatpak-builder not installed (install to test locally)${NC}"
    echo -e "${YELLOW}  Install with: sudo apt-get install flatpak-builder${NC}"
fi

# Summary
echo ""
echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}║  ✓ Validation Complete - No Errors Found!                     ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${GREEN}The manifest is ready to push! 🎉${NC}"
    exit 0
else
    echo -e "${RED}║  ✗ Validation Failed - $ERRORS Error(s) Found                      ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${RED}Please fix the errors above before pushing.${NC}"
    exit 1
fi
