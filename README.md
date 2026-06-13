# The Little Robot Builds a Digital Kingdom

A complete Computer Graphics algorithms demonstration built with **C++**, **OpenGL**, and **FreeGLUT**.  
A cartoon-style animated short (~2.5 minutes) where a cute robot builds a colorful kingdom while demonstrating **15 computer graphics algorithms** from scratch.

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
| Scene 5 | Painter's Algorithm (Depth Sort) |
| Scene 5 | Parallax Scrolling |
| Scene 5 | Particle System (Fireworks) |

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
    src/Algorithms/Transformations.cpp ^
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
| `1` – `6` | Jump directly to scenes 1–6 |
| `F` | Toggle full-screen |
| `ESC` / `Q` | Quit |

---

## Scene Descriptions

| # | Scene | Duration | Algorithms & Visualized Concepts |
|---|---|---|---|
| 0 | The Robot Arrives | 5s | Introductory scene and environment initialization. |
| 1 | Drawing the Kingdom | 20s | DDA Line, Bresenham Line (castle walls), Midpoint Circle, Bresenham Circle (cart wheels). |
| 2 | Colouring the Kingdom | 20s | Scan-Line Fill (towers/roofs), Flood Fill (river), Boundary Fill (windows, gate, garden). |
| 3 | The Magic Portal | 20s | Point Clip (fireflies), Cohen-Sutherland (lighthouse beams), Liang-Barsky (lasers), Sutherland-Hodgman (layered alien ship: dome, saucer, thruster, tractor beam, lights). |
| 4 | Kingdom Transformations | 25s | Translation (house sliding), Rotation (clock hands), Scaling (evergreen tree growth), Composite (orbiting/rotating sun). |
| 5 | Grand Finale | 25s | Painter's Algorithm (depth sorting layers), Parallax Scrolling (looping background), Particle System (fireworks). |

---

## Architecture

- **SceneManager** – Singleton orchestrating scene index, timing, fade transitions, and dynamic algorithm label updates.
- **Timer-based animation** – Runs GLUT timer at 60 FPS; each algorithm receives a `progress` ∈ [0,1] for smooth, frame-independent animation.
- **Algorithm isolation** – Core computer graphics algorithms are implemented from scratch in their own files without OpenGL's built-in fill or clip functions.
- **Robot Model** – Procedurally assembled from primitive shapes and animated via sine-wave joint rotations.
- **Painter's Algorithm** – Uses `std::stable_sort` by depth value; parallax translations are wrapped for seamless looping.

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
Lower the window resolution or build with optimizations (`-O2` or `-O3`) in the build/compilation stage.
