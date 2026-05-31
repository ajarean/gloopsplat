# gloopsplat
Fluid Animation with 3D Gaussian Splat (3DGS) Rendering and Position-Based Gaussian Kernel Simulation

Developers:
- Andy Jarean (ajarean)
- Tingxuan Wu (tingtingtingtin)

## Controls
- Right mouse (drag) - Change camera view
- Fly controls:
  - WASD - Lateral movement
  - QE - Vertical movement
  - Shift (hold) - Increase movement speed
- Space - Pause simulation
- R - Reset simulation
- P - Record video of current parameters using ffmpeg. See 'Record' tab for recording settings

## Dependencies
- CMake 3.30+
- C++20 compiler
- [FFmpeg](https://ffmpeg.org/download.html)

## Build Instructions
```
cmake -B build
cmake --build .\build
.\build\gloopsplat.exe
```

## Libraries Used
* GLFW (https://github.com/glfw/glfw)
  * https://www.glfw.org/docs/latest/
* GLAD (https://glad.dav1d.de/)
* GLM (https://github.com/g-truc/glm)
* imGUI (https://github.com/ocornut/imgui/releases/tag/v1.92.6)
* stb_image (https://github.com/nothings/stb)
* OpenMP (https://www.openmp.org/)
* OpenGL (https://www.opengl.org/)
* FFmpeg (https://ffmpeg.org/)

## Resources
- Image Loader Library: https://github.com/nothings/stb
- Skyboxes:
  - "Skybox" - https://learnopengl.com/img/textures/skybox.zip
  - "Space" - https://wwwtyro.github.io/space-2d/ (Free-to-use license)
    - https://github.com/wwwtyro/space-3d/blob/master/LICENSE
  - "Mountain" - http://www.humus.name
    - License: Creative Commons Attribution 3.0 Unported License
    - http://creativecommons.org/licenses/by/3.0/
- Tutorials referenced:
  - LearnOpenGL (https://learnopengl.com and https://github.com/JoeyDeVries/LearnOpenGL/)
    - License: CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/)
    - Attributed to: Joey de Vries (Twitter: @JoeyDeVriez)
- Research papers referenced:
  - Feng et. al, "Gaussian Splashing: Unified Particles for Versatile Motion Synthesis and Rendering" - https://gaussiansplashing.github.io/
  - Simulation
    - Muller et. al, "Position Based Fluids" - https://mmacklin.com/pbf_sig_preprint.pdf
    - Muller et. al, "Optimized Spatial Hashing for Collision Detection of Deformable Objects" - https://matthias-research.github.io/pages/publications/tetraederCollision.pdf
    - Muller et. al, "Particle-Based Fluid Simulation for Interactive Applications" - https://matthias-research.github.io/pages/publications/sca03.pdf
  - Rendering
    - Kerbl et. al, "3D Gaussian Splatting for Real-time Radiance Field Rendering" - https://arxiv.org/pdf/2308.04079
    - Zwicker et. al, "EWA Splatting" - https://www.cs.umd.edu/~zwicker/publications/EWASplatting-TVCG02.pdf
    - Botsch et. al, "High Quality Surface Splatting" - https://www.cs.umd.edu/~zwicker/publications/HighQualitySurfaceSplatting-PBG05.pdf
    - Van der Laan et. al, "Screen Space Fluid Rendering with Curvature Flow" - https://wstahw.win.tue.nl/edu/2IV06/andrei/particle_rendering/provided/p91-van_der_laan.pdf