<div align="center">
  <img src="Internal/Icons/AppIcon/appicon.png" alt="OpenGLEngine Logo" width="140"/>

  # OpenGLEngine

  *A custom C++20 rendering engine.*

  <br/>

  ![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat&logo=cplusplus&logoColor=white)
  ![OpenGL](https://img.shields.io/badge/OpenGL-4.6-5586A4?style=flat&logo=opengl&logoColor=white)
  ![Platform](https://img.shields.io/badge/Windows-x64-0078D4?style=flat&logo=windows&logoColor=white)
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat)](LICENSE)

  ![Premake](https://img.shields.io/badge/Build-Premake5-E8572A?style=flat&logo=cmake&logoColor=white)
  ![Visual Studio 2022](https://img.shields.io/badge/Visual%20Studio-2022-5C2D91?style=flat&logo=visualtudio&logoColor=white)
  ![Visual Studio 2026](https://img.shields.io/badge/Visual%20Studio-2026-5C2D91?style=flat&logo=visualstudio&logoColor=white)
  ![ImGui](https://img.shields.io/badge/GUI-ImGui-9B59B6?style=flat)
  ![GPU](https://img.shields.io/badge/Renderer-GPU%20Shaders-76B900?style=flat&logo=nvidia&logoColor=white)

</div>

## Architecture

The engine is organized into independent, single-responsibility layers. Each concern (*rendering*, *serialization*, *UI*, *input*) lives in its own module with a clearly defined interface, making the codebase easy to navigate and extend:

```
Source/
  Core/        - Application lifecycle, window management, layer stack
  Renderer/    - OpenGL abstraction (shaders, framebuffers, draw calls)
  Layers/      - Engine logic layer, UI layer; composed at runtime
  Editor/      - ImGui panels (Inspector, Viewport, Project, Settings...)
  Utilities/   - Serialization, file I/O, image export
  Platform/    - Platform-specific entry points
```

## Features

### User Interface
- Dockable ImGui layout: `Inspector`, `Viewport`, `Project`, `Statistics`, `Settings`, `About`
- 40+ built-in themes
- Configurable font size and UI scale

### Configuration & Export
- Save and load scenes as `YAML` files
- High-resolution image export
- Recent scenes list

## Building and Running

### Prerequisites
- Windows 10/11, Visual Studio 2022 or 2026, Git

### Setup

1. Clone the repository recursively to retrieve libraries

   ```bash
   git clone --recursive https://github.com/gonzaloivan121/openglengine
   ```

2. Navigate to the newly created repository folder

   ```bash
   cd openglengine
   ```

3. Run the setup script for your Visual Studio version:

   ```bash
   Scripts/Setup/Windows-vs2022.bat   # VS 2022
   Scripts/Setup/Windows-vs2026.bat   # VS 2026
   ```

4. Open `OpenGLEngine.sln` (or `OpenGLEngine.slnx` depending on the version you used) in Visual Studio, select a configuration, and build.

### Build Configurations

| Configuration   | Description                               |
|-----------------|-------------------------------------------|
| `Debug`         | Full debug symbols, no optimization       |
| `Release`       | Optimized with symbols                    |
| `Dist`          | Fully optimized, windowed app, no console |

## Contributing

Pull requests, bug reports, and feature suggestions are welcome.

- Use the `Debug` configuration for development
- Follow the existing module boundaries when adding features
- Document non-obvious shader math inline

## License

MIT - see [LICENSE](LICENSE) for details.

## Acknowledgments

- **Libraries:** `GLFW`, `ImGui`, `GLM`, `yaml-cpp`, `stb`, `GLAD`.

> **Note**: *Requires a GPU with OpenGL 4.6 support.*
