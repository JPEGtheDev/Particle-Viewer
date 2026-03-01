#!/usr/bin/env bash
# Helper for Particle-Viewer users running on NVIDIA hardware inside
# a Flatpak sandbox.  The viewer prints a warning when it falls back to
# llvmpipe because the host has no matching GL extension.  This script
# automates the detection of the driver version and installs the
# corresponding Flatpak module from Flathub.

set -euo pipefail

show_usage() {
    cat <<'EOF'
Usage: install-nvidia-gl-extension.sh

Detect the installed NVIDIA driver version and install the
org.freedesktop.Platform.GL.nvidia-<version>//1.4 Flatpak
runtime extension from Flathub.  Exit code 0 means either the
correct extension was already installed or the installation
completed successfully.

EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
    show_usage
    exit 0
fi

# Determine driver version first so we always print it.
# Prefer nvidia-smi when available because it reports the exact package version;
# fall back to parsing glxinfo.
driver=""
if command -v nvidia-smi >/dev/null 2>&1; then
    driver=$(nvidia-smi --query-gpu=driver_version --format=csv,noheader | head -n1) || true
fi

if [ -z "$driver" ]; then
    if command -v glxinfo >/dev/null 2>&1; then
        # glxinfo output looks like "OpenGL version string: 4.6.0 NVIDIA 535.86.10".
        driver=$(glxinfo | grep -i "OpenGL version string" | head -n1 \
                   | sed -E 's/.*NVIDIA[[:space:]]*([0-9]+\.[0-9]+\.[0-9]+).*/\1/') || true
    fi
fi

if [ -z "$driver" ]; then
    echo "Error: could not detect NVIDIA driver version." >&2
    echo "Are 'nvidia-smi' or 'glxinfo' installed and accessible?" >&2
    echo "Please install the appropriate Flatpak GL extension manually." >&2
    exit 1
fi

echo "Detected NVIDIA driver version: $driver"
# Flatpak extension names use dashes instead of dots (e.g. 580-126-18),
# so normalize the string accordingly before building the id.
flatpakver=$(printf "%s" "$driver" | tr '.' '-')
extension="org.freedesktop.Platform.GL.nvidia-$flatpakver//1.4"

# sanity check: flatpak binary must exist before we try to install anything
if ! command -v flatpak >/dev/null 2>&1; then
    echo "Error: 'flatpak' not found in PATH; cannot install extensions." >&2
    exit 2
fi

# check for presence without letting 'flatpak list' abort the script
if ! flatpak list --app >/dev/null 2>&1; then
    echo "Warning: 'flatpak list' failed - is the Flatpak service running?" >&2
else
    if flatpak list --app | grep -q "$extension"; then
        echo "Flatpak extension $extension is already installed."
        exit 0
    fi
fi

echo "Installing Flatpak extension: $extension"
if flatpak install --user flathub "$extension" -y; then
    echo "Installation complete.  Restart Particle-Viewer to pick up the new runtime."
else
    echo "Failed to install $extension" >&2
    exit 1
fi
