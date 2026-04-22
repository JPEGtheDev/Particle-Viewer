# C++ Toolchain Configuration

Clang-format settings and clang-tidy configuration for Particle-Viewer.

---

## clang-format Settings

Auto-enforced via `.clang-format`. Key settings:

| Rule | Value |
|------|-------|
| Standard | C++20 |
| Indentation | 4 spaces (no tabs) |
| Line length | 120 characters |
| Functions/classes braces | Allman (new line) |
| Control structure braces | K&R (same line) |
| Include order | C++ standard → system → external libs → project headers |
| Pointer alignment | Left (`int* ptr`) |

Full configuration: `.clang-format`

---

## clang-tidy Commands

```bash
# Analyze a source file
clang-tidy src/main.cpp -- -Isrc/glad/include

# Analyze using compilation database
clang-tidy src/particle.hpp -p build

# Auto-fix (always review auto-fix output before committing — it can change behavior)
clang-tidy -fix src/main.cpp -- -Isrc/glad/include
```

Configuration in `.clang-tidy` enforces:
- `modernize-*` — Modern C++ practices (smart pointers, nullptr, auto, range-based loops)
- `readability-*` — Const correctness, function complexity (≤25 cognitive, ≤50 statements, <5 params)
- `cppcoreguidelines-*` — Microsoft C++ Core Guidelines
- `performance-*`, `bugprone-*`, `portability-*`, `clang-analyzer-*`

Header filter excludes embedded libs: `glad`, `tinyFileDialogs`, `stb_*`.

**CI status:** clang-tidy runs in CI as advisory (non-blocking). clang-format is blocking.
