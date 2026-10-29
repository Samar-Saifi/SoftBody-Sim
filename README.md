# SoftBody-Sim

A real-time 3D Soft Body physics simulation engine built with **C++**, **OpenGL 3.3 Core**, and **Dear ImGui**.
The project simulates deformable soft bodies using a mass-spring-damper network combined with internal pressure and volume conservation forces. It features dynamic ground collision, directional shadow mapping, interactive particle ray-picking & dragging, and a modular architecture separating rendering, physics, and collision systems.
---

## ✨ Features

- **Mass-Spring-Damper Physics**:
  - **Structural Springs**: Preserve edges and lattice geometry.
  - **Shear / Diagonal Springs**: Resist in-plane shearing deformations.
  - **Bending Springs**: Provide curvature resistance and surface elasticity.
  - **Antipodal / Internal Springs**: Reinforce 3D volumetric structural integrity.
- **Internal Pressure & Volume Conservation**:
  - Computes instantaneous enclosed volume using divergence theorem on surface triangles.
  - Applies normal pressure forces to counter volume compression and maintain inflation.
- **Interactive Particle Picking & 3D Dragging**:
  - Raycasts from mouse cursor into 3D world space.
  - Interactively grabs the nearest particle and moves it along a camera-facing plane in real-time.
- **Multiple Deformable Shapes**:
  - **Sphere**: Configurable radius, stacks, sectors, and seam synchronization.
  - **Cube**: 3D volumetric lattice with configurable size and grid resolution.
- **Collision Dynamics**:
  - Ground plane collision detection and response with configurable coefficient of restitution and surface friction.
- **Shadow Mapping & Lighting**:
  - Directional depth framebuffer pass (FBO) with shadow bias and perspective-correct depth calculation.
  - Blinn-Phong lighting model with ambient, diffuse, and cast shadows.
- **Live ImGui Control Panel**:
  - Live parameter adjustment: Substeps, Stiffness ($K_1, K_2, K_3$), Damping ($D_1, D_2, D_3$).
  - Geometry reconfiguration and on-the-fly shape regeneration.
  - Real-time simulation statistics (FPS, frame time, particle count, spring count, volume).

---

## 🎮 Controls

| Input | Action |
| :--- | :--- |
| **Right Mouse Button + Drag** | Orbit / Rotate Camera |
| **Mouse Scroll** | Zoom Camera in / out |
| **Left Mouse Button (Click on Shape)** | Pick closest particle and drag in 3D |
| **Left Mouse Button (Release)** | Release picked particle |
| **Escape** | Close Application |

---

## 🏗️ Architecture

The codebase follows a modular design separating the core engine responsibilities:

```
SoftBody-Sim/
├── shaders/                   # GLSL shader programs
│   ├── vert.shader            # Main vertex shader (lighting + shadow coord)
│   ├── frag.shader            # Main fragment shader (directional lighting + shadow test)
│   ├── depthVert.shader       # Shadow depth map vertex pass
│   └── depthFrag.shader       # Shadow depth map fragment pass
├── src/
│   ├── renderer/              # Modern OpenGL rendering abstraction
│   │   ├── Buffer.h / .cpp    # VertexBuffer and IndexBuffer wrappers
│   │   ├── VertexArray.h / .cpp # VertexArray (VAO) layout and attribute binding
│   │   ├── Shader.h / .cpp    # Shader loader, compiler, linker & uniform utilities
│   │   └── Renderer.h / .cpp  # Centralized draw calls and render states
│   ├── physics/               # Soft body physics engine
│   │   ├── Particle.h         # Particle state (position, velocity, normal, mass)
│   │   ├── Spring.h           # Spring definition (endpoints, rest length, stiffness, damping)
│   │   ├── SoftBody.h / .cpp  # Base soft body class (volume, pressure, springs, integration)
│   │   ├── SoftBodySphere.h / .cpp # Deformable sphere generator
│   │   └── SoftBodyCube.h / .cpp   # 3D lattice deformable cube generator
│   ├── collision/             # Collision system
│   │   └── Collision.h / .cpp # Ground collision detection & response
│   ├── Camera.h / .cpp        # Orbit camera matrix calculations
│   ├── ShapeConfig.h          # Shape configuration descriptors
│   ├── UI.h / .cpp            # Dear ImGui interface and spawner controls
│   └── main.cpp               # Application setup and render loop
└── CMakeLists.txt             # CMake build configuration
```

---

## 🛠️ Build & Run

### Prerequisites

- **CMake** (v3.15 or newer)
- **C++17** or **C++20** compatible compiler (GCC, Clang, or MSVC)
- **OpenGL 3.3+** graphics drivers
- Standard development libraries (X11 / Wayland on Linux)

### Building

```bash
# Clone the repository
git clone https://github.com/Samar-Saifi/SoftBody-Sim.git
cd SoftBody-Sim

# Configure CMake
cmake -B build -S .

# Compile
cmake --build build -j$(nproc)
```

Dependencies (**GLFW**, **GLM**, **Dear ImGui**) are automatically fetched and built via CMake's `FetchContent`.

### Running

```bash
./Soft_Body
```

---

## 📦 Dependencies

- [OpenGL](https://www.opengl.org/) - Graphics API
- [GLAD](https://glad.dav1d.de/) - Multi-Language OpenGL Loader
- [GLFW](https://www.glfw.org/) - Windowing and Input
- [GLM](https://github.com/g-truc/glm) - Mathematics Library
- [Dear ImGui](https://github.com/ocornut/imgui) - Bloat-free Graphical User Interface