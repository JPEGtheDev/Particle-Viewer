# Third-Party Notices

This file contains the licenses and notices for third-party software used in Particle-Viewer.

**Note:** Full license texts are included for embedded libraries. External dependencies (GLFW, GLM, GoogleTest) are referenced with links to their respective repositories for the complete license texts.

---

## Embedded Libraries

The following libraries are embedded directly in the source code of this project:

### STB Libraries

**stb_image.h, stb_image_write.h, stb_easy_font.h**

- **Author:** Sean Barrett
- **Copyright:** Copyright (c) 2017 Sean Barrett
- **License:** Dual MIT/Public Domain
- **Source:** https://github.com/nothings/stb

These libraries are available under your choice of two licenses:

#### ALTERNATIVE A - MIT License

```
Copyright (c) 2017 Sean Barrett

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

#### ALTERNATIVE B - Public Domain (www.unlicense.org)

```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

---

### Tiny File Dialogs

**tinyfiledialogs.c, tinyfiledialogs.h**

- **Author:** Guillaume Vareille
- **Copyright:** Copyright (c) 2014 - 2023 Guillaume Vareille
- **License:** ZLIB License (SPDX-License-Identifier: ZLIB)
- **Website:** http://ysengrin.com
- **Source:** http://sourceforge.net/projects/tinyfiledialogs/
- **Version:** v3.16 [Nov 23, 2023]

```
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
```

---

### GLAD

**glad.c, glad.h** (OpenGL loader)

- **Generator:** glad 0.1.33
- **Generated:** Wed Jul 22 13:02:02 2020
- **Source:** https://github.com/Dav1dde/glad
- **License:** Public Domain (CC0 1.0 Universal)

GLAD is an OpenGL loader generator. The generated code in this repository does not contain an explicit license header, which is consistent with GLAD's default behavior of generating public domain code. The generated files may be used without restriction.

Code was generated for:
- **Language/Generator:** C/C++
- **Specification:** gl
- **APIs:** gl=4.6
- **Profile:** compatibility

---

### KHR Platform Headers

**khrplatform.h**

- **Copyright:** Copyright (c) 2008-2018 The Khronos Group Inc.
- **License:** MIT-style License
- **Source:** https://github.com/KhronosGroup/EGL-Registry

```
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
```

---

## External Dependencies

The following libraries must be installed on the system to build and run this project:

### GLFW

- **Purpose:** Cross-platform window and OpenGL context management
- **License:** zlib/libpng License
- **Source:** https://github.com/glfw/glfw
- **Website:** https://www.glfw.org/
- **Required Version:** 3.3 or higher

GLFW is licensed under the zlib/libpng license. For full license text, see the GLFW repository or website.

---

### GLM

- **Purpose:** OpenGL Mathematics library for graphics software
- **License:** MIT License (dual-licensed with The Happy Bunny License; this project uses MIT)
- **Source:** https://github.com/g-truc/glm
- **Website:** https://glm.g-truc.net/

GLM is a header-only library dual-licensed under MIT and The Happy Bunny License. This project uses GLM under the MIT License terms. For full license text, see the GLM repository.

---

### GoogleTest

- **Purpose:** Unit testing framework (development dependency)
- **License:** BSD 3-Clause License
- **Source:** https://github.com/google/googletest
- **Included Via:** CMake FetchContent (version 1.12.1)

GoogleTest is automatically fetched during the build process via CMake. For full license text, see the GoogleTest repository.

---

### Dear ImGui

- **Purpose:** Immediate mode GUI library for developer tools and menus
- **Author:** Omar Cornut
- **License:** MIT License
- **Source:** https://github.com/ocornut/imgui
- **Included Via:** CMake FetchContent (version 1.91.6)

```
The MIT License (MIT)

Copyright (c) 2014-2024 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## OpenGL

This software uses OpenGL, which is an industry-standard graphics API. OpenGL is a trademark of Silicon Graphics, Inc. (SGI) and later Khronos Group. The OpenGL specification and headers are available under various permissive licenses from the Khronos Group.

---

## Build Tools

### CMake

- **Purpose:** Cross-platform build system generator
- **License:** BSD 3-Clause License
- **Website:** https://cmake.org/
- **Required Version:** 3.18 or higher

CMake is used as the build system but is not distributed with this software.

---

## Summary

This project gratefully uses the following third-party components:

**Embedded in source:**
- STB Libraries (Sean Barrett) - MIT/Public Domain
- Tiny File Dialogs (Guillaume Vareille) - ZLIB License
- GLAD (OpenGL Loader Generator) - Public Domain/MIT
- KHR Platform Headers (Khronos Group) - MIT-style

**External dependencies:**
- GLFW - zlib/libpng License
- GLM - MIT/The Happy Bunny License
- GoogleTest (dev only) - BSD 3-Clause
- Dear ImGui - MIT License
- OpenGL - Industry standard API

For the most up-to-date license information, please refer to the respective project repositories and official documentation.

---

*Last Updated: 2026-02-08*
