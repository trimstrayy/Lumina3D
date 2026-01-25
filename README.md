# Lumina3D Engine

A software-based 3D graphics engine built for educational purposes in **COMP 342 Computer Graphics**.

## Overview

Lumina3D is designed to teach fundamental computer graphics concepts by manually implementing core rendering algorithms without relying on built-in GPU functions.

## Quick Start

### Prerequisites
- Windows OS
- CMake (version 3.15 or higher)
- C++ compiler (MinGW-w64, MSVC, or GCC)
- Git

### Installation (For New Users Cloning This Repo)

1. **Clone the repository:**
   ```powershell
   git clone <repository-url>
   cd Lumina3D
   ```

2. **Install dependencies:**
   ```powershell
   .\install-manual.ps1
   ```
   This script downloads and installs GLFW and GLM to the `external/` directory.

3. **Build the project:**
   ```powershell
   cmake -B build -G "MinGW Makefiles"
   cmake --build build --config Release
   ```
   
   Or use the automated build script:
   ```powershell
   .\build.ps1
   ```

4. **Run the engine:**
   ```powershell
   cd build\bin
   .\Lumina3D.exe
   ```

### Controls
- **Arrow Keys** - Rotate the 3D object
- **+/-** - Scale the object up/down
- **R** - Reset all transformations
- **ESC** - Exit the application

## Features

### Core Rendering Algorithms
- **Bresenham's Line Algorithm** - Efficient line rasterization using only integer arithmetic
- **Mid-Point Circle Algorithm** - Circle drawing with 8-way symmetry
- **Triangle Rasterization** - Scanline-based triangle filling with barycentric interpolation
- **Z-Buffer (Depth Buffer)** - Hidden surface removal for overlapping 3D objects

### Transformation Pipeline
- **Model-View-Projection (MVP) Matrices** using homogeneous coordinates
- **Perspective and Orthographic Projections**
- **Viewport Transformation** from NDC to screen coordinates
- **Cohen-Sutherland Line Clipping** for efficient viewport clipping

### Shading Models
- **Gouraud Shading** - Per-vertex lighting with color interpolation
- **Phong Shading** - Per-pixel lighting for accurate specular highlights
- **Blinn-Phong Reflection Model** - Ambient, diffuse, and specular components

## Project Structure

```
Lumina3D/
├── CMakeLists.txt          # Build configuration
├── build.ps1               # Automated build script
├── install-manual.ps1      # Dependency installer
├── README.md               # This file
├── include/                # Header files
│   ├── Engine.h           # Main engine class
│   ├── Rasterizer.h       # Drawing primitives
│   ├── Transform.h        # Transformation pipeline
│   └── Shaders.h          # Lighting and shading
├── src/                   # Source files
│   ├── main.cpp          # Entry point and GLFW setup
│   ├── Rasterizer.cpp    # Bresenham, Mid-point algorithms
│   ├── Transform.cpp     # Matrix operations, clipping
│   └── Renderer.cpp      # Shading implementations
└── assets/               # Resources (textures, models)
```

## Dependencies

- **C++17** or later
- **GLFW 3.x** - Windowing and input handling
- **GLM** - Mathematics library for vectors and matrices
- **OpenGL 3.3+** - Used only for displaying the pixel buffer (no GPU rendering)

## Building the Project

### Method 1: Automated Build Script (Recommended)

```powershell
# Install dependencies first (if not already done)
.\install-manual.ps1

# Build the project
.\build.ps1
```

### Method 2: Manual CMake Build

```powershell
# 1. Install dependencies
.\install-manual.ps1

# 2. Configure the project
cmake -B build -G "MinGW Makefiles"

# 3. Build
cmake --build build --config Release

# 4. Run
cd build\bin
.\Lumina3D.exe
```

### Rebuilding After Code Changes

```powershell
# Quick rebuild
cmake --build build --config Release

# Clean rebuild
Remove-Item -Recurse -Force build
cmake -B build -G "MinGW Makefiles"
cmake --build build --config Release
```

## Running the Application

After building, run:
```powershell
.\build\bin\Lumina3D.exe
```

The application will:
- Open a window displaying a colored 3D cube
- Print transformation matrices to the console
- Respond to keyboard input for rotation and scaling

## Educational Concepts Demonstrated

