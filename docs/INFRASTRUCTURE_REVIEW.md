---
title: "Infrastructure Review Instantiation"
description: "Particle Viewer's concrete build-reproducibility and packaging checklists, instantiating the story-to-ship infrastructure-review skill's generic rules for CMake and Flatpak."
domain: cross-cutting
subdomain: ""
tags: [cross-cutting, process, review, build, packaging, flatpak, cmake]
related:
  - "VERIFICATION_COMMANDS.md"
  - "flatpak/FLATPAK.md"
  - "flatpak/flatpak-build.md"
  - "flatpak/flatpak-gotchas.md"
---

# Particle-Viewer Infrastructure Review Instantiation

The story-to-ship `infrastructure-review` skill defines tool-agnostic checklists for CI/CD pipelines, reproducible builds, and sandboxed/least-privilege packaging. This doc is the concrete instantiation of those checklists against Particle Viewer's actual build system (CMake) and packaging format (Flatpak) -- use it alongside the skill's generic checklist, not instead of it.

## Reproducible Build Review (CMake)

Applies to `CMakeLists.txt` (root and `tests/`):

1. Third-party dependencies are declared via `FetchContent_Declare` in the root `CMakeLists.txt`: `imgui` (tag `v1.91.6`), `SDL3` (tag `release-3.2.6`), `googletest` (tag `release-1.12.1`). All three are pinned to an immutable git tag, never `main` or `master`.
2. `find_package(SDL3)` is preferred over `FetchContent` when a system-installed SDL3 is available (e.g. the Flatpak `sdl3` module); this is a build-time selection, not an unpinned fallback -- the Flatpak module itself is still pinned to `release-3.2.6`.
3. The test target `ParticleViewerTests` (`tests/CMakeLists.txt`) is a separate `add_executable` from the production target `Viewer` (`CMakeLists.txt`); neither links the other's object files.
4. No hardcoded absolute paths -- source globs and dependency paths are built from `CMAKE_SOURCE_DIR`.

## Sandboxed/Least-Privilege Packaging Review (Flatpak)

Applies to `flatpak/org.particleviewer.ParticleViewer.yaml` and its sibling `.desktop`/`.metainfo.xml`/`.svg` files:

1. `finish-args` declares only the capabilities the application needs, each with an inline comment justifying it:
   - `--device=all` -- required for the NVIDIA proprietary driver path (`--device=dri` alone does not expose the NVIDIA userspace libraries). Disclosure: this grant is broader than graphics alone -- it exposes all host device nodes under `/dev` (input, USB, and other devices), not just the GPU nodes.
   - `--socket=x11`, `--socket=wayland` -- display server access
   - `--share=ipc` -- required alongside the X11/Wayland sockets
   - `--filesystem=home` -- read-write access for writing `window.cfg`
   - `--filesystem=host:ro` -- the ImGui file dialog browses the filesystem directly instead of using the FileChooser portal; this subsumes `xdg-documents` and `xdg-download`
2. Third-party build modules declared in the manifest are pinned to an immutable ref: `glm` to a release archive with a `sha256` digest, `sdl3` to git tag `release-3.2.6` (the same tag the CMake `FetchContent` fallback uses in the Reproducible Build Review above) -- neither is a branch checkout. `glm` is pinned only here; it does not appear in `CMakeLists.txt` or `tests/CMakeLists.txt`.
3. `--share=network` is absent -- Particle Viewer does not need network access, and none is granted.
4. The app ID `org.particleviewer.ParticleViewer` matches the project's reverse-DNS naming convention and is consistent across the manifest, `.desktop`, and `.metainfo.xml` files.
5. `runtime: org.freedesktop.Platform` and `sdk: org.freedesktop.Sdk` are pinned to `runtime-version: '24.08'`, not a floating release.

## Related

- [Verification Commands](VERIFICATION_COMMANDS.md) -- the build/test/format gate referenced by the Reproducible Build Review
- [Flatpak Distribution Guide](flatpak/FLATPAK.md) -- navigation hub for Flatpak build, user, and troubleshooting docs
- [Flatpak Build Guide](flatpak/flatpak-build.md) -- manifest configuration details for the checks above
- [Flatpak CI/CD and Troubleshooting](flatpak/flatpak-gotchas.md) -- the permission grants and CI triggers referenced above
