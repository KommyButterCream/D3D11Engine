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
- Modular Core library integration via submodule
- Separate D3D11 engine interface module via submodule

# Dependencies
- [Core](./Modules/Core) as a submodule
- [D3D11EngineInterface](./Modules/D3D11EngineInterface) as a submodule
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
- `util/` : timer, pointer array, smooth value helpers
- `Modules/Core/` : external Core library submodule
- `Modules/D3D11EngineInterface/` : engine interface definitions for plugins and external modules
- `D3D11Engine/` : Visual Studio project files

# Notes
- This repository uses `Modules/Core` and `Modules/D3D11EngineInterface` as submodules.
- Make sure submodules are initialized before building.
- The project is configured for Windows-based development with Visual Studio 2022.
- The main build target is a DLL-based rendering engine.

# Clone
- Clone with submodules:
```bash
git clone --recurse-submodules https://github.com/KommyButterCream/D3D11Engine.git
```
- If already cloned without submodules:
```bash
git submodule update --init --recursive
```

