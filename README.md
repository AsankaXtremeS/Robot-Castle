# The Little Robot Builds a Digital Kingdom

A complete Computer Graphics algorithms demonstration built with **C++**, **OpenGL**, and **FreeGLUT**.  
A 2.5D cartoon-style animated short (~2.5 minutes) where a cute robot builds a colorful kingdom while
demonstrating **14 computer graphics algorithms** from scratch.

---

## Algorithms Demonstrated

| Scene | Algorithm |
|---|---|
| Scene 1 | DDA Line Drawing |
| Scene 1 | Bresenham Line Drawing |
| Scene 1 | Midpoint Circle Algorithm |
| Scene 1 | Bresenham Circle Algorithm |
| Scene 2 | Scan-Line Polygon Fill |
| Scene 2 | Flood Fill |
| Scene 2 | Boundary Fill |
| Scene 3 | Point Clipping |
| Scene 3 | Cohen-Sutherland Line Clipping |
| Scene 3 | Liang-Barsky Line Clipping |
| Scene 3 | Sutherland-Hodgman Polygon Clipping |
| Scene 4 | 3×3 Transformation Matrices (T, R, S, Composite) |
| Scene 5 | Isometric Projection Matrix |
| Scene 6 | Painter's Algorithm + Parallax |

---

## Project Structure

```
RobotKingdom/
├── build.bat                        ← Windows build script
├── README.md
├── include/
│   ├── Common.h                     ← Shared types (Color, Point2D, Matrix3x3 …)
│   ├── SceneManager.h               ← Scene orchestration
│   ├── Robot.h                      ← Robot character
│   ├── Renderer.h                   ← OpenGL rendering helpers
│   └── Algorithms/
│       ├── LineDrawing.h            ← DDA + Bresenham line
│       ├── CircleDrawing.h          ← Midpoint + Bresenham circle
│       ├── Filling.h                ← Scan-line, Flood fill, Boundary fill
│       ├── Clipping.h               ← Point, Cohen-S, Liang-B, Sutherland-H
│       ├── Transformations.h        ← 3×3 matrices (T/R/S/Composite)
│       ├── Projection.h             ← Isometric projection
│       └── PaintersAlgorithm.h      ← Depth sort + parallax
└── src/
    ├── main.cpp                     ← FreeGLUT setup + main loop
    ├── Renderer.cpp
    ├── Robot.cpp
    ├── SceneManager.cpp             ← All scene logic
    └── Algorithms/
        ├── LineDrawing.cpp
        ├── CircleDrawing.cpp
        ├── Filling.cpp
        ├── Clipping.cpp
        ├── Transformations.cpp
        ├── Projection.cpp
        └── PaintersAlgorithm.cpp
```

---

## Prerequisites

### 1. MinGW-w64 (g++ compiler)

Download from: https://winlibs.com/  
Choose **UCRT runtime** version with **POSIX threads**.  
After installing, add the `bin` folder to your **PATH** environment variable.

Verify: open a Command Prompt and run:
```
g++ --version
```

### 2. FreeGLUT

**Option A – MSYS2 (easiest):**
```bash
pacman -S mingw-w64-x86_64-freeglut
```

**Option B – Manual:**
1. Download pre-built binaries from: https://www.transmissionzero.co.uk/software/freeglut-devel/
2. Extract to `C:\freeglut\`
3. The build script will auto-detect this location.

**Option C – vcpkg:**
```
vcpkg install freeglut:x64-windows
```

---

## Building

Open a **Command Prompt** in the `RobotKingdom` folder and run:

```batch
build.bat
```

This will:
1. Detect FreeGLUT automatically
2. Compile all source files with g++ -std=c++17 -O2
3. Copy the FreeGLUT DLL next to the executable
4. Launch the animation automatically

**Compile only (no launch):**
```batch
build.bat compile
```

**Manual compilation command:**
```batch
g++ -std=c++17 -O2 ^
    src/main.cpp src/Renderer.cpp src/Robot.cpp src/SceneManager.cpp ^
    src/Algorithms/LineDrawing.cpp src/Algorithms/CircleDrawing.cpp ^
    src/Algorithms/Filling.cpp src/Algorithms/Clipping.cpp ^
    src/Algorithms/Transformations.cpp src/Algorithms/Projection.cpp ^
    src/Algorithms/PaintersAlgorithm.cpp ^
    -IC:\freeglut\include -LC:\freeglut\lib ^
    -lfreeglut -lopengl32 -lglu32 ^
    -o RobotKingdom.exe
```

---

## Controls

| Key | Action |
|---|---|
| `SPACE` / `→` | Next scene |
| `←` | Previous scene |
| `1` – `7` | Jump directly to scene 1–7 |
| `F` | Toggle full-screen |
| `ESC` / `Q` | Quit |

---

## Scene Descriptions

| # | Scene | Duration | Algorithms |
|---|---|---|---|
| 1 | The Robot Arrives | 5s | (intro / setup) |
| 2 | Drawing the Kingdom | 20s | DDA Line, Bresenham Line, Midpoint Circle, Bresenham Circle |
| 3 | Colouring the Kingdom | 20s | Scan-Line Fill, Flood Fill, Boundary Fill |
| 4 | The Magic Portal | 20s | Point Clip, Cohen-Sutherland, Liang-Barsky, Sutherland-Hodgman |
| 5 | Kingdom Transformations | 25s | Translation, Rotation, Scaling, Composite (3×3 matrices) |
| 6 | Isometric Upgrade | 25s | Isometric Projection Matrix |
| 7 | Grand Finale | 25s | Painter's Algorithm, Parallax Scrolling, Fireworks |

---

## Architecture

- **SceneManager** – singleton that tracks scene index, elapsed time, and transitions
- **Timer-based animation** – GLUT timer at 60 FPS; each algorithm receives a `progress` ∈ [0,1] for smooth animation
- **Algorithm isolation** – every algorithm is in its own `.cpp`/`.h` pair; **no** built-in OpenGL fill/clip functions are used
- **Robot** – assembled from transformed primitives; animated via sine-wave joint angles
- **Painter's Algorithm** – `std::stable_sort` by depth; parallax via GL matrix translation

---

## Troubleshooting

**`freeglut.dll not found` error:**  
Copy `freeglut.dll` from your FreeGLUT `bin/` folder next to `RobotKingdom.exe`.

**Black screen / no rendering:**  
Ensure your graphics driver supports OpenGL 2.x or later.

**`g++` not found:**  
Add MinGW's `bin` directory to your PATH.

**Slow performance:**  
The flood fill / boundary fill visual simulation uses many draw calls.  
Lower the window resolution or set `-O3` in build.bat for better performance.