### 1. Rasterization
- Converting geometric primitives (lines, circles, triangles) to pixels
- Bresenham's algorithm for efficient line drawing
- Mid-point algorithms for curve generation

### 2. 3D Transformations
- Homogeneous coordinates for unified transformation representation
- Model-View-Projection pipeline
- Matrix composition and transformation order

### 3. Clipping
- Cohen-Sutherland algorithm with region codes
- Efficient viewport culling

### 4. Visible Surface Determination
- Z-buffer algorithm for depth testing
- Per-pixel depth comparison

### 5. Illumination Models
- Gouraud vs Phong shading comparison
- Blinn-Phong reflection model
- Ambient, diffuse, and specular lighting components

## Code Structure

### Engine.h/cpp
Main engine class that manages:
- Window creation and event handling
- Rendering loop
- User input processing
- Scene setup

### Rasterizer.h/cpp
Low-level drawing primitives:
- `draw_line()` - Bresenham's algorithm
- `draw_circle()` - Mid-point circle algorithm
- `drawTriangle()` - Scanline rasterization with Z-buffering
- Frame buffer and depth buffer management

### Transform.h/cpp
Transformation pipeline:
- Matrix creation (translation, rotation, scale)
- Camera setup (lookAt, perspective, orthographic)
- Vertex transformation (MVP pipeline)
- Cohen-Sutherland clipping

### Shaders.h/cpp (Renderer.cpp)
Lighting calculations:
- Gouraud shading implementation
- Phong shading implementation
- Blinn-Phong reflection model
- Color interpolation

## Mathematical Concepts

### Homogeneous Coordinates
Points are represented as $(x, y, z, w)$ where $w=1$ for positions and $w=0$ for directions.

### Transformation Matrix (4×4)
$$
\begin{bmatrix}
r_{11} & r_{12} & r_{13} & t_x \\
r_{21} & r_{22} & r_{23} & t_y \\
r_{31} & r_{32} & r_{33} & t_z \\
0 & 0 & 0 & 1
\end{bmatrix}
$$

### Perspective Division
After MVP transformation, convert from clip space to NDC:
$$
\text{NDC} = \frac{(x, y, z)}{w}
$$

### Blinn-Phong Lighting
$$
I = I_a \cdot k_a + I_d \cdot k_d \cdot (\mathbf{N} \cdot \mathbf{L}) + I_s \cdot k_s \cdot (\mathbf{N} \cdot \mathbf{H})^n
$$

Where:
- $I_a, I_d, I_s$ = ambient, diffuse, specular light intensities
- $k_a, k_d, k_s$ = material coefficients
- $\mathbf{N}$ = surface normal
- $\mathbf{L}$ = light direction
- $\mathbf{H}$ = halfway vector between light and view
- $n$ = shininess factor

## Future Enhancements

- Texture mapping
- Normal mapping
- Shadow mapping
- Anti-aliasing (MSAA, FXAA)
- 3D model loading (OBJ format)
- Additional clipping algorithms (Liang-Barsky, Sutherland-Hodgman)

## Troubleshooting

### Build Issues
- **CMake not found**: Install CMake from https://cmake.org/download/
- **Compiler not found**: Install MinGW-w64 or ensure your compiler is in PATH
- **Dependencies not found**: Run `.\install-manual.ps1` again

### Runtime Issues
- **Window doesn't open**: Update your graphics drivers to support OpenGL 3.3+
- **DLL missing**: Ensure `glfw3.dll` is in the same directory as `Lumina3D.exe` (the build script handles this automatically)

## For Contributors

When cloning this repository:
1. Do NOT commit the `external/` directory (it's in .gitignore)
2. Do NOT commit the `build/` directory
3. Run `.\install-manual.ps1` to set up dependencies locally
4. Each developer needs to install dependencies on their machine

## License

This project is created for educational purposes in COMP 342 Computer Graphics course.

## Author

Created for Lumina3D Engine - An Educational Graphics Project

## References

- *Computer Graphics: Principles and Practice* by Foley, van Dam, Feiner, and Hughes
- *Fundamentals of Computer Graphics* by Peter Shirley
- OpenGL Programming Guide (Red Book)
- GLM Documentation: https://glm.g-truc.net/
- GLFW Documentation: https://www.glfw.org/

<!-- Update 1: 2026-01-22 -->

<!-- Update 2: 2026-01-26 -->
