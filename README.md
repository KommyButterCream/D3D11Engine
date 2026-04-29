# D3D11Engine
Windows Direct3D 11 rendering engine DLL with plugin-oriented interfaces

# Info
Modular C++ Direct3D 11 rendering engine for Windows.
Provides rendering engine/context abstractions, Direct2D/DirectWrite integration, UI and overlay-friendly interfaces, font management, camera utilities, and reusable rendering support code for plugin-based applications.

# Features
- Direct3D 11-based rendering engine
- Direct2D / DirectWrite integrated rendering context
- Plugin-oriented engine interfaces
- Render context abstraction for external modules
- UI / overlay rendering interface support
- Font manager and text format caching
- 2D camera utility for image/view control
- Modular integration with the shared Core library
- Separate D3D11 engine interface project support

# Dependencies
- Core
- D3D11EngineInterface
- Windows Direct3D 11 / DXGI / Direct2D / DirectWrite
- C++20
- MSVC (Visual Studio 2022)

# Build Environment
- C++20
- MSVC (Visual Studio 2022)
- Windows 10/11 x64

# Project Structure
- `Core/` : D3D11 render engine and render context implementation
- `Camera/` : 2D camera and view control utilities
- `Font/` : font manager and DirectWrite text format cache
- `Shader/` : HLSL shader sources
- `Shaders/` : compiled or runtime shader resources
- `util/` : timer, pointer array, smooth value helpers
- `D3D11Engine/` : Visual Studio project files

# Repository Layout
This project expects D3D11Engine, Core, and D3D11EngineInterface to be placed under the same parent directory.

Example:
```text
Module/
+-- Core/
+-- D3D11Engine/
+-- D3D11EngineInterface/
```

The Visual Studio solution references the shared Core project at `../Core/Core/Core.vcxproj`.
The engine source also includes interface headers from `../D3D11EngineInterface/`.

# Notes
- The shared Core library is managed as a sibling repository/project.
- D3D11EngineInterface is managed as a sibling repository/project.
- Open `D3D11Engine.sln` with Visual Studio 2022.
- Build the x64 configuration to produce the D3D11Engine DLL.
