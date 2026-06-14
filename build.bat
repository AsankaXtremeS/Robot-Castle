@echo off
REM ============================================================
REM  build.bat
REM  Compile "The Little Robot Builds a Digital Kingdom"
REM  using MinGW g++ and FreeGLUT on Windows
REM
REM  Prerequisites:
REM    - MinGW-w64 (g++ on PATH)
REM    - FreeGLUT installed (see README.md for locations)
REM
REM  Usage:
REM    build.bat            -- compile and run
REM    build.bat compile    -- compile only
REM    build.bat clean      -- remove build artifacts
REM ============================================================

setlocal EnableDelayedExpansion

REM ── Configuration ────────────────────────────────────────────────────────────

REM If FreeGLUT headers/libs are in a custom location, set these:
set FREEGLUT_INC=C:\Users\LOQ\Downloads\Glut\freeglut\include
set FREEGLUT_LIB=C:\Users\LOQ\Downloads\Glut\freeglut\lib
set FREEGLUT_BIN=C:\Users\LOQ\Downloads\Glut\freeglut\bin

REM Auto-detect common install locations
if exist "%USERPROFILE%\Desktop\Glut\freeglut\include\GL\freeglut.h" (
    set FREEGLUT_INC=%USERPROFILE%\Desktop\Glut\freeglut\include
    set FREEGLUT_LIB=%USERPROFILE%\Desktop\Glut\freeglut\lib\x64
    set FREEGLUT_BIN=%USERPROFILE%\Desktop\Glut\freeglut\bin\x64
) else if exist "C:\freeglut\include\GL\freeglut.h" (
    set FREEGLUT_INC=C:\freeglut\include
    set FREEGLUT_LIB=C:\freeglut\lib\x64
    set FREEGLUT_BIN=C:\freeglut\bin\x64
) else if exist "C:\Program Files\freeglut\include\GL\freeglut.h" (
    set "FREEGLUT_INC=C:\Program Files\freeglut\include"
    set "FREEGLUT_LIB=C:\Program Files\freeglut\lib\x64"
    set "FREEGLUT_BIN=C:\Program Files\freeglut\bin\x64"
) else if exist "C:\mingw64\include\GL\freeglut.h" (
    set FREEGLUT_INC=C:\mingw64\include
    set FREEGLUT_LIB=C:\mingw64\lib
    set FREEGLUT_BIN=C:\mingw64\bin
) else if exist "C:\msys64\mingw64\include\GL\freeglut.h" (
    set FREEGLUT_INC=C:\msys64\mingw64\include
    set FREEGLUT_LIB=C:\msys64\mingw64\lib
    set FREEGLUT_BIN=C:\msys64\mingw64\bin
) else (
    echo [WARNING] FreeGLUT not found in common paths.
    echo           Set FREEGLUT_INC and FREEGLUT_LIB manually in this script.
    echo           See README.md for installation instructions.
)

REM ── Source files ─────────────────────────────────────────────────────────────
set SRCS=^
    src\main.cpp ^
    src\Renderer.cpp ^
    src\Robot.cpp ^
    src\SceneManager.cpp ^
    src\Algorithms\LineDrawing.cpp ^
    src\Algorithms\CircleDrawing.cpp ^
    src\Algorithms\Filling.cpp ^
    src\Algorithms\Clipping.cpp ^
    src\Algorithms\Transformations.cpp ^
    src\Algorithms\Projection.cpp ^
    src\Algorithms\PaintersAlgorithm.cpp

set OUT=RobotKingdom.exe

REM ── Compiler flags ────────────────────────────────────────────────────────────
set CXXFLAGS=-std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter

REM ── Clean ─────────────────────────────────────────────────────────────────────
if "%1"=="clean" (
    echo Cleaning...
    if exist "%OUT%" del "%OUT%"
    if exist "*.o" del /Q *.o
    echo Done.
    goto :EOF
)

REM ── Build ─────────────────────────────────────────────────────────────────────
echo.
echo ============================================================
echo  Building: The Little Robot Builds a Digital Kingdom
echo ============================================================
echo.

if defined FREEGLUT_INC (
    set INC_FLAG=-I"%FREEGLUT_INC%"
    set LIB_FLAG=-L"%FREEGLUT_LIB%"
) else (
    set INC_FLAG=
    set LIB_FLAG=
)

set CMD=g++ %CXXFLAGS% -I. -Isrc %INC_FLAG% %SRCS% %LIB_FLAG% -lfreeglut -lopengl32 -lglu32 -o %OUT%

echo Command: %CMD%
echo.

%CMD%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    echo.
    echo  Troubleshooting:
    echo   1. Make sure g++ (MinGW) is installed and on your PATH
    echo   2. Make sure FreeGLUT is installed -- see README.md
    echo   3. If FreeGLUT is in a non-standard location, edit FREEGLUT_INC
    echo      and FREEGLUT_LIB at the top of this build.bat file
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo  Build successful!  Output: %OUT%
echo ============================================================
echo.

REM ── Run ──────────────────────────────────────────────────────────────────────
if "%1"=="compile" (
    echo Compile-only mode. Not launching.
    goto :EOF
)

echo  Launching animation...
echo  Controls: SPACE=next scene, 1-7=jump to scene, ESC=quit
echo.

REM Copy freeglut DLL next to exe if found
if defined FREEGLUT_BIN (
    if exist "%FREEGLUT_BIN%\freeglut.dll" (
        copy /Y "%FREEGLUT_BIN%\freeglut.dll" . >nul 2>&1
        echo  Copied freeglut.dll from %FREEGLUT_BIN%
    )
) else if defined FREEGLUT_LIB (
    if exist "%FREEGLUT_LIB%\freeglut.dll" (
        copy /Y "%FREEGLUT_LIB%\freeglut.dll" . >nul 2>&1
    )
)

start "" "%OUT%"
